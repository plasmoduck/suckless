#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <scc/cstd.h>
#include <scc/scc.h>
#include "as.h"

#define NARGS 20
#define NR_INPUTS 10
#define MAXLINE 100

struct input {
	char *fname;
	unsigned lineno;
	FILE *fp;
};

int nerrors;
jmp_buf recover;
char yytext[INTIDENTSIZ+1];
int yytoken;
size_t yylen;
union yylval yylval;

static char *textp, *endp;
static int regmode;
static unsigned lineno;
static struct input inputs[NR_INPUTS], *isp = inputs;

static int
follow(int expect1, int expect2, int ifyes1, int ifyes2, int ifno)
{
	int c;

	if ((c = *++textp) == expect1)
		return ifyes1;
	if (c == expect2)
		return ifyes2;
	--textp;
	return ifno;
}

static void
tok2str(void)
{
	if ((yylen = endp - textp) > INTIDENTSIZ) {
		error("token too big");
		yylen = INTIDENTSIZ;
	}
	memcpy(yytext, textp, yylen);
	yytext[yylen] = '\0';
	textp = endp;
}

static int
iden(void)
{
	int c;
	char *p;

	for ( ; c = *endp; ++endp) {
		if (isalnum(c))
			continue;
		switch (c) {
		case '\'':
		case '_':
		case '-':
		case '.':
		case '$':
			continue;
		default:
			goto out_loop;
		}
	}

out_loop:
	tok2str();
	yylval.sym = lookup(yytext);

	return (yylval.sym->flags & FREG) ? REG : IDEN;
}

static int
number(void)
{
	int c, base = 10;
	char *p;
	TUINT n;

	if (*endp == '0') {
		base = 8;
		++endp;
		if (*endp == 'x') {
			base = 16;
			++endp;
		}
	}
	for (n = 0; (c = *endp) && isxdigit(c); n += c) {
		n *= base;
		c -= '0';
		if (n >= TUINT_MAX - c*base)
			error("overflow in number");
		endp++;
	}
	tok2str();
	yylval.sym = tmpsym(n);

	return NUMBER;
}

static int
character(void)
{
	int c;
	char *p;

	while (*endp != '\'')
		++endp;
	return NUMBER;
}

static int
string(void)
{
	int c;
	size_t l;
	char *s;
	Symbol *sym = tmpsym(0);

	for (++endp; *endp != '"'; ++endp)
		;
	++endp;
	tok2str();
	yylval.sym = sym;
	/* FIXME: this memory is not freed ever */
	l = yylen-2;
	s = memcpy(xmalloc(l+1), yytext+1, l);
	s[l] = '\0';
	sym->name.buf = s;

	return STRING;
}

static int
operator(void)
{
	int c;

	++endp;
	if ((c = *textp) == '>')
		c = follow('=', '>', LE, SHL, '>');
	else if (c == '<')
		c = follow('=', '<', GE, SHR, '>');
	tok2str();

	return c;
}

int
next(void)
{
	int c;

	while (isspace(*textp))
		++textp;

	endp = textp;

	switch (c = *textp) {
	case '\0':
		strcpy(yytext, "EOS");
		yylen = 3;
		c = EOS;
		break;
	case '"':
		c = string();
		break;
	case '\'':
		c = character();
		break;
	case '%':
		c = (regmode ? iden : operator)();
		break;
	case '_':
		c = iden();
		break;
	default:
		if (isdigit(c))
			c = number();
		else if (isalpha(c))
			c = iden();
		else
			c = operator();
		break;
	}
	return yytoken = c;
}

void
expect(int token)
{
	if (yytoken != token)
		unexpected();
	next();
}

void
unexpected(void)
{
	error("unexpected '%s'", yytext);
}

void
error(char *msg, ...)
{
	va_list va;
	struct input *ip;

	assert(isp > inputs);
	ip = &isp[-1];

	va_start(va, msg);
	fprintf(stderr, "as:%s:%u: ", ip->fname, ip->lineno);
	vfprintf(stderr, msg, va);
	putc('\n', stderr);
	nerrors++;
	va_end(va);

	if (nerrors == 10)
		die("as: too many errors");
	longjmp(recover, 1);
}

Node *
getreg(void)
{
	Node *np;

	np = node(REG, NULL, NULL);
	np->sym = yylval.sym;
	np->addr = AREG;
	expect(REG);
	return np;
}

void
regctx(int mode)
{
	regmode = mode;
}

Node *
operand(char **strp)
{
	int imm = 0;
	Node *np;

	textp = *strp;
	regctx(1);
	switch (next()) {
	case EOS:
		np = NULL;
		break;
	case REG:
		np = getreg();
		break;
	case STRING:
		np = node(yytoken, NULL, NULL);
		np->sym = yylval.sym;
		np->addr = ASTR;
		next();
		break;
	case '$':
		next();
		imm = 1;
	default:
		if (!imm) {
			np = moperand();
		} else {
			np = expr();
			np->addr = AIMM;
		}
	}
	if (yytoken != ',' && yytoken != EOS)
		error("trailing characters in expression '%s'", textp);
	*strp = endp;

	return np;
}

Node **
getargs(char *s)
{
	Node **ap;
	static Node *args[NARGS];

	if (!s)
		return NULL;

	for (ap = args; ap < &args[NARGS-1]; ++ap) {
		if ((*ap = operand(&s)) == NULL)
			return args;
	}
	error("too many arguments in one instruction");
}

static char *
field(char **oldp, size_t *siz)
{
	char *s, *t, *begin;
	size_t n;

	if ((begin = *oldp) == NULL)
		return NULL;

	for (s = begin; isspace(*s) && *s != '\t'; ++s)
		;
	if (*s == '\0' || *s == '#') {
		*s = '\0';
		return *oldp = NULL;
	}

	for (t = s; *t && *t != '\t'; ++t)
		;
	if (*t == '\t')
		*t++ = '\0';
	*siz -= begin - t;
	*oldp = t;

	while (t >= s && isspace(*t))
		*t-- = '\0';
	return (*s != '\0') ? s : NULL;
}

static int
validlabel(char *name)
{
	int c;

	while ((c = *name++) != '\0') {
		if (isalnum(c))
			continue;
		switch (c) {
		case '_':
		case '-':
		case '.':
		case '$':
			continue;
		case ':':
			if (*name != '\0')
				return 0;
			*--name = '\0';
			continue;
		default:
			return 0;
		}
	}
	return 1;
}

static int
extract(char *s, size_t len, struct line *lp)
{
	int r = 0;

	if (lp->label = field(&s, &len))
		r++;
	if (lp->op = field(&s, &len))
		r++;
	if (lp->args = field(&s, &len))
		r++;

	if (s && *s && *s != '#')
		error("trailing characters at the end of the line");
	if (lp->label && !validlabel(lp->label))
		error("incorrect label name '%s'", lp->label);

	return r;
}

static size_t
getline(FILE *fp, char buff[MAXLINE])
{
	int c;
	char *bp;

	for (bp = buff; (c = getc(fp)) != EOF; *bp++ = c) {
		if (c == '\n')
			break;

		if (c > UCHAR_MAX)
			error("invalid character '%x'", c);

		if (bp == &buff[MAXLINE-1])
			error("line too long");
	}
	*bp = '\0';

	return bp - buff;
}

int
nextline(struct line *lp)
{
	struct input *ip;
	size_t n;
	static char buff[MAXLINE];

	assert(isp > inputs);
repeat:
	if (isp == inputs)
		return 0;
	ip = &isp[-1];
	if (feof(ip->fp)) {
		delinput();
		goto repeat;
	}
	n = getline(ip->fp, buff);
	if (++ip->lineno == 0)
		die("as: %s: file too long", infile);
	if (n == 0)
		goto repeat;
	if (extract(buff, n, lp) == 0)
		goto repeat;
	return 1;
}

void
addinput(char *fname)
{
	FILE *fp;

	if (isp == &inputs[NR_INPUTS])
		die("as: too many included files");
	if ((fp = fopen(fname, "r")) == NULL)
		die("as: %s: %s", fname, strerror(errno));
	isp->fname = xstrdup(fname);
	isp->fp = fp;
	isp->lineno = 0;
	++isp;
}

int
delinput(void)
{
	if (isp == inputs)
		return EOF;
	--isp;
	if (fclose(isp->fp) == EOF)
		die("as: %s: %s", isp->fname, strerror(errno));
	free(isp->fname);
	return 0;
}

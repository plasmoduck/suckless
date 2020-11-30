/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <regex.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#include "arg.h"

#define LINSIZ        128
#define LEN(x)        (sizeof (x) / sizeof *(x))
#define UTF8LEN(ch)   ((unsigned char)ch>=0xFC ? 6 : \
                      ((unsigned char)ch>=0xF8 ? 5 : \
                      ((unsigned char)ch>=0xF0 ? 4 : \
                      ((unsigned char)ch>=0xE0 ? 3 : \
                      ((unsigned char)ch>=0xC0 ? 2 : 1)))))
#define ISASCII(ch)   ((unsigned char)ch < 0x80)
#define ISCTRL(ch)    (((unsigned char)ch < ' ') || (ch == 0x7F))
#define ISFILL(ch)    (isutf8 && !ISASCII(ch) && (unsigned char)ch<=0xBF)
#define ISBLANK(ch)   (ch == ' ' || ch == '\t' || ch == '\0')
#define ISWORDBRK(ch) (ISASCII(ch) && (ch < 0x30 || \
                      (ch > 0x39 && ch < 0x41) || \
                      (ch > 0x5A && ch < 0x5F) || \
                      ch == 0x60 || \
                      ch > 0x7A)) /* A bit flawed because we assume multibyte UTF8 chars are alnum */
#define VLEN(ch,col)  (ch=='\t' ? tabstop-(col%tabstop) : (ISCTRL(ch) ? 2: (ISFILL(ch) ? 0: 1)))
#define VLINES(l)     (1+(l?l->vlen/cols:0))
#define FIXNEXT(pos)  while(isutf8 && ISFILL(pos.l->c[pos.o]) && ++pos.o < pos.l->len)
#define FIXPREV(pos)  while(isutf8 && ISFILL(pos.l->c[pos.o]) && --pos.o > 0)
#define LINESABS      (lines - (titlewin==NULL?0:1))

/* Typedefs */
typedef struct Line Line;
struct Line {    /** The internal representation of a line of text */
	char *c;     /* Line content */
	size_t len;  /* Line byte length */
	size_t vlen; /* On-screen line-length */
	size_t mul;  /* How many times LINSIZ is c malloc'd to */
	bool dirty;  /* Should I repaint on screen? */
	Line *next;  /* Next line, NULL if I'm last */
	Line *prev;  /* Previous line, NULL if I'm first */
};

typedef struct {  /** A position in the file */
	Line *l;  /* Line */
	size_t o; /* Offset inside the line */
} Filepos;

typedef union { /** An argument to a f_* function, generic */
	long i;
	const void *v;
	Filepos(*m)(Filepos);
} Arg;

typedef struct {                    /** A keybinding */
	union {
		char c[6];                  /* Standard chars */
		int i;                      /* NCurses code */
	} keyv;
	bool(*test[4])(void);           /* Conditions to match, make sure the last one is 0x00 */
	void (*func)(const Arg * arg);  /* Function to perform */
	const Arg arg;                  /* Argument to func() */
} Key;

typedef struct {                    /** A mouse click */
	mmask_t mask;                   /* Mouse mask */
	bool place[2];                  /* Place fcur / fsel at click place before testing */
	bool(*test[3])(void);           /* Conditions to match, make sure the last one is 0x00 */
	void (*func)(const Arg * arg);  /* Function to perform */
	const Arg arg;                  /* Argument to func() */
} Click;

typedef struct {                    /** A command read at the fifo */
	const char *re_text;            /* A regex to match the command, must have a parentheses group for argument */
	bool(*test[3])(void);           /* Conditions to match, make sure the last one is 0x00 */
	void (*func)(const Arg * arg);  /* Function to perform, argument is determined as arg->v from regex above */
	const Arg arg;                  /* Argument to func(), if empty will fill .v = re_match */
} Command;

#define SYN_COLORS 8
typedef struct {               /** A syntax definition */
	char *name;                /* Syntax name */
	char *file_re_text;        /* Apply to files matching this regex */
	char *re_text[SYN_COLORS]; /* Apply colors (in order) matching these regexes */
} Syntax;

typedef struct Undo Undo;
struct Undo {                   /** Undo information */
	char flags;                 /* Flags: is insert/delete?, should concatenate with next undo/redo? */
	unsigned long startl, endl; /* Line number for undo/redo start and end */
	size_t starto, endo;        /* Character offset for undo/redo start and end */
	char *str;                  /* Content (added or deleted text) */
	Undo *prev;                 /* Previous undo/redo in the ring */
};

/* ENUMS */
/* Colors */
enum { DefFG, CurFG, SelFG, SpcFG, CtrlFG, Syn0FG, Syn1FG, Syn2FG, Syn3FG,
	Syn4FG, Syn5FG, Syn6FG, Syn7FG, LastFG,
};
enum { DefBG, CurBG, SelBG, LastBG, }; /* NOTE: BGs MUST have a matching FG */

/* arg->i to use in f_extsel() */
enum { ExtDefault, ExtWord, ExtLines, ExtAll, };

/* To use in lastaction */
enum { LastNone, LastDelete, LastInsert, LastPipe, LastPipeRO, };

/* Environment variables index */
enum { EnvFind, EnvPipe, EnvLine, EnvOffset, EnvFile, EnvSyntax, EnvFifo,
	EnvLast,
};

enum {                      /* To use in statusflags */
	S_Running = 1,          /* Keep the editor running, flip to stop */
	S_Readonly = 1 << 1,    /* File is readonly, this should be enforced so that file is not modified */
	S_InsEsc = 1 << 2,      /* Insert next character as-is, even if it's a control sequence */
	S_CaseIns = 1 << 3,     /* Search is casi insensitive */
	S_Modified = 1 << 4,    /* File has been modified, set automatically */
	S_DirtyScr = 1 << 5,    /* Screen is dirty and should be repainted, set automatically */
	S_DirtyDown = 1 << 6,   /* Line has changed so that we must repaint from here down, set automatically */
	S_NeedResize = 1 << 7,  /* Need to resize screen in next update, set automatically */
	S_Warned = 1 << 8,      /* Set to warn about file not being saved */
	S_GroupUndo = 1 << 9,   /* Last action was an insert, so another insert should group with it, set automatically */
	S_AutoIndent = 1 << 10, /* Perform autoindenting on RET */
	S_DumpStdout = 1 << 11, /* Dump to stdout instead of writing to a file */
	S_Command = 1 << 12,    /* Command mode */
	S_Sentence = 1 << 13,   /* Sentence mode. Pass the next command's parameters (if adjective) to the verb's function */
	S_Parameter = 1 << 14,  /* Parameter mode. Pass the next character as parameter to the function of the command */
	S_Multiply = 1 << 15,   /* Multiply mode. Replay a command x times */
	S_Visual = 1 << 16,     /* Visual mode. You just select things */
};

enum {                 /* To use in Undo.flags */
	UndoIns = 1,       /* This undo is an insert (otherwise a delete) */
	UndoMore = 1 << 1, /* This undo must be chained with the next one when redoing */
	RedoMore = 1 << 2, /* This undo must be chained with the next one when undoing */
};

/* Constants */
static const char *envs[EnvLast] = {
	[EnvFind] = "SANDY_FIND",
	[EnvPipe] = "SANDY_PIPE",
	[EnvLine] = "SANDY_LINE",
	[EnvOffset] = "SANDY_OFFSET",
	[EnvFile] = "SANDY_FILE",
	[EnvSyntax] = "SANDY_SYNTAX",
	[EnvFifo] = "SANDY_FIFO",
};

/* Variables */
static Line   *fstline;            /* First line */
static Line   *lstline;            /* Last line */
static Line   *scrline;            /* First line seen on screen */
static Filepos fsel;               /* Selection point on file */
static Filepos fcur;               /* Insert position on file, cursor, current position */
static Filepos fmrk = { NULL, 0 }; /* Mark */

static int      syntx = -1;                          /* Current syntax index */
static regex_t *find_res[2];                         /* Compiled regex for search term */
static int      sel_re = 0;                          /* Index to the above, we keep 2 REs so regexec does not segfault */
static int      ch;                                  /* Used to store input */
static char     c[7];                                /* Used to store input */
static char     fifopath[PATH_MAX];                  /* Path to command fifo */
static char    *filename = NULL;                     /* Path to file loade on buffer */
static char     title[BUFSIZ];                       /* Screen title */
static char    *tmptitle = NULL;                     /* Screen title, temporary */
static char    *tsl_str = NULL;                      /* String to print to status line */
static char    *fsl_str = NULL;                      /* String to come back from status line */
static WINDOW  *titlewin = NULL;                     /* Title ncurses window, NULL if there is a status line */
static WINDOW  *textwin = NULL;                      /* Main ncurses window */
static Undo    *undos;                               /* Undo ring */
static Undo    *redos;                               /* Redo ring */
static int      textattrs[LastFG][LastBG];           /* Text attributes for each color pair */
static int      savestep = 0;                        /* Index to determine the need to save in undo/redo action */
static int      fifofd;                              /* Command fifo file descriptor */
static long     statusflags = S_Running | S_Command; /* Status flags, very important, OR'd (see enums above) */
static int      lastaction = LastNone;               /* The last action we took (see enums above) */
static int      cols, lines;                         /* Ncurses: to use instead of COLS and LINES, wise */
static mmask_t  defmmask = 0;                        /* Ncurses: mouse event mask */
static void   (*verb)(const Arg * arg);              /* Verb of current sentence */
static Arg      varg;                                /* Arguments of the verb (some will be overwritten by adjective) */
static int      vi;                                  /* Helping var to store place of verb in key chain */
static int      multiply = 1;                        /* Times to replay a command */

/* allocate memory or die. */
static void *ecalloc(size_t, size_t);
static void *erealloc(void *, size_t);
static char *estrdup(const char *);

/* Functions */
/* f_* functions can be linked to an action or keybinding */
static void f_adjective(const Arg *);
static void f_center(const Arg *);
static void f_delete(const Arg *);
static void f_extsel(const Arg *);
static void f_findbw(const Arg *);
static void f_findfw(const Arg *);
static void f_insert(const Arg *);
static void f_line(const Arg *);
static void f_mark(const Arg *);
static void f_move(const Arg *);
static void f_offset(const Arg *);
static void f_pipe(const Arg *);
static void f_pipero(const Arg *);
static void f_repeat(const Arg *);
static void f_save(const Arg *);
static void f_select(const Arg *);
static void f_spawn(const Arg *);
static void f_suspend(const Arg *);
static void f_syntax(const Arg * arg);
static void f_toggle(const Arg * arg);
static void f_undo(const Arg *);

/* i_* funcions are called from inside the main code only */
static Filepos       i_addtext(char *, Filepos);
static void          i_addtoundo(Filepos, const char *);
static void          i_addundo(bool, Filepos, Filepos, char *);
static void          i_advpos(Filepos * pos, int o);
static void          i_calcvlen(Line * l);
static void          i_cleanup(int);
static bool          i_deltext(Filepos, Filepos);
static void          i_die(const char *str);
static void          i_dirtyrange(Line *, Line *);
static bool          i_dotests(bool(*const a[])(void));
static void          i_dokeys(const Key[], unsigned int);
static void          i_edit(void);
static void          i_find(bool);
static char         *i_gettext(Filepos, Filepos);
static void          i_killundos(Undo **);
static Line         *i_lineat(unsigned long);
static unsigned long i_lineno(Line *);
static void          i_mouse(void);
static void          i_multiply(void (*func)(const Arg * arg), const Arg arg);
static void          i_pipetext(const char *);
static void          i_readfifo(void);
static void          i_readfile(char *);
static void          i_resize(void);
static Filepos       i_scrtofpos(int, int);
static bool          i_setfindterm(const char *);
static void          i_setup(void);
static void          i_sigwinch(int);
static void          i_sigcont(int);
static void          i_sortpos(Filepos *, Filepos *);
static void          i_termwininit(void);
static void          i_update(void);
static void          i_usage(void);
static bool          i_writefile(char *);

/* t_* functions to know whether to process an action or keybinding */
static bool t_ai(void);
static bool t_bol(void);
static bool t_eol(void);
static bool t_ins(void);
static bool t_mod(void);
static bool t_rw(void);
static bool t_redo(void);
static bool t_sel(void);
static bool t_sent(void);
static bool t_undo(void);
static bool t_vis(void);
static bool t_warn(void);

/* m_ functions represent a cursor movement and can be passed in an Arg */
static Filepos m_bof(Filepos);
static Filepos m_bol(Filepos);
static Filepos m_smartbol(Filepos);
static Filepos m_eof(Filepos);
static Filepos m_eol(Filepos);
static Filepos m_nextchar(Filepos);
static Filepos m_prevchar(Filepos);
static Filepos m_nextword(Filepos);
static Filepos m_prevword(Filepos);
static Filepos m_nextline(Filepos);
static Filepos m_prevline(Filepos);
static Filepos m_nextscr(Filepos);
static Filepos m_prevscr(Filepos);
static Filepos m_parameter(Filepos);
static Filepos m_sentence(Filepos);
static Filepos m_stay(Filepos);
static Filepos m_tomark(Filepos);
static Filepos m_tosel(Filepos);

#include "config.h"

/* Some extra stuff that depends on config.h */
static regex_t *cmd_res[LEN(cmds)];
static regex_t *syntax_file_res[LEN(syntaxes)];
static regex_t *syntax_res[LEN(syntaxes)][SYN_COLORS];

char *argv0;

static void *
ecalloc(size_t nmemb, size_t size) {
	void *p;

	p = calloc(nmemb, size);
	if(p == NULL)
		i_die("Can't calloc.\n");
	return p;
}

static void *
erealloc(void *ptr, size_t size) {
	void *p;

	p = realloc(ptr, size);
	if(p == NULL) {
		free(ptr);
		i_die("Can't realloc.\n");
	}
	return p;
}

static char *
estrdup(const char *s) {
	char *p;

	p = strdup(s);
	if(p == NULL)
		i_die("Can't strdup.\n");
	return p;
}

/* F_* FUNCTIONS
 * Can be linked to an action or keybinding. Always return void and take
 * const Arg* */

void
f_adjective(const Arg * arg) {
	statusflags &= ~S_Sentence;

	if(arg->m)
		varg.m = arg->m;
	if(arg->i)
		varg.i = arg->i;
	if(arg->v)
		varg.v = arg->v;

	i_multiply(verb, varg);
}

/* Make cursor line the one in the middle of the screen if possible,
 * refresh screen */
void
f_center(const Arg * arg) {
	(void) arg;

	int i = LINESABS / 2;

	scrline = fcur.l;
	while((i -= VLINES(scrline)) > 0 && scrline->prev)
		scrline = scrline->prev;
	i_resize();
}

/* Delete text as per arg->m. Your responsibility: call only if t_rw() */
void
f_delete(const Arg * arg) {
	char *s;
	Filepos pos0 = fcur, pos1 = arg->m(fcur);

#ifdef HOOK_DELETE_ALL
	HOOK_DELETE_ALL;
#endif
	i_sortpos(&pos0, &pos1);
	s = i_gettext(pos0, pos1);
	i_addundo(FALSE, pos0, pos1, s);
	free(s);
	if(i_deltext(pos0, pos1))
		fcur = pos0;
	else
		fcur = fsel = pos0;
	if(fsel.o > fsel.l->len)
		fsel.o = fsel.l->len;
	statusflags |= S_Modified;
	lastaction = LastDelete;
}

/* Extend the selection as per arg->i (see enums above) */
void
f_extsel(const Arg * arg) {
	fmrk = fcur;
	i_sortpos(&fsel, &fcur);
	switch (arg->i) {
	case ExtWord:
		if(fsel.o > 0 && !ISWORDBRK(fsel.l->c[fsel.o - 1]))
			fsel = m_prevword(fsel);
		if(!ISWORDBRK(fcur.l->c[fcur.o]))
			fcur = m_nextword(fcur);
		break;
	case ExtLines:
		fsel.o = 0;
		fcur.o = fcur.l->len;
		break;
	case ExtAll:
		fsel.l = fstline;
		fcur.l = lstline;
		f_extsel(&(const Arg) { .i = ExtLines });
		break;
	case ExtDefault:
	default:
		if(fsel.o == 0 && fcur.o == fcur.l->len)
			f_extsel(&(const Arg) { .i = ExtAll });
		else if(t_sel() || ISWORDBRK(fcur.l->c[fcur.o]))
			f_extsel(&(const Arg) { .i = ExtLines });
		else
			f_extsel(&(const Arg) { .i = ExtWord });
	}
}

/* Find arg->v regex backwards, same as last if arg->v == NULL */
void
f_findbw(const Arg * arg) {
	if(i_setfindterm((char *) arg->v))
		i_find(FALSE);
}

/* Find arg->v regex forward, same as last if arg->v == NULL */
void
f_findfw(const Arg * arg) {
	if(i_setfindterm((char *) arg->v))
		i_find(TRUE);
}

/* Insert arg->v at cursor position. Your responsibility: call only if t_rw() */
void
f_insert(const Arg * arg) {
	Filepos newcur;

	newcur = i_addtext((char *) arg->v, fcur);

	if((statusflags & S_GroupUndo) && undos && (undos->flags & UndoIns) &&
	    fcur.o == undos->endo && undos->endl == i_lineno(fcur.l) &&
	    ((char *) arg->v)[0] != '\n') {
		i_addtoundo(newcur, arg->v);
	} else {
		i_addundo(TRUE, fcur, newcur, (char *) arg->v);
		if(fcur.l != newcur.l)
			fsel = newcur;
	}
	fcur = fsel = newcur;
	statusflags |= (S_Modified | S_GroupUndo);
	lastaction = LastInsert;
}

/* Go to atoi(arg->v) line */
void
f_line(const Arg * arg) {
	long int l;

	l = atoi(arg->v);
	if(!l)
		l = 1;
	fcur.l = i_lineat(l);
	if(fcur.o > fcur.l->len)
		fcur.o = fcur.l->len;
	FIXNEXT(fcur);
}

/* Set mark at current position */
void
f_mark(const Arg * arg) {
	(void) arg;

	fmrk = fcur;
}

/* Move cursor and extend/shrink selection as per arg->m */
void
f_move(const Arg * arg) {
	fcur = arg->m(fcur);
	if(!t_vis())
		fsel = fcur;
}

/* Got to atoi(arg->v) position in the current line */
void
f_offset(const Arg * arg) {
	fcur.o = atoi(arg->v);
	if(fcur.o > fcur.l->len)
		fcur.o = fcur.l->len;
	FIXNEXT(fcur);
}

/* Pipe selection through arg->v external command. Your responsibility:
 * call only if t_rw() */
void
f_pipe(const Arg * arg) {
	i_pipetext(arg->v);
	statusflags |= S_Modified;
	lastaction = LastPipe;
}

/* Pipe selection through arg->v external command but do not update text
 * on screen */
void
f_pipero(const Arg * arg) {
	(void) arg;

	long oldsf = statusflags;

	statusflags |= S_Readonly;
	i_pipetext(arg->v);
	statusflags = oldsf;
	lastaction = LastPipeRO;
}

/* Repeat the last action. Your responsibility: call only if t_rw() */
void
f_repeat(const Arg * arg) {
	(void) arg;

	i_sortpos(&fsel, &fcur);
	switch (lastaction) {
	case LastDelete:
		if(t_sel())
			f_delete(&(const Arg) { .m = m_tosel });
		break;
	case LastInsert:
		if(undos && undos->flags & UndoIns)
			f_insert(&(const Arg) { .v = undos->str });
		break;
	case LastPipe:
		f_pipe(&(const Arg) { .v = getenv(envs[EnvPipe]) });
		break;
	case LastPipeRO:
		f_pipero(&(const Arg) { .v = getenv(envs[EnvPipe]) });
		break;
	}
}

/* Save file with arg->v filename, same if NULL. Your responsibility: call
   only if t_mod() && t_rw() */
void
f_save(const Arg * arg) {
	Undo *u;

	if(arg && arg->v && *((char *) arg->v)) {
		statusflags &= ~S_DumpStdout;
		free(filename);
		filename = estrdup((char *) arg->v);
		setenv(envs[EnvFile], filename, 1);
	} else if(filename == NULL && !(statusflags & S_DumpStdout)) {
		unsetenv(envs[EnvFile]);
#ifdef HOOK_SAVE_NO_FILE
		HOOK_SAVE_NO_FILE;
#endif
		return;
	}

	if(i_writefile((statusflags & S_DumpStdout) ? NULL : filename)) {
		statusflags &= ~S_Modified;
		for(savestep = 0, u = undos; u; u = u->prev, savestep++)
			;
	}
	if(statusflags & S_DumpStdout)
		statusflags &= ~S_Running;
}

/* Move cursor as per arg->m, then move the selection point to previous cursor */
void
f_select(const Arg * arg) {
	/* for f_select(m_tosel), which reverses the selection */
	Filepos tmppos = fcur;

	fcur = arg->m(fcur);
	fsel = tmppos;
}

/* Spawn (char **)arg->v */
void
f_spawn(const Arg * arg) {
	pid_t pid = -1;

	reset_shell_mode();
	if((pid = fork()) == 0) {
		/* setsid() used to be called here, but it does not look as a good idea
		   anymore. TODO: test and delete! */
		execvp(((char **) arg->v)[0], (char **) arg->v);
		fprintf(stderr, "sandy: execvp %s", ((char **) arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	} else if(pid > 0) {
		waitpid(pid, NULL, 0);
	}
	reset_prog_mode();
	if(titlewin)
		redrawwin(titlewin);
	redrawwin(textwin); /* TODO: make this work! */
}

void
f_suspend(const Arg * arg) {
	(void) arg;

	endwin();
	signal(SIGCONT, i_sigcont);
	raise(SIGSTOP);
}

/* Set syntax with name arg->v */
void
f_syntax(const Arg * arg) {
	unsigned int i, j;

	statusflags |= S_DirtyScr;
	for(i = 0; i < LEN(syntaxes); i++)
		if((arg && arg->v)
			? !strcmp(arg->v, syntaxes[i].name)
		    : !regexec(syntax_file_res[i], filename, 0, NULL, 0)) {
			for(j = 0; j < SYN_COLORS; j++) {
				if((syntx >= 0) && syntax_res[syntx][j])
					regfree(syntax_res[syntx][j]);
				if(syntaxes[i].re_text[j] &&
				   regcomp(syntax_res[i][j],
					syntaxes[i].re_text[j],
					REG_EXTENDED | REG_NEWLINE))
					i_die("Faulty regex.\n");
			}
			syntx = i;
			setenv(envs[EnvSyntax], syntaxes[syntx].name, 1);
			return;
		}
	for(i = 0; i < SYN_COLORS; i++) {
		if((syntx >= 0) && syntax_res[syntx][i])
			regfree(syntax_res[syntx][i]);
	}
	syntx = -1;
	setenv(envs[EnvSyntax], "none", 1);
}

/* Toggle the arg->i statusflag. Careful with this one! */
void
f_toggle(const Arg * arg) {
	statusflags ^= (long) arg->i;

	/* Specific operations for some toggles */
	switch (arg->i) {
	case S_CaseIns: /* Re-compile regex with/without REG_ICASE */
		i_setfindterm(getenv(envs[EnvFind]));
		break;
	case S_Warned: /* Set warning title */
		tmptitle = "WARNING! File Modified!!!";
		break;
	}
}

/* Undo last action if arg->i >=0, redo otherwise. Your responsibility:
 * call only if t_undo() / t_redo() */
void
f_undo(const Arg * arg) {
	Filepos start, end;
	const bool isredo = (arg->i < 0);
	Undo *u;
	int n;

	u = (isredo ? redos : undos);
	fsel.o = u->starto, fsel.l = i_lineat(u->startl);
	fcur = fsel;

	while(u) {
		start.o = u->starto, start.l = i_lineat(u->startl);
		end.o = u->endo, end.l = i_lineat(u->endl);

		if(isredo ^ (u->flags & UndoIns)) {
			i_sortpos(&start, &end);
			i_deltext(start, end);
			fcur = fsel = start;
		} else {
			fcur = fsel = i_addtext(u->str, fcur);
		}
		if(isredo)
			redos = u->prev, u->prev = undos, undos = u;
		else
			undos = u->prev, u->prev = redos, redos = u;

		if(!(u->flags & (isredo ? RedoMore : UndoMore)))
			break;
		u = (isredo ? redos : undos);
	}

	for(n = 0, u = undos; u; u = u->prev, n++)
		;
	/* True if we saved at this undo point */
	if(n == savestep)
		statusflags ^= S_Modified;
	else
		statusflags |= S_Modified;
}

/* I_* FUNCTIONS
   Called internally from the program code */

/* Add information to the last undo in the ring */
void
i_addtoundo(Filepos newend, const char *s) {
	size_t oldsiz, newsiz;

	if(!undos)
		return;
	oldsiz = strlen(undos->str);
	newsiz = strlen(s);
	undos->endl = i_lineno(newend.l);
	undos->endo = newend.o;

	undos->str = (char *) erealloc(undos->str, 1 + oldsiz + newsiz);
	strncat(undos->str, s, newsiz);
}

/* Add new undo information to the undo ring */
void
i_addundo(bool ins, Filepos start, Filepos end, char *s) {
	Undo *u;

	if(!s || !*s)
		return;
	if(undos && (statusflags & S_GroupUndo)) {
		end.l = i_lineat((undos->endl - undos->startl) + i_lineno(end.l));
		i_addtoundo(end, s);
		return;
	}

	/* Once you make a change, the old redos go away */
	if(redos)
		i_killundos(&redos);
	u = (Undo *) ecalloc(1, sizeof(Undo));

	u->flags = (ins ? UndoIns : 0);
	u->startl = i_lineno(start.l);
	u->endl = i_lineno(end.l);
	u->starto = start.o;
	u->endo = end.o;
	u->str = estrdup(s);
	u->prev = undos;
	undos = u;
}

/* Add text at pos, return the position after the inserted text */
Filepos
i_addtext(char *buf, Filepos pos) {
	Line *l = pos.l, *lnew = NULL;
	size_t o = pos.o, vlines, i = 0, il = 0;
	Filepos f;
	char c;

	vlines = VLINES(l);
	for(c = buf[0]; c != '\0'; c = buf[++i]) {
		/* newline / line feed */
		if(c == '\n' || c == '\r') {
			lnew = (Line *)ecalloc(1, sizeof(Line));
			lnew->c = ecalloc(1, LINSIZ);
			lnew->dirty = l->dirty = TRUE;
			lnew->len = lnew->vlen = 0;
			lnew->mul = 1;
			lnew->next = l->next;
			lnew->prev = l;
			if(l->next)
				l->next->prev = lnew;
			else
				lstline = lnew;
			l->next = lnew;
			l = lnew;
			/* \n in the middle of a line */
			if(o + il < l->prev->len) {
				f.l = l;
				f.o = 0;
				i_addtext(&(l->prev->c[o + il]), f);
				l->prev->len = o + il;
				l->prev->c[o + il] = '\0';
			}
			i_calcvlen(l->prev);
			o = il = 0;
		} else {
			/* Regular char */
			if(2 + (l->len) >= LINSIZ * (l->mul))
				l->c = (char *) erealloc(l->c, LINSIZ * (++(l->mul)));
			memmove(l->c + il + o + 1, l->c + il + o,
			    (1 + l->len - (il + o)));
			l->c[il + o] = c;
			l->dirty = TRUE;
			if(il + o >= (l->len)++)
				l->c[il + o + 1] = '\0';
			il++;
		}
	}
	i_calcvlen(l);
	f.l = l;
	f.o = il + o;
	if(lnew != NULL || vlines != VLINES(pos.l))
		statusflags |= S_DirtyDown;
	return f;
}

/* Take a file position and advance it o bytes */
void
i_advpos(Filepos * pos, int o) {
	int toeol;

	toeol = pos->l->len - pos->o;
	if(o <= toeol)
		o += pos->o;
	else
		while(o > toeol && pos->l->next) {
			pos->l = pos->l->next;
			o -= (1 + toeol);
			toeol = pos->l->len;
		}
	pos->o = o;
	FIXNEXT((*pos)); /* TODO: this should not be needed here */
}

/* Update the vlen value of a Line */
void
i_calcvlen(Line * l) {
	size_t i;

	l->vlen = 0;
	for(i = 0; i < l->len; i++)
		l->vlen += VLEN(l->c[i], l->vlen);
}

/* Cleanup and exit */
void
i_cleanup(int sig) {
	unsigned int i;

	i_killundos(&undos);
	i_killundos(&redos);
	close(fifofd);
	unlink(fifopath);
	free(filename);
	for(i = 0; i < LEN(cmds); i++)
		regfree(cmd_res[i]);
	for(i = 0; i < LEN(syntaxes); i++)
		regfree(syntax_file_res[i]);
	if(syntx >= 0) {
		for(i = 0; i < SYN_COLORS; i++)
			regfree(syntax_res[syntx][i]);
	}
	regfree(find_res[0]);
	regfree(find_res[1]);
	endwin();
	exit(sig > 0 ? 128 + sig : t_mod()? EXIT_FAILURE : EXIT_SUCCESS);
}

/* Quit less gracefully */
void
i_die(const char *str) {
	reset_shell_mode();
	fputs(str, stderr);
	exit(EXIT_FAILURE);
}

/* The lines between l0 and l1 should be redrawn to the screen */
void
i_dirtyrange(Line * l0, Line * l1) {
	Line *l;
	bool d = FALSE;

	/* Warning: l0 and/or l1 may not even exist!!! */
	for(l = fstline; l; l = l->next) {
		if(d && (l == l0 || l == l1)) {
			l->dirty = TRUE;
			break;
		}
		if(l == l0 || l == l1)
			d ^= 1;
		if(d)
			l->dirty = TRUE;
	}
}

/* Delete text between pos0 and pos1, which MUST be in order, fcur integrity
   is NOT assured after deletion, fsel integrity is returned as a bool */
bool
i_deltext(Filepos pos0, Filepos pos1) {
	Line *ldel = NULL;
	size_t vlines = 1;
	bool integrity = TRUE;

	if(pos0.l == fsel.l)
		integrity = (fsel.o <= pos0.o || (pos0.l == pos1.l
			&& fsel.o > pos1.o));
	if(pos0.l == pos1.l) {
		vlines = VLINES(pos0.l);
		memmove(pos0.l->c + pos0.o, pos0.l->c + pos1.o,
		    (pos0.l->len - pos1.o));
		pos0.l->dirty = TRUE;
		pos0.l->len -= (pos1.o - pos0.o);
		pos0.l->c[pos0.l->len] = '\0';
		i_calcvlen(pos0.l);
	} else {
		pos0.l->len = pos0.o;
		pos0.l->c[pos0.l->len] = '\0';
		pos0.l->dirty = TRUE; /* <<-- glitch in screen updates! */
		/* i_calcvlen is unneeded here, because we call i_addtext later */
		while(pos1.l != ldel) {
			if(pos1.l == pos0.l->next)
				i_addtext(&(pos0.l->next->c[pos1.o]), pos0);
			if(pos0.l->next->next)
				pos0.l->next->next->prev = pos0.l;
			ldel = pos0.l->next;
			pos0.l->next = pos0.l->next->next;
			if(scrline == ldel)
				scrline = ldel->prev;
			if(lstline == ldel)
				lstline = ldel->prev;
			if(fsel.l == ldel)
				integrity = FALSE;
			free(ldel->c);
			free(ldel);
		}
	}
	if(ldel != NULL || vlines != VLINES(pos0.l))
		statusflags |= S_DirtyDown;
	return integrity;
}

/* test an array of t_ functions */
bool
i_dotests(bool(*const a[])(void)) {
	int i;

	for(i = 0; a[i]; i++) {
		if(!a[i]())
			return FALSE;
	}
	return TRUE;
}

void
i_dokeys(const Key bindings[], unsigned int length_bindings) {
	unsigned int index, i, j;

	for(index = 0; index < length_bindings; index++) {
		if(((bindings[index].keyv.c &&
		     memcmp(c, bindings[index].keyv.c,
		            sizeof bindings[index].keyv.c) == 0) ||
		    (bindings[index].keyv.i && ch == bindings[index].keyv.i)) &&
		     i_dotests(bindings[index].test)) {

			if(bindings[index].func != f_insert)
				statusflags &= ~(S_GroupUndo);

			/* Handle sentences */
			if(t_sent()) {
				if(bindings[index].func == verb) {
					varg.m = m_nextline;
					i_multiply(verb, varg);
					statusflags &= ~S_Sentence;
				} else if(bindings[index].func != f_adjective) {
					statusflags &= ~S_Sentence;
					break;
				}
			} else if(bindings[index].arg.m == m_sentence) {
				statusflags |= (long) S_Sentence;
				verb = bindings[index].func;
				varg = bindings[index].arg;
				vi = index;
				break;
			}

			/* Handle parameter sentences (verb is used here to define the
			   command to execute) */
			if(statusflags & S_Parameter) {
				statusflags &= ~S_Parameter;
				i_multiply(verb, (const Arg) { .v = c });
				break;
			} else if(bindings[index].arg.m == m_parameter) {
				statusflags |= (long) S_Parameter;
				verb = bindings[index].func;
				break;
			}

			i_multiply(bindings[index].func, bindings[index].arg);
			if(t_sent() && bindings[index].func == f_adjective)
				i = vi;
			else
				i = index;

			/* Handle multi-function commands */
			if(i + 1 < length_bindings) {
				if((bindings[i + 1].keyv.c &&
				    memcmp(bindings[i + 1].keyv.c, bindings[i].keyv.c,
					       sizeof bindings[i].keyv.c) == 0) ||
				    (bindings[i + 1].keyv.i	&&
					bindings[i + 1].keyv.i == bindings[index].keyv.i)) {

					for(j = 0; j < LEN(bindings[i].test); j++) {
						if(bindings[i].test[j] != bindings[i + 1].test[j])
							break;
					}
					if(j == LEN(bindings[i].test))
						continue;
				}
			}
			break;
		}
	}
}

/* Main editing loop */
void
i_edit(void) {
	int i, tch;
	fd_set fds;
	Filepos oldsel, oldcur;

	oldsel.l = oldcur.l = fstline;
	oldsel.o = oldcur.o = 0;

	while(statusflags & S_Running) {
		if(fsel.l != oldsel.l)
			i_dirtyrange(oldsel.l, fsel.l);
		else if(fsel.o != oldsel.o)
			fsel.l->dirty = TRUE;
		if(fcur.l != oldcur.l)
			i_dirtyrange(oldcur.l, fcur.l);
		else if(fcur.o != oldcur.o)
			fcur.l->dirty = TRUE;
		oldsel = fsel, oldcur = fcur;
		i_update();

#ifdef HOOK_SELECT_ALL
		if(fsel.l != fcur.l || fsel.o != fcur.o)
			HOOK_SELECT_ALL;
#endif
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(fifofd, &fds);
		signal(SIGWINCH, i_sigwinch);
		if(select(FD_SETSIZE, &fds, NULL, NULL, NULL) == -1 &&
		   errno == EINTR) {
			signal(SIGWINCH, SIG_IGN);
			continue;
		}
		signal(SIGWINCH, SIG_IGN);
		if(FD_ISSET(fifofd, &fds))
			i_readfifo();
		if(!FD_ISSET(0, &fds))
			continue;
		if((ch = wgetch(textwin)) == ERR) {
			tmptitle = "ERR";
			continue;
		}

		/* NCurses special chars are processed first to avoid UTF-8 collision */
		if(ch >= KEY_MIN) {
			/* These are not really chars */
#if HANDLE_MOUSE
			if(ch == KEY_MOUSE)
				i_mouse();
			else
#endif /* HANDLE_MOUSE */
				i_dokeys(curskeys, LEN(curskeys));
			continue;
		}

		/* Mundane characters are processed later */
		c[0] = (char) ch;
		if(c[0] == 0x1B || (isutf8 && !ISASCII(c[0]))) {
			/* Multi-byte char or escape sequence */
			wtimeout(textwin, 1);
			for(i = 1; i < (c[0] == 0x1B ? 6 : UTF8LEN(c[0])); i++) {
				tch = wgetch(textwin);
				c[i] = (char)tch;
				if(tch == ERR)
					break;
			}
			for(; i < 7; i++)
				c[i] = '\0';
			wtimeout(textwin, 0);
		} else {
			c[1] = c[2] = c[3] = c[4] = c[5] = c[6] = '\0';
		}

		if(!(statusflags & S_InsEsc) && ISCTRL(c[0])) {
			i_dokeys(stdkeys, LEN(stdkeys));
			continue;
		}
		statusflags &= ~(S_InsEsc);

		if(t_rw() && t_ins()) {
			f_insert(&(const Arg) { .v = c });
		}
#if VIM_BINDINGS
		else if(!t_ins()) {
			if(ch >= '0' && ch <= '9' && !(statusflags & S_Parameter)) {
				if(statusflags & S_Multiply) {
					multiply *= 10;
					multiply += (int) ch - '0';
				} else {
					statusflags |= S_Multiply;
					multiply = (int) ch - '0';
				}
			} else {
				i_dokeys(commkeys, LEN(commkeys));
			}
		}
#endif /* VIM_BINDINGS */
		else {
			tmptitle = "WARNING! File is read-only!!!";
		}
	}
}

/* Find text as per the current find_res[sel_re] */
void
i_find(bool fw) {
	char *s, *subs;
	int wp, _so, _eo, status, flags;
	Filepos start, end;
	regmatch_t result[1];

	start.l = fstline;
	start.o = 0;
	end.l = lstline;
	end.o = lstline->len;
	i_sortpos(&fsel, &fcur);

	for(wp = 0; wp < 2; wp++) {
		if(wp > 0)
			s = i_gettext(start, end);
		else if(fw)
			s = i_gettext(fcur, end);
		else
			s = i_gettext(start, fsel);

		flags = 0;
		if(fw && (fcur.o != 0))
			flags = REG_NOTBOL;
		else if(!fw && (fsel.o != fsel.l->len))
			flags = REG_NOTEOL;

		if((status = regexec(find_res[sel_re], s, 1, result, flags)) == 0) {
			f_mark(NULL);
			if(wp > 0 || !fw)
				fcur = start;
			fsel = fcur;
			_so = result[0].rm_so;
			_eo = result[0].rm_eo;
			if(!fw) {
				subs = &s[_eo];
				while(!regexec(find_res[sel_re], subs, 1,
					result, REG_NOTBOL)
				    && result[0].rm_eo) {
					/* This is blatantly over-simplified: do not try to match
					 * an empty string backwards as it will match the first hit
					 * on the file. */
					_so = _eo + result[0].rm_so;
					_eo += result[0].rm_eo;
					subs = &s[_eo];
				}
			}
			i_advpos(&fsel, _so);
			i_advpos(&fcur, _eo);
			wp++;
		}
		free(s);
	}
}

/* Return text between pos0 and pos1, which MUST be in order; you MUST free
   the returned string after use */
char *
i_gettext(Filepos pos0, Filepos pos1) {
	Line *l;
	unsigned long long i = 1;
	char *buf;

	for(l = pos0.l; l != pos1.l->next; l = l->next)
		i += 1 + (l == pos1.l ? pos1.o : l->len) - (l == pos0.l ? pos0.o : 0);
	buf = ecalloc(1, i);
	for(l = pos0.l, i = 0; l != pos1.l->next; l = l->next) {
		memcpy(buf + i, l->c + (l == pos0.l ? pos0.o : 0),
		    (l == pos1.l ? pos1.o : l->len) - (l == pos0.l ? pos0.o : 0));
		i += (l == pos1.l ? pos1.o : l->len) - (l == pos0.l ? pos0.o : 0);
		if(l != pos1.l)
			buf[i++] = '\n';
	}
	return buf;
}

/* Kill the content of the &list undo/redo ring */
void
i_killundos(Undo ** list) {
	Undo *u;

	for(; *list;) {
		u = (*list)->prev;
		free((*list)->str);
		free(*list);
		*list = u;
	}
}

/* Return the Line numbered il */
Line *
i_lineat(unsigned long il) {
	unsigned long i;
	Line *l;

	for(i = 1, l = fstline; i != il && l && l->next; i++)
		l = l->next;
	return l;
}

/* Return the line number for l0 */
unsigned long
i_lineno(Line * l0) {
	unsigned long i;
	Line *l;
	for(i = 1, l = fstline; l != l0; l = l->next)
		i++;
	return i;
}

#if HANDLE_MOUSE
/* Process mouse input */
void
i_mouse(void) {
	unsigned int i;
	MEVENT ev;
	Filepos f;

	if(getmouse(&ev) == ERR)
		return;
	if(!wmouse_trafo(textwin, &ev.y, &ev.x, FALSE))
		return;

	for(i = 0; i < LEN(clks); i++)
		/* Warning! cursor placement code takes place BEFORE tests are taken
		 * into account */
		if(ev.bstate & clks[i].mask) {
			/* While this allows to extend the selection, it may cause some confusion */
			f = i_scrtofpos(ev.x, ev.y);
			if(clks[i].place[0])
				fcur = f;
			if(clks[i].place[1])
				fsel = f;
			if(i_dotests(clks[i].test)) {
				if(clks[i].func)
					clks[i].func(&(clks[i].arg));
				break;
			}
		}
}
#endif /* HANDLE_MOUSE */

/* Handle multiplication */
void
i_multiply(void (*func)(const Arg * arg), const Arg arg) {
	int i;

	if(statusflags & S_Multiply) {
		func(&arg);
		statusflags |= S_GroupUndo;

		for(i = 1; i < multiply; i++)
			func(&arg);

		statusflags &= ~S_GroupUndo;
		statusflags &= ~S_Multiply;
		multiply = 1;
	} else
		func(&arg);
}

/* Pipe text between fsel and fcur through cmd */
void
i_pipetext(const char *cmd) {
	struct timeval tv;
	int pin[2], pout[2], perr[2], nr = 1, nerr = 1, nw, written;
	int iw = 0, closed = 0, exstatus;
	char *buf = NULL, *ebuf = NULL, *s = NULL;
	Filepos auxp;
	fd_set fdI, fdO;
	pid_t pid = -1;

	if(!cmd || cmd[0] == '\0')
		return;
	setenv(envs[EnvPipe], cmd, 1);
	if(pipe(pin) == -1)
		return;
	if(pipe(pout) == -1) {
		close(pin[0]);
		close(pin[1]);
		return;
	}
	if(pipe(perr) == -1) {
		close(pin[0]);
		close(pin[1]);
		close(pout[0]);
		close(pout[1]);
		return;
	}

	i_sortpos(&fsel, &fcur);

	/* Things I will undo or free at the end of this function */
	s = i_gettext(fsel, fcur);

	if((pid = fork()) == 0) {
		dup2(pin[0], 0);
		dup2(pout[1], 1);
		dup2(perr[1], 2);
		close(pin[0]);
		close(pin[1]);
		close(pout[0]);
		close(pout[1]);
		close(perr[0]);
		close(perr[1]);
		/* I actually like it with sh so I can input pipes et al. */
		execl("/bin/sh", "sh", "-c", cmd, NULL);
		fprintf(stderr, "sandy: execl sh -c %s", cmd);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}

	if(pid > 0) {
		close(pin[0]);
		close(pout[1]);
		close(perr[1]);
		if(t_rw()) {
			i_addundo(FALSE, fsel, fcur, s);
			if(undos)
				undos->flags ^= RedoMore;
			i_deltext(fsel, fcur);
			fcur = fsel;
		}
		fcntl(pin[1], F_SETFL, O_NONBLOCK);
		fcntl(pout[0], F_SETFL, O_NONBLOCK);
		fcntl(perr[0], F_SETFL, O_NONBLOCK);
		buf = ecalloc(1, BUFSIZ + 1);
		ebuf = ecalloc(1, BUFSIZ + 1);
		FD_ZERO(&fdO);
		FD_SET(pin[1], &fdO);
		FD_ZERO(&fdI);
		FD_SET(pout[0], &fdI);
		FD_SET(perr[0], &fdI);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		nw = strlen(s);
		while(select(FD_SETSIZE, &fdI, &fdO, NULL, &tv) > 0 &&
		     (nw > 0 || nr > 0)) {
			fflush(NULL);
			if(FD_ISSET(pout[0], &fdI) && nr > 0) {
				nr = read(pout[0], buf, BUFSIZ);
				if(nr >= 0)
					buf[nr] = '\0';
				else
					break; /* ...not seen it yet */
				if(nr && t_rw()) {
					auxp = i_addtext(buf, fcur);
					i_addundo(TRUE, fcur, auxp, buf);
					if(undos)
						undos->flags ^= RedoMore | UndoMore;
					fcur = auxp;
				}
			} else if(nr > 0) {
				FD_SET(pout[0], &fdI);
			} else {
				FD_CLR(pout[0], &fdI);
			}
			if(FD_ISSET(perr[0], &fdI) && nerr > 0) {
				/* Blatant TODO: take last line of stderr and copy as tmptitle */
				ebuf[0] = '\0';
				nerr = read(perr[0], ebuf, BUFSIZ);
				if(nerr == -1)
					tmptitle = "WARNING! command reported an error!!!";
				if(nerr < 0)
					break;
			} else if(nerr > 0) {
				FD_SET(perr[0], &fdI);
			} else {
				FD_CLR(perr[0], &fdI);
			}
			if(FD_ISSET(pin[1], &fdO) && nw > 0) {
				written = write(pin[1], &(s[iw]),
				    (nw < BUFSIZ ? nw : BUFSIZ));
				if(written < 0)
					break; /* broken pipe? */
				iw += (nw < BUFSIZ ? nw : BUFSIZ);
				nw -= written;
			} else if(nw > 0) {
				FD_SET(pin[1], &fdO);
			} else {
				if(!closed++)
					close(pin[1]);
				FD_ZERO(&fdO);
			}
		}
		if(t_rw()) {
			if(undos)
				undos->flags ^= RedoMore;
		}
		free(buf);
		free(ebuf);
		if(!closed)
			close(pin[1]);
		waitpid(pid, &exstatus, 0); /* We don't want to close the pipe too soon */
		close(pout[0]);
		close(perr[0]);
	} else {
		close(pin[0]);
		close(pin[1]);
		close(pout[0]);
		close(pout[1]);
		close(perr[0]);
		close(perr[1]);
	}
	free(s);
}

/* Read the command fifo */
void
i_readfifo(void) {
	char *buf, *tofree;
	regmatch_t result[2];
	unsigned int i;
	int r;

	tofree = ecalloc(1, BUFSIZ + 1);
	buf = tofree;
	r = read(fifofd, buf, BUFSIZ);
	if(r != -1) {
		buf[r] = '\0';
		buf = strtok(buf, "\n");
		while(buf != NULL) {
			for(i = 0; i < LEN(cmds); i++) {
				if(!regexec(cmd_res[i], buf, 2, result, 0) &&
				   i_dotests(cmds[i].test)) {
					*(buf + result[1].rm_eo) = '\0';
					if(cmds[i].arg.i > 0)
						cmds[i].func(&(cmds[i].arg));
					else
						cmds[i].func(&(const Arg) { .v = (buf + result[1].rm_so) });
					break;
				}
			}
			buf = strtok(NULL, "\n");
		}
	}
	free(tofree);

	/* Kludge: we close and reopen to circumvent a bug?
	 * if we don't do this, fifofd seems to be always ready to select()
	 * also, I was unable to mark fifofd as blocking after opening */
	close(fifofd);
	if((fifofd = open(fifopath, O_RDONLY | O_NONBLOCK)) == -1)
		i_die("Can't open FIFO for reading.\n");
}

/* Read file content into the Line* structure */
void
i_readfile(char *fname) {
	int fd;
	ssize_t n;
	char *buf = NULL, *linestr = "1";

	if(fname == NULL || !strcmp(fname, "-")) {
		fd = 0;
		reset_shell_mode();
	} else {
		free(filename);
		filename = estrdup(fname);
		setenv(envs[EnvFile], filename, 1);
		if((fd = open(filename, O_RDONLY)) == -1) {
			/* start at line number if specified, NOTE that filenames
			 * containing ':' are possible. */
			if((linestr = strrchr(filename, ':'))) {
				*(linestr++) = '\0';
				setenv(envs[EnvFile], filename, 1);
				fd = open(filename, O_RDONLY);
			} else {
				linestr = "1";
			}
			if(fd == -1) {
				tmptitle = "WARNING! Can't read file!!!";
				return;
			}
		}
		f_syntax(NULL); /* Try to guess a syntax */
	}

	buf = ecalloc(1, BUFSIZ + 1);
	while((n = read(fd, buf, BUFSIZ)) > 0) {
		buf[n] = '\0';
		fcur = i_addtext(buf, fcur);
	}
	if(fd > 0) {
		close(fd);
	} else {
		if((fd = open("/dev/tty", O_RDONLY)) == -1)
			i_die("Can't reopen stdin.\n");
		dup2(fd, 0);
		close(fd);
		reset_prog_mode();
	}
	free(buf);
	fcur.l = fstline;
	fcur.o = 0;
	fsel = fcur;
	f_line(&(const Arg) { .v = linestr });
}

/* Handle term resize, ugly. TODO: clean and change */
void
i_resize(void) {
	const char *tty;
	int fd, result;
	struct winsize ws;

	if((tty = ttyname(0)) == NULL)
		return;
	fd = open(tty, O_RDWR);
	if(fd == -1)
		return;
	result = ioctl(fd, TIOCGWINSZ, &ws);
	close(fd);
	if(result == -1)
		return;
	cols = ws.ws_col;
	lines = ws.ws_row;
	endwin();
	doupdate();
	i_termwininit();
	statusflags |= S_DirtyScr;
}

#if HANDLE_MOUSE
/* Return file position at screen coordinates x and y */
Filepos
i_scrtofpos(int x, int y) {
	Filepos pos;
	Line *l;
	int irow, ixrow, ivchar, vlines = 1;

	pos.l = lstline;
	pos.o = pos.l->len;
	for(l = scrline, irow = 0; l && irow < LINESABS; l = l->next, irow += vlines) {
		vlines = VLINES(l);
		for(ixrow = ivchar = 0; ixrow < vlines && (irow + ixrow) < LINESABS; ixrow++) {
			if(irow + ixrow == y) {
				pos.l = l;
				pos.o = 0;
				while(x > (ivchar % cols)
				    || (ivchar / cols) < ixrow) {
					ivchar += VLEN(l->c[pos.o], ivchar);
					pos.o++;
				}
				if(pos.o > pos.l->len)
					pos.o = pos.l->len;
				break;
			}
		}
	}
	return pos;
}
#endif /* HANDLE_MOUSE */

/* Update find_res[sel_re] and sel_re. Return TRUE if find term is a valid
   RE or NULL */
bool
i_setfindterm(const char *find_term) {
	int flags;

	/* Modify find term; use NULL to repeat search */
	if(!find_term)
		return TRUE;

	flags = REG_EXTENDED | REG_NEWLINE;
	if(statusflags & S_CaseIns)
		flags |= REG_ICASE;
	if(!regcomp(find_res[sel_re ^ 1], find_term, flags)) {
		sel_re ^= 1;
		setenv(envs[EnvFind], find_term, 1);
		return TRUE;
	}
	return FALSE;
}

/* Setup everything */
void
i_setup(void) {
	unsigned int i, j;
	Line *l = NULL;

	/* Signal handling, default */
	/* TODO: use sigaction() ? */
	signal(SIGWINCH, SIG_IGN);
	signal(SIGINT, i_cleanup);
	signal(SIGTERM, i_cleanup);

	/* Some allocs */
	title[0] = '\0';
	fifopath[0] = '\0';
	find_res[0] = (regex_t *) ecalloc(1, sizeof(regex_t));
	find_res[1] = (regex_t *) ecalloc(1, sizeof(regex_t));

	for(i = 0; i < LEN(cmds); i++) {
		cmd_res[i] = (regex_t *) ecalloc(1, sizeof(regex_t));
		if(regcomp(cmd_res[i], cmds[i].re_text,
			REG_EXTENDED | REG_ICASE | REG_NEWLINE))
			i_die("Faulty regex.\n");
	}

	for(i = 0; i < LEN(syntaxes); i++) {
		syntax_file_res[i] = (regex_t *) ecalloc(1, sizeof(regex_t));
		if(regcomp(syntax_file_res[i], syntaxes[i].file_re_text,
			REG_EXTENDED | REG_NOSUB | REG_ICASE | REG_NEWLINE))
			i_die("Faulty regex.\n");
		for(j = 0; j < SYN_COLORS; j++)
			syntax_res[i][j] = (regex_t *) ecalloc(1, sizeof(regex_t));
	}

	snprintf(fifopath, sizeof(fifopath), "%s%d", fifobase, getpid());
	if(mkfifo(fifopath, (S_IRUSR | S_IWUSR)) != 0)
		i_die("FIFO already exists.\n");
	if((fifofd = open(fifopath, O_RDONLY | O_NONBLOCK)) == -1)
		i_die("Can't open FIFO for reading.\n");
	setenv(envs[EnvFifo], fifopath, 1);
	regcomp(find_res[0], "\0\0", 0); /* This should not match anything */
	regcomp(find_res[1], "\0\0", 0);

	if(!newterm(NULL, stderr, stdin)) {
		if(!(newterm("xterm", stderr, stdin)))
			i_die("Can't fallback $TERM to xterm, exiting...\n");
		tmptitle = "WARNING! $TERM not recognized, using xterm as fallback!!!";
	}
	if(has_colors()) {
		start_color();
		use_default_colors();
		for(i = 0; i < LastFG; i++) {
			for(j = 0; j < LastBG; j++) {
				/* Handle more than 8 colors */
				if(fgcolors[i] > 7)
					init_color(fgcolors[i], fgcolors[i] >> 8,
					           (fgcolors[i] >> 4) & 0xF, fgcolors[i] & 0xFF);

				if(bgcolors[j] > 7)
					init_color(bgcolors[j], bgcolors[j] >> 8, (bgcolors[j] >> 4) & 0xF,
					           bgcolors[j] & 0xFF);
				init_pair((i * LastBG) + j, fgcolors[i], bgcolors[j]);
				textattrs[i][j] = COLOR_PAIR((i * LastBG) + j) | colorattrs[i];
			}
		}
	} else {
		for(i = 0; i < LastFG; i++) {
			for(j = 0; j < LastBG; j++)
				textattrs[i][j] = bwattrs[i];
		}
	}
	lines = LINES;
	cols = COLS;
	i_termwininit();

	/* Init line structure */
	l = (Line *) ecalloc(1, sizeof(Line));
	l->c = ecalloc(1, LINSIZ);
	l->dirty = FALSE;
	l->len = l->vlen = 0;
	l->mul = 1;
	l->next = NULL;
	l->prev = NULL;
	fstline = lstline = scrline = fcur.l = l;
	fcur.o = 0;
	fsel = fcur;
}

/* Process SIGWINCH, the terminal has been resized */
void
i_sigwinch(int unused) {
	(void) unused;

	statusflags |= S_NeedResize;
}

/* Process SIGCONT to return after STOP */
void
i_sigcont(int unused) {
	(void) unused;

	i_resize();
}

/* Exchange pos0 and pos1 if not in order */
void
i_sortpos(Filepos * pos0, Filepos * pos1) {
	Filepos p;

	for(p.l = fstline; p.l; p.l = p.l->next) {
		if(p.l == pos0->l || p.l == pos1->l) {
			if((p.l == pos0->l && (p.l == pos1->l
				    && pos1->o < pos0->o)) || (p.l == pos1->l
				&& p.l != pos0->l))
				p = *pos0, *pos0 = *pos1, *pos1 = p;
			break;
		}
	}
}

/* Initialize terminal */
void
i_termwininit(void) {
	unsigned int i;

	raw();
	noecho();
	nl();
	if(textwin)
		delwin(textwin);
	if(USE_TERM_STATUS && tigetflag("hs") > 0) {
		tsl_str = tigetstr("tsl");
		fsl_str = tigetstr("fsl");
		textwin = newwin(lines, cols, 0, 0);
	} else {
		if(titlewin)
			delwin(titlewin);
		titlewin = newwin(1, cols, BOTTOM_TITLE ? lines - 1 : 0, 0);
		wattron(titlewin, A_REVERSE);
		textwin = newwin(lines - 1, cols, BOTTOM_TITLE ? 0 : 1, 0);
	}
	idlok(textwin, TRUE);
	keypad(textwin, TRUE);
	meta(textwin, TRUE);
	nodelay(textwin, FALSE);
	wtimeout(textwin, 0);
	curs_set(1);
	ESCDELAY = 20;
	mouseinterval(20);
	scrollok(textwin, FALSE);

#if HANDLE_MOUSE
	for(i = 0; i < LEN(clks); i++)
		defmmask |= clks[i].mask;
	mousemask(defmmask, NULL);
#endif /* HANDLE_MOUSE */
}

/* Repaint screen. This is where everything happens. Apologies for the
   unnecessary complexity and length */
void
i_update(void) {
	int iline, irow, ixrow, ivchar, i, ifg, ibg, vlines;
	int cursor_r = 0, cursor_c = 0;
	int lines3; /* How many lines fit on screen */
	long int nscr, ncur = 1, nlst = 1; /* Line number for scrline, fcur.l and lstline */
	size_t ichar;
	bool selection;
	regmatch_t match[SYN_COLORS][1];
	Line *l;
	char c[7], buf[16];

	/* Check if we need to resize */
	if(statusflags & S_NeedResize)
		i_resize();

	/* Check offset */
	scrollok(textwin, TRUE); /* Here I scroll */
	for(selection = FALSE, l = fstline, iline = 1;
	    l && scrline->prev && l != scrline; iline++, l = l->next) {
		if(l == fcur.l) { /* Can't have fcur.l before scrline, move scrline up */
			i = 0;
			while(l != scrline) {
				if(VLINES(scrline) > 1) {
					i = -1;
					break;
				}
				i++;
				scrline = scrline->prev;
			}
			if(i < 0 || i > LINESABS) {
				scrline = l;
				statusflags |= S_DirtyScr;
			} else
				wscrl(textwin, -i);
			break;
		}
		if(l == fsel.l) /* Selection starts before screen view */
			selection = !selection;
	}
	for(i = irow = 0, l = scrline; l; l = l->next, irow += vlines) {
		if((vlines = VLINES(l)) > 1)
			statusflags |= S_DirtyDown; /* if any line before fcur.l has vlines>1 */
		if(fcur.l == l) {
			if(irow + vlines > 2 * LINESABS)
				statusflags |= S_DirtyScr;
			/* Can't have fcur.l after screen end, move scrline down */
			while(irow + vlines > LINESABS && scrline->next) {
				irow -= VLINES(scrline);
				i += VLINES(scrline);
				if(scrline == fsel.l)
					selection = !selection; /* We just scrolled past the selection point */
				scrline = scrline->next;
				iline++;
			}
			if(!(statusflags & S_DirtyScr))
				wscrl(textwin, i);
			break;
		}
	}
	scrollok(textwin, FALSE);
	nscr = iline;

	/* Actually update lines on screen */
	for(irow = lines3 = 0, l = scrline; irow < LINESABS;
	    irow += vlines, lines3++, iline++) {
		vlines = VLINES(l);
		if(fcur.l == l) {
			ncur = iline;
			/* Update screen cursor position */
			cursor_c = 0;
			cursor_r = irow;
			for(ichar = 0; ichar < fcur.o; ichar++)
				cursor_c += VLEN(fcur.l->c[ichar], cursor_c);
			while(cursor_c >= cols) {
				cursor_c -= cols;
				cursor_r++;
			}
		}
		if(statusflags & S_DirtyScr || (l && l->dirty
			&& (statusflags & S_DirtyDown ? statusflags |= S_DirtyScr : 1))) {
			/* Print line content */
			if(l)
				l->dirty = FALSE;
			if(syntx >= 0 && l)
				for(i = 0; i < SYN_COLORS; i++)
					if(regexec(syntax_res[syntx][i], l->c, 1, match[i], 0) ||
						match[i][0].rm_so == match[i][0].rm_eo)
						match[i][0].rm_so = match[i][0].rm_eo = -1;
			for(ixrow = ichar = ivchar = 0; ixrow < vlines && (irow + ixrow) < LINESABS; ixrow++) {
				wmove(textwin, (irow + ixrow), (ivchar % cols));
				while(ivchar < (1 + ixrow) * cols) {
					if(fcur.l == l && ichar == fcur.o)
						selection = !selection;
					if(fsel.l == l && ichar == fsel.o)
						selection = !selection;
					ifg = DefFG, ibg = DefBG;
					if(fcur.l == l)
						ifg = CurFG, ibg = CurBG;
					if(selection)
						ifg = SelFG, ibg = SelBG;
					if(syntx >= 0 && l)
						for(i = 0; i < SYN_COLORS; i++) {
							if(match[i][0].rm_so == -1)
								continue;
							if(ichar >= (size_t) match[i][0].rm_eo) {
								if(regexec(syntax_res[syntx][i], &l->c [ichar], 1, match[i],REG_NOTBOL) ||
								    match[i][0].rm_so == match[i][0].rm_eo)
									continue;
								match[i][0].rm_so += ichar;
								match[i][0].rm_eo += ichar;
							}
							if(ichar >= (size_t) match[i][0].rm_so && ichar < (size_t) match[i][0].rm_eo)
								ifg = Syn0FG + i;
						}
					wattrset(textwin, textattrs[ifg][ibg]);
					if(l && ichar < l->len) {
						/* Tab nightmare */
						if(l->c[ichar] == '\t') {
							wattrset(textwin, textattrs[SpcFG][ibg]);
							for(i = 0; i < VLEN('\t', ivchar); i++)
								waddstr(textwin, ((i == 0 && isutf8) ? tabstr : " "));
						} else if(l->c[ichar] == ' ') { /* Space */
							wattrset(textwin, textattrs[SpcFG][ibg]);
							waddstr(textwin, (isutf8 ? spcstr : " "));
						} else if(ISCTRL(l->c[ichar])) {
							/* Add Ctrl-char as string to avoid problems at right screen end */
							wattrset(textwin, textattrs[CtrlFG][ibg]);
							waddstr(textwin, unctrl(l->c[ichar]));
						} else if(isutf8 && !ISASCII(l->c[ichar])) {
							/* Begin multi-byte char, dangerous at right screen end */
							for(i = 0; i < UTF8LEN(l->c[ichar]); i++) {
								if(ichar + i < l->len)
									c[i] = l->c[ichar + i];
								else
									c[i] = '\0';
							}
							c[i] = '\0'; /* WARNING: we use i later... */
							waddstr(textwin, c);
						} else {
							waddch(textwin, l->c[ichar]);
						}
						ivchar += VLEN(l->c[ichar], ivchar);
						if(isutf8 && !ISASCII(l->c[ichar]) && i)
							ichar += i; /* ...here */
						else
							ichar++;
					} else {
						ifg = DefFG, ibg = DefBG;
						if(fcur.l == l) {
							ifg = CurFG;
							ibg = CurBG;
						}
						if(selection) {
							ifg = SelFG;
							ibg = SelBG;
						}
						wattrset(textwin, textattrs[ifg][ibg]);
						waddch(textwin, ' ');
						ivchar++;
						ichar++;
					}
				}
			}
		} else if(l == fsel.l || l == fcur.l) {
			selection = !selection;
		}
		if(l)
			l = l->next;
	}

	/* Calculate nlst */
	for(iline = ncur, l = fcur.l; l; l = l->next, iline++) {
		if(l == lstline)
			nlst = iline;
	}

	/* Position cursor */
	wmove(textwin, cursor_r, cursor_c);

	/* Update env */
	snprintf(buf, 16, "%ld", ncur);
	setenv(envs[EnvLine], buf, 1);
	snprintf(buf, 16, "%d", (int) fcur.o);
	setenv(envs[EnvOffset], buf, 1);

	/* Update title */
	if(tmptitle) {
		snprintf(title, sizeof(title), "%s", tmptitle);
	} else {
		statusflags &= ~S_Warned;	/* Reset warning */
		snprintf(buf, 4, "%ld%%", (100 * ncur) / nlst);
		snprintf(title, BUFSIZ, "%s%s [%s]%s%s%s%s %ld,%d  %s",
		    t_vis()? "Visual " :
#if VIM_BINDINGS
		    (t_ins()? "Insert " : "Command "),
#else
		    "",
#endif /* VIM_BINDINGS */
		    (statusflags & S_DumpStdout ? "<Stdout>" : (filename == NULL ? "<No file>" : filename)),
		    (syntx >= 0 ? syntaxes[syntx].name : "none"),
		    (t_mod()? "[+]" : ""), (!t_rw()? "[RO]" : ""),
		    (statusflags & S_CaseIns ? "[icase]" : ""),
		    (statusflags & S_AutoIndent ? "[ai]" : ""), ncur,
		    (int) fcur.o,
		    (scrline == fstline ? (nlst <
			    lines3 ? "All" : "Top") : (nlst - nscr <
			    lines3 ? "Bot" : buf)
		    ));
	}
	if(titlewin) {
		wmove(titlewin, 0, 0);
		for(i = 0; i < cols; i++)
			waddch(titlewin, ' ');
		mvwaddnstr(titlewin, 0, 0, title, cols);
	} else {
		putp(tsl_str);
		putp(title);
		putp(fsl_str);
	}

	/* Clean global dirty bits */
	statusflags &= ~(S_DirtyScr | S_DirtyDown | S_NeedResize);
	tmptitle = NULL;

	/* And go.... */
	if(titlewin)
		wnoutrefresh(titlewin);
	wnoutrefresh(textwin);
	doupdate();
}

/* Print help, die */
void
i_usage(void) {
	i_die("sandy - simple editor\n"
	      "usage: sandy [-a] [-d] [-r] [-u] [-t TABSTOP] [-S] [-s SYNTAX] [file | -]\n");
}

/* Write buffer to disk */
bool
i_writefile(char *fname) {
	int fd = 1; /* default: write to stdout */
	bool wok = TRUE;
	Line *l;

	if(fname != NULL
	    && (fd = open(fname, O_WRONLY | O_TRUNC | O_CREAT,
		    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
		    S_IWOTH)) == -1) {
		/* error */
		tmptitle = "WARNING! Can't save file!!!";
		return FALSE;
	}

	for(l = fstline; wok && l; l = l->next) {
		if(write(fd, l->c, l->len) == -1 ||
		    (l->next && write(fd, "\n", 1) == -1))
			wok = FALSE;
	}
	if(fd != 1)
		close(fd);
	return wok;
}

/* M_* FUNCTIONS
	Represent a cursor motion, always take a Filepos and return an update Filepos */

/* Go to beginning of file */
Filepos
m_bof(Filepos pos) {
	pos.l = fstline;
	pos.o = 0;
	return pos;
}

/* Go to beginning of line */
Filepos
m_bol(Filepos pos) {
	pos.o = 0;
	return pos;
}

/* Go to smart beginning of line */
Filepos
m_smartbol(Filepos pos) {
	Filepos vbol = pos;

	vbol.o = 0;
	while(ISBLANK(vbol.l->c[vbol.o]) && ++vbol.o < vbol.l->len) ;
	if(pos.o != 0 && pos.o <= vbol.o)
		vbol.o = 0;
	return vbol;
}

/* Go to end of file */
Filepos
m_eof(Filepos pos) {
	pos.l = lstline;
	pos.o = pos.l->len;
	return pos;
}

/* Go to end of line */
Filepos
m_eol(Filepos pos) {
	pos.o = pos.l->len;
	return pos;
}

/* Advance one char, next line if needed */
Filepos
m_nextchar(Filepos pos) {
	if(pos.o < pos.l->len) {
		pos.o++;
		FIXNEXT(pos);
	} else if(pos.l->next) {
		pos.l = pos.l->next;
		pos.o = 0;
	}
	return pos;
}

/* Backup one char, previous line if needed */
Filepos
m_prevchar(Filepos pos) {
	if(pos.o > 0) {
		pos.o--;
		FIXPREV(pos);
	} else if(pos.l->prev) {
		pos.l = pos.l->prev;
		pos.o = pos.l->len;
	}
	return pos;
}

/* Advance one word, next line if needed */
Filepos
m_nextword(Filepos pos) {
	Filepos p0 = m_nextchar(pos);

	while((p0.o != pos.o || p0.l != pos.l) && ISWORDBRK(pos.l->c[pos.o]))
		pos = p0, p0 = m_nextchar(pos);	/* Find the current or next word */

	do {
		/* Move to word end */
		p0 = pos, pos = m_nextchar(pos);
	} while((p0.o != pos.o || p0.l != pos.l) && !ISWORDBRK(pos.l->c[pos.o]));
	return pos;
}

/* Backup one word, previous line if needed */
Filepos
m_prevword(Filepos pos) {
	Filepos p0 = m_prevchar(pos);

	if(ISWORDBRK(pos.l->c[pos.o]))
		while((p0.o != pos.o || p0.l != pos.l)
		    && ISWORDBRK(pos.l->c[pos.o]))
			pos = p0, p0 = m_prevchar(pos);	/* Find the current or previous word */
	else
		pos = p0;

	do {
		/* Move to word start */
		p0 = pos, pos = m_prevchar(pos);
	} while((p0.o != pos.o || p0.l != pos.l) && !ISWORDBRK(pos.l->c[pos.o]));
	return p0;
}

/* Advance one line, or to eol if at last line */
Filepos
m_nextline(Filepos pos) {
	size_t ivchar, ichar;

	for(ivchar = ichar = 0; ichar < pos.o; ichar++)
		ivchar += VLEN(pos.l->c[ichar], ivchar);

	if(pos.l->next) {
		/* Remember: here we re-use ichar as a second ivchar */
		for(pos.l = pos.l->next, pos.o = ichar = 0; ichar < ivchar && pos.o < pos.l->len; pos.o++)
			ichar += VLEN(pos.l->c[pos.o], ichar);
		FIXNEXT(pos);
	} else {
		pos.o = pos.l->len;
	}
	return pos;
}

/* Backup one line, or to bol if at first line */
Filepos
m_prevline(Filepos pos) {
	size_t ivchar, ichar;

	for(ivchar = ichar = 0; ichar < pos.o; ichar++)
		ivchar += VLEN(pos.l->c[ichar], (ivchar % (cols - 1)));

	if(pos.l->prev) {
		/* Remember: here we re-use ichar as a second ivchar */
		for(pos.l = pos.l->prev, pos.o = ichar = 0; ichar < ivchar && pos.o < pos.l->len; pos.o++)
			ichar += VLEN(pos.l->c[pos.o], ichar);
		FIXNEXT(pos);
	} else {
		pos.o = 0;
	}
	return pos;
}

/* Advance as many lines as the screen size */
Filepos
m_nextscr(Filepos pos) {
	int i;
	Line *l;

	for(i = LINESABS, l = pos.l; l->next && i > 0; i -= VLINES(l), l = l->next)
		;
	pos.l = l;
	pos.o = pos.l->len;
	return pos;
}

/* Go to where the adjective says */
Filepos
m_parameter(Filepos pos) {
	/* WARNING: this code is actually not used */
	return pos;
}

/* Backup as many lines as the screen size */
Filepos
m_prevscr(Filepos pos) {
	int i;
	Line *l;

	for(i = LINESABS, l = pos.l; l->prev && i > 0; i -= VLINES(l), l = l->prev)
		;
	pos.l = l;
	pos.o = 0;
	return pos;
}

/* Go to where the adjective says */
Filepos
m_sentence(Filepos pos) {
	/* WARNING: this code is actually not used */
	return pos;
}

/* Do not move */
Filepos
m_stay(Filepos pos) {
	return pos;
}

/* Go to mark if valid, stay otherwise */
Filepos
m_tomark(Filepos pos) {
	/* Be extra careful when moving to mark, as it might not exist */
	Line *l;
	for(l = fstline; l; l = l->next) {
		if(l != NULL && l == fmrk.l) {
			pos.l = fmrk.l;
			pos.o = fmrk.o;
			if(pos.o > pos.l->len)
				pos.o = pos.l->len;
			FIXNEXT(pos);
			f_mark(NULL);
			break;
		}
	}
	return pos;
}

/* Go to selection point */
Filepos
m_tosel(Filepos pos) {
	(void) pos;

	return fsel;
}

/* T_* FUNCTIONS
 * Used to test for conditions, take no arguments and return bool. */

/* TRUE if autoindent is on */
bool
t_ai(void) {
	return (statusflags & S_AutoIndent);
}

/* TRUE at the beginning of line */
bool
t_bol(void) {
	return (fcur.o == 0);
}

/* TRUE at end of line */
bool
t_eol(void) {
	return (fcur.o == fcur.l->len);
}

/* TRUE if the file has been modified */
bool
t_mod(void) {
	return (statusflags & S_Modified);
}

/* TRUE if we are not in command mode */
bool
t_ins(void) {
#if VIM_BINDINGS
	return !(statusflags & S_Command);
#else
	return TRUE;
#endif /* VIM_BINDINGS */
}

/* TRUE if the file is writable */
bool
t_rw(void) {
	return !(statusflags & S_Readonly);
}

/* TRUE if there is anything to redo */
bool
t_redo(void) {
	return (redos != NULL);
}

/* TRUE if any text is selected */
bool
t_sel(void) {
	return !(fcur.l == fsel.l && fcur.o == fsel.o);
}

/* TRUE if a sentence has started */
bool
t_sent(void) {
#if VIM_BINDINGS
	return (statusflags & S_Sentence);
#else
	return FALSE;
#endif /* VIM_BINDINGS */
}

/* TRUE if there is anything to undo */
bool
t_undo(void) {
	return (undos != NULL);
}

/* TRUE if we are in visual mode */
bool
t_vis(void) {
	return (statusflags & S_Visual);
}

/* TRUE if we have warned the file is modified */
bool
t_warn(void) {
	return (statusflags & S_Warned);
}

/* main() starts everything else */
int
main(int argc, char *argv[]) {
	char *local_syn = NULL;

	/* Use system locale, hopefully UTF-8 */
	setlocale(LC_ALL, "");

	ARGBEGIN {
	case 'r':
		statusflags |= S_Readonly;
		break;
	case 'a':
		statusflags |= S_AutoIndent;
		break;
	case 'd':
		statusflags |= S_DumpStdout;
		break;
	case 't':
		tabstop = atoi(EARGF(i_usage()));
		break;
	case 'S':
		local_syn = "";
		break;
	case 's':
		local_syn = EARGF(i_usage());
		break;
	case 'v':
		i_die("sandy-" VERSION ", © 2014 sandy engineers, see LICENSE for details\n");
		break;
	default:
		i_usage();
		break;
	} ARGEND;

	i_setup();

	if(argc > 0)
		i_readfile(argv[0]);

	if(local_syn)
		f_syntax(&(const Arg) { .v = local_syn });

	i_edit();
	i_cleanup(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

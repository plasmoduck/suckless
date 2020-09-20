#define __USE_MACROS
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>

/*
 * This test assumes an ascii representation
 */

#define TESTW(f) fputs(#f ":" , stdout); \
	for (i = 0; i <= UCHAR_MAX; i++)  \
		if (f(i)) printf(" %d", i); \
	putchar('\n')

#define TESTC(f) fputs(#f ": " , stdout); \
	for (i = 0; i <= UCHAR_MAX; i++)  \
		if (f(i)) putchar(i); \
	putchar('\n')

#define TESTEOF(f) fputs(#f ": " , stdout); \
	if (!f(EOF)) putchar('!'); puts("EOF");

#define TESTLU(f) \
	for (i = 0; i < UCHAR_MAX; i++) { \
		n = f(i); \
		if (!isgraph(i)) \
			continue; \
		printf("%s: %c <-> %c\n", #f, i, n); \
	}

void
test1()
{
	int i;

	puts("\ntest1");
	TESTC(isalnum);
	TESTC(isalpha);
	TESTC(isdigit);
	TESTC(isgraph);
	TESTC(islower);
	TESTC(isupper);
	TESTC(isprint);
	TESTC(ispunct);
	TESTC(isxdigit);
	TESTC(isdigit);
	TESTW(iscntrl);
	TESTW(isspace);
	TESTEOF(isalpha);
	TESTEOF(isdigit);
	TESTEOF(isgraph);
	TESTEOF(islower);
	TESTEOF(isupper);
	TESTEOF(isprint);
	TESTEOF(ispunct);
	TESTEOF(isxdigit);
	TESTEOF(isdigit);
	TESTEOF(iscntrl);
	TESTEOF(isspace);
}

#undef isalnum
#undef isalpha
#undef isdigit
#undef isgraph
#undef islower
#undef isupper
#undef isprint
#undef ispunct
#undef isxdigit
#undef isdigit

void
test2()
{
	int i;

	puts("\ntest2");
	TESTC(isalnum);
	TESTC(isalpha);
	TESTC(isdigit);
	TESTC(isgraph);
	TESTC(islower);
	TESTC(isupper);
	TESTC(isprint);
	TESTC(ispunct);
	TESTC(isxdigit);
	TESTC(isdigit);
	TESTW(iscntrl);
	TESTW(isspace);
	TESTEOF(isalpha);
	TESTEOF(isdigit);
	TESTEOF(isgraph);
	TESTEOF(islower);
	TESTEOF(isupper);
	TESTEOF(isprint);
	TESTEOF(ispunct);
	TESTEOF(isxdigit);
	TESTEOF(isdigit);
	TESTEOF(iscntrl);
	TESTEOF(isspace);
}

void test3()
{
	int i, n;

	puts("\ntest3");
	TESTLU(tolower);
	TESTLU(toupper);
}	

#undef tolower
#undef toupper

void test4()
{
	int i, n;

	puts("\ntest4");
	TESTLU(tolower);
	TESTLU(toupper);
	assert(tolower(EOF) == EOF);
	assert(toupper(EOF) == EOF);
}

int
main()
{
	test1();
	test2();
	test3();
	test4();

	return 0;
}

/*
output:

test1
isalnum: 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
isalpha: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
isdigit: 0123456789
isgraph: !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
islower: abcdefghijklmnopqrstuvwxyz
isupper: ABCDEFGHIJKLMNOPQRSTUVWXYZ
isprint:  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
ispunct: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
isxdigit: 0123456789ABCDEFabcdef
isdigit: 0123456789
iscntrl: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 127
isspace: 9 10 11 12 13 32
isalpha: !EOF
isdigit: !EOF
isgraph: !EOF
islower: !EOF
isupper: !EOF
isprint: !EOF
ispunct: !EOF
isxdigit: !EOF
isdigit: !EOF
iscntrl: !EOF
isspace: !EOF

test2
isalnum: 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
isalpha: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
isdigit: 0123456789
isgraph: !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
islower: abcdefghijklmnopqrstuvwxyz
isupper: ABCDEFGHIJKLMNOPQRSTUVWXYZ
isprint:  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
ispunct: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~
isxdigit: 0123456789ABCDEFabcdef
isdigit: 0123456789
iscntrl: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 127
isspace: 9 10 11 12 13 32
isalpha: !EOF
isdigit: !EOF
isgraph: !EOF
islower: !EOF
isupper: !EOF
isprint: !EOF
ispunct: !EOF
isxdigit: !EOF
isdigit: !EOF
iscntrl: !EOF
isspace: !EOF

test3
tolower: ! <-> !
tolower: " <-> "
tolower: # <-> #
tolower: $ <-> $
tolower: % <-> %
tolower: & <-> &
tolower: ' <-> '
tolower: ( <-> (
tolower: ) <-> )
tolower: * <-> *
tolower: + <-> +
tolower: , <-> ,
tolower: - <-> -
tolower: . <-> .
tolower: / <-> /
tolower: 0 <-> 0
tolower: 1 <-> 1
tolower: 2 <-> 2
tolower: 3 <-> 3
tolower: 4 <-> 4
tolower: 5 <-> 5
tolower: 6 <-> 6
tolower: 7 <-> 7
tolower: 8 <-> 8
tolower: 9 <-> 9
tolower: : <-> :
tolower: ; <-> ;
tolower: < <-> <
tolower: = <-> =
tolower: > <-> >
tolower: ? <-> ?
tolower: @ <-> @
tolower: A <-> a
tolower: B <-> b
tolower: C <-> c
tolower: D <-> d
tolower: E <-> e
tolower: F <-> f
tolower: G <-> g
tolower: H <-> h
tolower: I <-> i
tolower: J <-> j
tolower: K <-> k
tolower: L <-> l
tolower: M <-> m
tolower: N <-> n
tolower: O <-> o
tolower: P <-> p
tolower: Q <-> q
tolower: R <-> r
tolower: S <-> s
tolower: T <-> t
tolower: U <-> u
tolower: V <-> v
tolower: W <-> w
tolower: X <-> x
tolower: Y <-> y
tolower: Z <-> z
tolower: [ <-> [
tolower: \ <-> \
tolower: ] <-> ]
tolower: ^ <-> ^
tolower: _ <-> _
tolower: ` <-> `
tolower: a <-> a
tolower: b <-> b
tolower: c <-> c
tolower: d <-> d
tolower: e <-> e
tolower: f <-> f
tolower: g <-> g
tolower: h <-> h
tolower: i <-> i
tolower: j <-> j
tolower: k <-> k
tolower: l <-> l
tolower: m <-> m
tolower: n <-> n
tolower: o <-> o
tolower: p <-> p
tolower: q <-> q
tolower: r <-> r
tolower: s <-> s
tolower: t <-> t
tolower: u <-> u
tolower: v <-> v
tolower: w <-> w
tolower: x <-> x
tolower: y <-> y
tolower: z <-> z
tolower: { <-> {
tolower: | <-> |
tolower: } <-> }
tolower: ~ <-> ~
toupper: ! <-> !
toupper: " <-> "
toupper: # <-> #
toupper: $ <-> $
toupper: % <-> %
toupper: & <-> &
toupper: ' <-> '
toupper: ( <-> (
toupper: ) <-> )
toupper: * <-> *
toupper: + <-> +
toupper: , <-> ,
toupper: - <-> -
toupper: . <-> .
toupper: / <-> /
toupper: 0 <-> 0
toupper: 1 <-> 1
toupper: 2 <-> 2
toupper: 3 <-> 3
toupper: 4 <-> 4
toupper: 5 <-> 5
toupper: 6 <-> 6
toupper: 7 <-> 7
toupper: 8 <-> 8
toupper: 9 <-> 9
toupper: : <-> :
toupper: ; <-> ;
toupper: < <-> <
toupper: = <-> =
toupper: > <-> >
toupper: ? <-> ?
toupper: @ <-> @
toupper: A <-> A
toupper: B <-> B
toupper: C <-> C
toupper: D <-> D
toupper: E <-> E
toupper: F <-> F
toupper: G <-> G
toupper: H <-> H
toupper: I <-> I
toupper: J <-> J
toupper: K <-> K
toupper: L <-> L
toupper: M <-> M
toupper: N <-> N
toupper: O <-> O
toupper: P <-> P
toupper: Q <-> Q
toupper: R <-> R
toupper: S <-> S
toupper: T <-> T
toupper: U <-> U
toupper: V <-> V
toupper: W <-> W
toupper: X <-> X
toupper: Y <-> Y
toupper: Z <-> Z
toupper: [ <-> [
toupper: \ <-> \
toupper: ] <-> ]
toupper: ^ <-> ^
toupper: _ <-> _
toupper: ` <-> `
toupper: a <-> A
toupper: b <-> B
toupper: c <-> C
toupper: d <-> D
toupper: e <-> E
toupper: f <-> F
toupper: g <-> G
toupper: h <-> H
toupper: i <-> I
toupper: j <-> J
toupper: k <-> K
toupper: l <-> L
toupper: m <-> M
toupper: n <-> N
toupper: o <-> O
toupper: p <-> P
toupper: q <-> Q
toupper: r <-> R
toupper: s <-> S
toupper: t <-> T
toupper: u <-> U
toupper: v <-> V
toupper: w <-> W
toupper: x <-> X
toupper: y <-> Y
toupper: z <-> Z
toupper: { <-> {
toupper: | <-> |
toupper: } <-> }
toupper: ~ <-> ~

test4
tolower: ! <-> !
tolower: " <-> "
tolower: # <-> #
tolower: $ <-> $
tolower: % <-> %
tolower: & <-> &
tolower: ' <-> '
tolower: ( <-> (
tolower: ) <-> )
tolower: * <-> *
tolower: + <-> +
tolower: , <-> ,
tolower: - <-> -
tolower: . <-> .
tolower: / <-> /
tolower: 0 <-> 0
tolower: 1 <-> 1
tolower: 2 <-> 2
tolower: 3 <-> 3
tolower: 4 <-> 4
tolower: 5 <-> 5
tolower: 6 <-> 6
tolower: 7 <-> 7
tolower: 8 <-> 8
tolower: 9 <-> 9
tolower: : <-> :
tolower: ; <-> ;
tolower: < <-> <
tolower: = <-> =
tolower: > <-> >
tolower: ? <-> ?
tolower: @ <-> @
tolower: A <-> a
tolower: B <-> b
tolower: C <-> c
tolower: D <-> d
tolower: E <-> e
tolower: F <-> f
tolower: G <-> g
tolower: H <-> h
tolower: I <-> i
tolower: J <-> j
tolower: K <-> k
tolower: L <-> l
tolower: M <-> m
tolower: N <-> n
tolower: O <-> o
tolower: P <-> p
tolower: Q <-> q
tolower: R <-> r
tolower: S <-> s
tolower: T <-> t
tolower: U <-> u
tolower: V <-> v
tolower: W <-> w
tolower: X <-> x
tolower: Y <-> y
tolower: Z <-> z
tolower: [ <-> [
tolower: \ <-> \
tolower: ] <-> ]
tolower: ^ <-> ^
tolower: _ <-> _
tolower: ` <-> `
tolower: a <-> a
tolower: b <-> b
tolower: c <-> c
tolower: d <-> d
tolower: e <-> e
tolower: f <-> f
tolower: g <-> g
tolower: h <-> h
tolower: i <-> i
tolower: j <-> j
tolower: k <-> k
tolower: l <-> l
tolower: m <-> m
tolower: n <-> n
tolower: o <-> o
tolower: p <-> p
tolower: q <-> q
tolower: r <-> r
tolower: s <-> s
tolower: t <-> t
tolower: u <-> u
tolower: v <-> v
tolower: w <-> w
tolower: x <-> x
tolower: y <-> y
tolower: z <-> z
tolower: { <-> {
tolower: | <-> |
tolower: } <-> }
tolower: ~ <-> ~
toupper: ! <-> !
toupper: " <-> "
toupper: # <-> #
toupper: $ <-> $
toupper: % <-> %
toupper: & <-> &
toupper: ' <-> '
toupper: ( <-> (
toupper: ) <-> )
toupper: * <-> *
toupper: + <-> +
toupper: , <-> ,
toupper: - <-> -
toupper: . <-> .
toupper: / <-> /
toupper: 0 <-> 0
toupper: 1 <-> 1
toupper: 2 <-> 2
toupper: 3 <-> 3
toupper: 4 <-> 4
toupper: 5 <-> 5
toupper: 6 <-> 6
toupper: 7 <-> 7
toupper: 8 <-> 8
toupper: 9 <-> 9
toupper: : <-> :
toupper: ; <-> ;
toupper: < <-> <
toupper: = <-> =
toupper: > <-> >
toupper: ? <-> ?
toupper: @ <-> @
toupper: A <-> A
toupper: B <-> B
toupper: C <-> C
toupper: D <-> D
toupper: E <-> E
toupper: F <-> F
toupper: G <-> G
toupper: H <-> H
toupper: I <-> I
toupper: J <-> J
toupper: K <-> K
toupper: L <-> L
toupper: M <-> M
toupper: N <-> N
toupper: O <-> O
toupper: P <-> P
toupper: Q <-> Q
toupper: R <-> R
toupper: S <-> S
toupper: T <-> T
toupper: U <-> U
toupper: V <-> V
toupper: W <-> W
toupper: X <-> X
toupper: Y <-> Y
toupper: Z <-> Z
toupper: [ <-> [
toupper: \ <-> \
toupper: ] <-> ]
toupper: ^ <-> ^
toupper: _ <-> _
toupper: ` <-> `
toupper: a <-> A
toupper: b <-> B
toupper: c <-> C
toupper: d <-> D
toupper: e <-> E
toupper: f <-> F
toupper: g <-> G
toupper: h <-> H
toupper: i <-> I
toupper: j <-> J
toupper: k <-> K
toupper: l <-> L
toupper: m <-> M
toupper: n <-> N
toupper: o <-> O
toupper: p <-> P
toupper: q <-> Q
toupper: r <-> R
toupper: s <-> S
toupper: t <-> T
toupper: u <-> U
toupper: v <-> V
toupper: w <-> W
toupper: x <-> X
toupper: y <-> Y
toupper: z <-> Z
toupper: { <-> {
toupper: | <-> |
toupper: } <-> }
toupper: ~ <-> ~
end:
*/

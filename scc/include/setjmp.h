#ifndef _SETJMP_H
#define _SETJMP_H

#include <arch/setjmp.h>

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

#define setjmp setjmp

#endif

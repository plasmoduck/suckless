#!/bin/sh

awk '! /^#/ && $2 == "'$1'" {
	syscall=$2
	fname=$2".s"

	printf ("\t.file\t"	\
	       "\"%s\"\n"	\
	       "\t.globl\t%s\n"	\
	       "%s:\n",
	       fname, syscall, syscall)

	printf ("\tli\t0,%d\n"	\
	       "\tsc\n"		\
	       "\tmfcr\t0\n"	\
	       "\tb\t_cerrno\n", $1)
} ' syscall.lst >$1.s

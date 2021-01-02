#!/bin/sh
rm -f *.o tests
cc tests.c -o tests -O2 -Wall
SURF_ADBLOCK_FILE=`pwd`/rules ./tests

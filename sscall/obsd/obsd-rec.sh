#!/bin/sh
#
# Sample OpenBSD recording script
# Pipe this over to sscall

aucat -r 16000 -c0:0 -e s16le -o -

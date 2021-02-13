#!/bin/sh
#
# Sample Linux ALSA recording script
# Pipe this over to sscall

arecord -r 16000 -f S16_LE -c 1 -D default

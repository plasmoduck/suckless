#!/bin/sh

awk '/define[ 	]*E/ && $3 ~ /[0-9]+/ && $3 > 0 {
	sub(/\#define[ 	]*/, "")
	sub(/\/\*/, "")
	sub(/\*\//, "")
	print
}' /usr/include/sys/errno.h

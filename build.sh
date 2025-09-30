#!/bin/sh

if ! $(test -f config.h)
then
	echo "config.h created"
	cp config.def.h config.h
fi

gcc -O2 -ansi -std=c89 -Wall -Wextra -Wno-sign-compare -Wno-pointer-arith -I3rd-party tr.c -o tr -lzip


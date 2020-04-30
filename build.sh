#! /bin/sh

# $1 is the first argument.
# ${1%.*} means "strip the file extension, if any, from argument 1."
# I mean, clearly. isn't bash SO readable? 🙄

gcc -Wall -Werror -lpthread --std=c99 -g -o ${1%.*} ${@:2} ${1%.*}.c

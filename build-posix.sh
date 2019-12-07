#!/bin/sh

set -xe

CFLAGS="$CFLAGS -Wall -Wextra -Wconversion -Wunused -Wunused-function -Wunused-label -Wunused-macros -Wunused-parameter -Wunused-value -Wunused-variable -Wcast-align -Wcast-qual -Wmissing-declarations -Wredundant-decls -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wwrite-strings -Wswitch -Wmissing-field-initializers -fno-common -fno-strict-aliasing -pedantic -std=c11 -ggdb -O3 $(pkg-config --cflags sdl2) -I./src/"

if [ ! -z "$NOTHING_CI" ]; then
    CFLAGS="$CFLAGS -Werror"
fi

if [ ! -z "$NOTHING_PROFILE" ]; then
    CFLAGS="$CFLAGS -pg"
fi

LIBS="$LIBS $(pkg-config --libs sdl2) -lm"

if [ ${#CC} -eq "0" ]; then
    echo "CC variable is not set up!"
    exit 1
fi

$CC $CFLAGS -o nothing nothing.c $LIBS

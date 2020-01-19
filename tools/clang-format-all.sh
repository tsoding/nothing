#!/bin/sh

find ./src/ -type f \( -name \*.c -or -name \*.h \) -exec clang-format -verbose -i {} \;

#!/bin/sh

./tools/clang-format-all.sh

if [ ! -z "$(git --no-pager diff)" ]; then
    echo "------------------------------"
    echo "FORMATTING IS INCORRECT"
    echo "------------------------------"
    git --no-pager diff
    exit 1
fi

#!/bin/sh

./tools/clang-format-all.sh

if [ ! -z $(git diff) ]; do
    echo "------------------------------"
    echo "FORMATTING IS INCORRECT"
    echo "------------------------------"
    git diff
done

#!/bin/sh

clear

[ ! -d "build" ] && mkdir build
pushd build

clang -Wall -g -o breakout ../src/breakout.c -lSDL2
success=$?
popd

exit $success


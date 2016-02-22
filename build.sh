#!/bin/sh

cc=clang
src=`pwd`/src/breakout.c

clear

[ ! -d "build" ] && mkdir build

pushd build

$cc -std=c11 -W -Wall -g $src -lSDL2

success=$?

popd

exit $success


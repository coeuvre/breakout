#!/bin/sh

cc=clang
src=`pwd`/src/breakout.c

clear

[ ! -d "build" ] && mkdir build

pushd build

$cc -o breakout -std=c11 -W -Wall -g $src -lSDL2

success=$?

popd

exit $success


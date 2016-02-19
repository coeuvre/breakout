#!/bin/sh

clear

[ ! -d "build" ] && mkdir build

pushd build

clang -W -Wall ../src/breakout.c -lSDL2 -g

success=$?

popd

exit $success



#!/bin/sh

clear

[ ! -d "build" ] && mkdir build

pushd build

[ ! -f "Makefile" ] && cmake ..

make

success=$?

popd

exit $success


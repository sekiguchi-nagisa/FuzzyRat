#!/bin/sh

mkdir -p build-asan
cd build-asan
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
         -DCMAKE_BUILD_TYPE=debug \
         -DSANITIZER=address

make -j2

ctest

#! /bin/bash

if [ -d ./build ]; then
    rm -r ./build
fi

mkdir build
(cd build && cmake ..)
(cd build && make)
cp ./tests/run.py ./build/tests/
cp -r ./tests/str_tests ./build/tests/
cp -r ./tests/fail_tests ./build/tests/
cp -r ./tests/full_tests ./build/tests/
cp -r ./tests/list_tests ./build/tests/
cp -r ./tests/object_tests ./build/tests/

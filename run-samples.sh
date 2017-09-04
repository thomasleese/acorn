#!/bin/sh
set -e

mkdir -p build
pushd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
popd

pushd samples
python3 run.py
popd

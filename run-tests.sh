#!/bin/sh
set -e

pushd build
make
popd

pushd tests
python3 run.py
popd

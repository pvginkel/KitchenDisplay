#!/bin/sh

cd "$(dirname "$0")/.."

set -e

mkdir -p build

cd build

if [ "$1" = "debug" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  make -j
else
  cmake ..
  make
fi

#!/bin/sh

cd "$(dirname "$0")/.."

set -e

if [ ! -d build ]; then mkdir build; fi

cd build

rm CMakeCache.txt

if grep -q 'Raspbian' /etc/os-release; then
  cmake ..
  make
else
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  make -j
fi

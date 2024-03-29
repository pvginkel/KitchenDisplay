#!/bin/sh

set -e

if [ ! -d build ]; then mkdir build; fi

cd build
if grep -q 'Raspbian' /etc/os-release; then
  cmake ..
  make
else
  cmake -DCMAKE_BUILD_TYPE=Debug ..
  make -j
fi

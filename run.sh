#!/bin/sh

set -e

if [ ! -d build ]; then mkdir build; fi

cd build
cmake ..
make -j

cd ../bin

sudo fbset -fb /dev/fb0 -g 1024 600 1024 600 32
sudo ./main

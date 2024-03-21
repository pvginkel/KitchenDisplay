#!/bin/sh

set -e

./build.sh

cd bin

sudo fbset -fb /dev/fb0 -g 1024 600 1024 600 32
sudo ./main

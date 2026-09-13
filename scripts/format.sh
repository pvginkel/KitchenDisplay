#!/bin/sh

cd "$(dirname "$0")"

find ../src -type f \( -iname '*.c' -o -iname '*.cpp' -o -iname '*.h' \) -exec clang-format -i {} +

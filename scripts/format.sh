#!/bin/sh

# Formats src/ in place. Arguments replace -i, so `format.sh --dry-run --Werror`
# checks the same files without changing them.

cd "$(dirname "$0")"

[ $# -eq 0 ] && set -- -i

find ../src -type f \( -iname '*.c' -o -iname '*.cpp' -o -iname '*.h' \) -exec clang-format "$@" {} +

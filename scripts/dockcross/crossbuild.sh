#!/bin/sh

set -e

cd "$(dirname "$0")/../.."

ROOT="$(pwd)"

build_libbacktrace() {
    echo "Building libbacktrace..."

    cd $ROOT/lib/libbacktrace
    mkdir -p $ROOT/build/lib/libbacktrace

    ./configure --build "$(gcc -dumpmachine)" --host "$CROSS_TRIPLE" --prefix=$ROOT/build/lib/libbacktrace
    make -j$(nproc)
    make -j$(nproc) install
}

build_icu_host() {
    echo "Building icu..."

    mkdir -p $ROOT/build/lib/icu-host

    cd $ROOT/build/lib/icu-host
    $ROOT/lib/icu/icu4c/source/runConfigureICU Linux
    make -j$(nproc)
}

build_icu() {
    echo "Building icu..."

    mkdir -p $ROOT/build/lib/icu-cross $ROOT/build/lib/icu
    cd $ROOT/build/lib/icu-cross
    
    $ROOT/lib/icu/icu4c/source/runConfigureICU Linux --host="$CROSS_TRIPLE" --prefix=$ROOT/build/lib/icu --with-cross-build=$ROOT/build/lib/icu-host
    make -j$(nproc)
    make -j$(nproc) install
}

build_app() {
    echo "Building app..."

    cd $ROOT/build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j$(nproc)
}

main() {
    mkdir -p $ROOT/build

    if [ "$1" = "prerequisites" ]; then 
        build_icu_host
    else
        build_libbacktrace
        build_icu
        build_app
    fi
}

main "$@"

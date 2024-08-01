#!/bin/sh

set -e

cd "$(dirname "$0")/../.."

ROOT="$(pwd)"

build_libbacktrace() {
    if [ -d $ROOT/build/lib/libbacktrace ]; then
        return
    fi

    echo "Building libbacktrace..."

    cd $ROOT/lib/libbacktrace
    mkdir -p $ROOT/build/lib/libbacktrace

    ./configure --build "$(gcc -dumpmachine)" --host "$CROSS_TRIPLE" --prefix=$ROOT/build/lib/libbacktrace
    make -j$(nproc)
    make -j$(nproc) install
}

build_icu_host() {
    if [ -d $ROOT/build/lib/icu ]; then
        return
    fi

    echo "Building icu..."

    mkdir -p $ROOT/build/lib/icu-host

    cd $ROOT/build/lib/icu-host
    $ROOT/lib/icu/icu4c/source/runConfigureICU Linux
    make -j$(nproc)
}

build_icu() {
    if [ -d $ROOT/build/lib/icu ]; then
        return
    fi

    echo "Building icu..."

    mkdir -p $ROOT/build/lib/icu-cross $ROOT/build/lib/icu
    cd $ROOT/build/lib/icu-cross
    
    $ROOT/lib/icu/icu4c/source/runConfigureICU Linux --host="$CROSS_TRIPLE" --prefix=$ROOT/build/lib/icu --with-cross-build=$ROOT/build/lib/icu-host
    make -j$(nproc)
    make -j$(nproc) install
}

build_curl() {
    if [ -d $ROOT/build/lib/curl ]; then
        return
    fi

    echo "Building cURL"

    cd $ROOT/lib/curl
    mkdir -p $ROOT/build/lib/curl
    
    mkdir -p build
    cd build
    cmake -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CURL_EXE=OFF -DCMAKE_INSTALL_PREFIX:PATH=$ROOT/build/lib/curl ..
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
        build_curl
        build_icu
        build_app
    fi
}

main "$@"

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

    mkdir -p $ROOT/build/lib/icu-cross
    cd $ROOT/build/lib/icu-cross
    
    $ROOT/lib/icu/icu4c/source/runConfigureICU Linux --host="$CROSS_TRIPLE" --prefix=$ROOT/build/lib/icu --with-cross-build=$ROOT/build/lib/icu-host
    make -j$(nproc)
    make -j$(nproc) install
}

build_openssl() {
    if [ -d $ROOT/build/lib/openssl ]; then
        return
    fi

    echo "Building openssl..."

    mkdir -p $ROOT/lib/openssl/build
    cd $ROOT/lib/openssl/build

    $ROOT/lib/openssl/Configure linux-aarch64 --cross-compile-prefix= --prefix=$ROOT/build/lib/openssl
    make -j$(nproc) build_sw
    make -j$(nproc) install_sw
}

build_zlib() {
    if [ -d $ROOT/build/lib/zlib ]; then
        return
    fi

    echo "Building zlib"

    mkdir -p $ROOT/lib/zlib/build
    cd $ROOT/lib/zlib/build

    cmake -DCMAKE_INSTALL_PREFIX:PATH=$ROOT/build/lib/zlib ..
    make -j$(nproc) install
}

build_curl() {
    if [ -d $ROOT/build/lib/curl ]; then
        return
    fi

    echo "Building cURL"

    mkdir -p $ROOT/lib/curl/build
    cd $ROOT/lib/curl/build

    cmake \
        -DBUILD_STATIC_LIBS=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_CURL_EXE=OFF \
        -DOPENSSL_ROOT_DIR=$ROOT/build/lib/openssl \
        -DZLIB_ROOT=$ROOT/build/lib/zlib \
        -DCMAKE_INSTALL_PREFIX:PATH=$ROOT/build/lib/curl \
        ..
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
        build_openssl
        build_zlib
        build_curl
        build_icu
        build_app
    fi
}

main "$@"

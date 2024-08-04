#!/bin/sh

set -e

do_crossbuild() {
    LAST_ID=$(docker ps -q --filter ancestor=$0)

    if [ ! -z $LAST_ID ]; then
        docker stop $LAST_ID
    fi

    docker run -it --rm \
        -v $(pwd)/../..:/work \
        -e BUILDER_UID=$(id -u) \
        -e BUILDER_GID=$(id -g) \
        -e BUILDER_USER="$(id -un)" \
        -e BUILDER_GROUP="$(id -gn)" \
        "$@"

}

cd "$(dirname "$0")"

do_crossbuild dockbuild/ubuntu2104 scripts/dockcross/crossbuild.sh prerequisites
do_crossbuild dockcross/linux-arm64 scripts/dockcross/crossbuild.sh

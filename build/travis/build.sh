#!/bin/bash

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

case "$DOCKER_IMAGE" in
    mingw-gtk2)
        ${SOURCES_DIR}/build/travis/build-mingw.sh
        ;;

    *)
        ${SOURCES_DIR}/build/travis/build-linux.sh
        ;;
esac

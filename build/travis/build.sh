#!/bin/bash

ROOT_DIR=/workspace
SOURCES_DIR=${ROOT_DIR}/source

case "$DOCKER_IMAGE" in
    mingw-gtk2)
        ${SOURCES_DIR}/build/travis/build-mingw.sh
        ;;

    *)
        ${SOURCES_DIR}/build/travis/build-linux.sh
        ;;
esac

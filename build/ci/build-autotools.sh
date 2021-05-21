#!/bin/bash -e

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

case "$DOCKER_IMAGE" in
    mingw-gtk2)
        ${CI_DIR}/build-autotools-mingw.sh
        ;;

    mingw-gtk*)
        ${CI_DIR}/build-autotools-mingw.sh
        ;;

    *)
        ${CI_DIR}/build-autotools-linux.sh
        ;;
esac

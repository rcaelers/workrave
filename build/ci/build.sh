#!/bin/bash -xe

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

case "$DOCKER_IMAGE" in
    mingw-gtk2)
        ${CI_DIR}/build-mingw.sh
        ;;

    *)
        ${CI_DIR}/build-linux.sh
        ;;
esac

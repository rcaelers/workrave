#!/bin/bash

IMAGE=
CONFIG=
BUILD_ARGS=()
DOCKER_ARGS=()
BUILT_TYPE=RelWithDebInfo
WORKING_DIR=
OUTPUT_DIR=
CONF_COMPILER=
CONF_CONFIGURATION=Release
CONF_APPIMAGE=
DOSHELL=

ROOT=$(git rev-parse --show-toplevel)
if [ $? -ne 0 ]; then
    echo "Not inside git working directory"
    exit 1
fi

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {
    while getopts "Ac:C:dD:O:STvW:" o; do
        case "${o}" in
        A)
            CONF_APPIMAGE=1
            ;;
        c)
            CONFIG="${OPTARG}"
            BUILD_ARGS+=("-d${CONFIG}")
            ;;
        C)
            CONF_COMPILER="${OPTARG}"
            ;;
        d)
            BUILD_TYPE=Debug
            CONF_CONFIGURATION=Debug
            BUILD_ARGS+=("-DWITH_TRACING=ON")
            ;;
        D)
            BUILD_ARGS+=("-D${OPTARG}")
            ;;
        O)
            OUTPUT_DIR="${OPTARG}"
            ;;
        W)
            WORKING_DIR="${OPTARG}"
            ;;
        S)
            DOSHELL="1"
            ;;
        T)
            BUILD_ARGS+=("-DWITH_TESTS=ON")
            # BUILD_ARGS+=("-DCODE_COVERAGE=ON")
            ;;
        v)
            BUILD_ARGS+=("-M\"-v\"")
            ;;
        *)
            usage
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

parse_arguments $*

case "$CONFIG" in
mingw-gtk-vs)
    IMAGE=mingw-gtk
    ;;
*)
    IMAGE=$CONFIG
    ;;
esac

if [[ -z "$CONF_COMPILER" ]]; then
    if [[ $IMAGE =~ "mingw" ]]; then
        CONF_COMPILER=clang
    else
        CONF_COMPILER=gcc
    fi
fi

BUILD_ARGS+=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")
BUILD_ARGS+=("-M\"-j4\"")
DOCKER_ARGS+=("-v ${ROOT}/:/workspace/source")

if [ -n "${WORKING_DIR}" ]; then
    if [ ! -d "${WORKING_DIR}" ]; then
        mkdir -p "${WORKING_DIR}"
    fi

    DOCKER_ARGS+=("-v ${WORKING_DIR}:/workspace/build")

    REL_DIR=$(git rev-parse --show-prefix)
    if [ $? -eq 0 -a -n "${REL_DIR}" ]; then
        BUILD_ARGS+=("-C${REL_DIR}")
    fi
fi

if [ -n "${OUTPUT_DIR}" ]; then
    if [ ! -d "${OUTPUT_DIR}" ]; then
        mkdir -p "${OUTPUT_DIR}"
    fi

    DOCKER_ARGS+=("-v ${OUTPUT_DIR}:/workspace/output")
fi

DOCKER_ARGS+=("-e WORKRAVE_ENV=docker-linux")
DOCKER_ARGS+=("-e CONF_COMPILER=${CONF_COMPILER}")
DOCKER_ARGS+=("-e CONF_CONFIGURATION=${CONF_CONFIGURATION}")
DOCKER_ARGS+=("-e CONF_APPIMAGE=${CONF_APPIMAGE}")
DOCKER_ARGS+=("--rm ghcr.io/rcaelers/workrave-build:${IMAGE}")

if [[ $DOSHELL ]]; then
    docker run -ti --rm --privileged ${DOCKER_ARGS[*]} bash
else
    docker run --privileged ${DOCKER_ARGS[*]} sh -c "/workspace/source/tools/ci/build.sh ${BUILD_ARGS[*]}"
fi

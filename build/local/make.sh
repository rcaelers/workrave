#!/bin/bash

IMAGE=
CONFIG=
BUILD_ARGS=()
DOCKER_ARGS=()
BUILT_TYPE=RelWithDebInfo
WORKING_DIR=
OUTPUT_DIR=
CONF_COMPILER=gcc

ROOT=`git rev-parse --show-toplevel`
if [ $? -ne 0 ]; then
  echo "Not inside git working directory"
  exit 1
fi

usage()
{
  echo "Usage: $0 " 1>&2;
  exit 1;
}

parse_arguments()
{
    while getopts "c:C:dD:O:vW:" o; do
        case "${o}" in
            c)
                CONFIG="${OPTARG}"
                BUILD_ARGS+=("-d${CONFIG}")
                ;;
            C)
                CONF_COMPILER="${OPTARG}"
                ;;
            d)
                BUILT_TYPE=Debug
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
            v)
                BUILD_ARGS+=("-M\"VERBOSE=1\"")
                ;;
            *)
                usage
                ;;
        esac
    done
    shift $((OPTIND-1))
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

BUILD_ARGS+=("-DCMAKE_BUILD_TYPE=${BUILT_TYPE}")
BUILD_ARGS+=("-M\"-j4\"")
DOCKER_ARGS+=("-v ${ROOT}/:/workspace/source")

if [ -n "${WORKING_DIR}" ]; then
    if [ ! -d "${WORKING_DIR}" ]; then
        mkdir -p "${WORKING_DIR}"
    fi

    DOCKER_ARGS+=("-v ${WORKING_DIR}:/workspace/build")

    REL_DIR=`git rev-parse --show-prefix`
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

DOCKER_ARGS+=("-e WORKRAVE_ENV=github-docker")
DOCKER_ARGS+=("-e CONF_COMPILER=${CONF_COMPILER}")
DOCKER_ARGS+=("--rm rcaelers/workrave-build:${IMAGE}")

docker run --privileged ${DOCKER_ARGS[*]} sh -c "/workspace/source/build/ci/build.sh ${BUILD_ARGS[*]}"

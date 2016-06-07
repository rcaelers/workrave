#!/bin/bash

IMAGE=
CONFIG=
BUILD_ARGS=()
DOCKER_ARGS=()
BUILT_TYPE=RelWithDebInfo
INCREMENTAL_PATH=

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
  while getopts "i:c:d:D:" o; do
    case "${o}" in
      i)
        INCREMENTAL_PATH="${OPTARG}"
        ;;
      c)
        CONFIG="${OPTARG}"
        BUILD_ARGS+=("-c${CONFIG}")
        ;;
      d)
        BUILT_TYPE=Debug
        BUILD_ARGS+=("-DWITH_TRACING=ON")
        ;;
      D)
        BUILD_ARGS+=("-D${OPTARG}")
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
    mingw32*)
        IMAGE=mingw-gtk2
        ;;
    mingw64*)
        IMAGE=mingw-qt5
        ;;
    *)
        IMAGE=$CONFIG
      ;;
esac

BUILD_ARGS+=("-DCMAKE_BUILD_TYPE=${BUILT_TYPE}")
DOCKER_ARGS+=("-v ${ROOT}/:/workspace/source")

if [ -n "${INCREMENTAL_PATH}" ]; then
  if [ ! -d "${INCREMENTAL_PATH}" ]; then
    mkdir -p "${INCREMENTAL_PATH}"
  fi
  DOCKER_ARGS+=("-v ${INCREMENTAL_PATH}:/workspace/output")

  REL_PATH=`git rev-parse --show-prefix`
  if [ $? -eq 0 ]; then
    BUILD_ARGS+=("-C${REL_PATH}")
  fi
fi

DOCKER_ARGS+=("--rm rcaelers/workrave-build-${IMAGE}")

docker run ${DOCKER_ARGS[*]} sh -c "/workspace/source/build/docker/build.sh ${BUILD_ARGS[*]}"

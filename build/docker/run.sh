#!/bin/bash -x

IMAGE=
CONFIG=
CMAKE_FLAGS=()
BUILT_TYPE=RelWithDebInfo

usage()
{
  echo "Usage: $0 " 1>&2;
  exit 1;
}

parse_arguments()
{
  while getopts "i:d:C:D:" o; do
    case "${o}" in
      d)
        BUILT_TYPE=Debug
        CMAKE_FLAGS+=("-DWITH_TRACING=ON")
        ;;
      C)
        CONFIG="${OPTARG}"
        ;;
      D)
        CMAKE_FLAGS+=("-D${OPTARG}")
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
    mingw-*)
        IMAGE=mingw-gtk2
        ;;
    *)
        IMAGE=$CONFIG
      ;;
esac

CMAKE_FLAGS+=("-DCMAKE_BUILD_TYPE=${BUILT_TYPE}")

docker run \
   -v $PWD/:/workspace/source \
   --rm rcaelers/workrave-build-${IMAGE} \
   sh -c "/workspace/source/build/docker/build.sh -C${CONFIG} ${CMAKE_FLAGS[*]}"

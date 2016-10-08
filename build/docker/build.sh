#!/bin/bash
WORKSPACE=/workspace
SOURCE_DIR=${WORKSPACE}/source
OUTPUT_DIR=${WORKSPACE}/output
BUILD_DIR=${WORKSPACE}/build

CMAKE_FLAGS=()
MAKE_FLAGS=()
REL_DIR=

build()
{
  config=$1
  rel_dir=$2
  cmake_args=("${!3}")

  

  if [ ! -d ${BUILD_DIR}/${config} ]; then
      mkdir -p ${BUILD_DIR}/${config}
  fi
  if [ ! -d ${OUTPUT_DIR}/${config} ]; then
      mkdir -p ${OUTPUT_DIR}/${config}
  fi

  if [ -n "${rel_dir}" -a ! -d "${BUILD_DIR}/${rel_dir}" ]; then
      echo Performing build at toplevel first
      rel_dir=
  fi

  cd ${BUILD_DIR}/${config}/${rel_dir}

  if [ -z "${rel_dir}" ]; then
    cmake ${SOURCE_DIR} -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${config} ${cmake_args[@]}
  fi

  make ${MAKE_FLAGS[@]}
  make ${MAKE_FLAGS[@]} install
}

parse_arguments()
{
  while getopts "c:o:C:D:M:" o; do
      case "${o}" in
          c)
            CONFIG=${OPTARG}
            ;;
          C)
            REL_DIR=${OPTARG}
            ;;
          D)
            CMAKE_FLAGS+=("-D${OPTARG}")
            ;;
          M)
            MAKE_FLAGS+=("${OPTARG}")
            ;;
      esac
  done
  shift $((OPTIND-1))
}

parse_arguments $*


case "$CONFIG" in
    mingw64*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCE_DIR}/build/cmake/mingw64.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    mingw32-vs*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCE_DIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    mingw32*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCE_DIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${OUTPUT_DIR}/.64")

        CMAKE_FLAGS64=()
        CMAKE_FLAGS64+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCE_DIR}/build/cmake/mingw64.cmake")
        CMAKE_FLAGS64+=("-DWITH_UI=None")
        CMAKE_FLAGS64+=("-DCMAKE_BUILD_TYPE=Release")

        build ".64" "" CMAKE_FLAGS64[@]
        ;;
    "*")
      ;;
esac

build "" "${REL_DIR}" CMAKE_FLAGS[@]

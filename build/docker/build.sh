#!/bin/bash
WORKSPACE=/workspace
SOURCEDIR=${WORKSPACE}/source
OUTPUTDIR=${WORKSPACE}/output

CMAKE_FLAGS=()
MAKE_FLAGS=()
REL_PATH=

build()
{
  config=$1
  rel_path=$2
  cmake_args=("${!3}")

  BUILDDIR=${OUTPUTDIR}/${config}/build
  INSTALLDIR=${OUTPUTDIR}/${config}/install

  if [ ! -d ${BUILDDIR}/${rel_path} ]; then
    echo Performing build at toplevel first
    rel_path=
  fi

  mkdir -p ${BUILDDIR} ${INSTALLDIR}
  cd ${BUILDDIR}/${rel_path}

  if [ -z "${rel_path}" ]; then
    cmake ${SOURCEDIR} -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${INSTALLDIR} ${cmake_args[@]}
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
            REL_PATH=${OPTARG}
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
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw64.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    mingw32-vs*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    mingw32*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/noui64/install")

        CMAKE_FLAGS64=()
        CMAKE_FLAGS64+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw64.cmake")
        CMAKE_FLAGS64+=("-DWITH_UI=None")
        CMAKE_FLAGS64+=("-DCMAKE_BUILD_TYPE=Release")

        build noui64 "" CMAKE_FLAGS64[@]
        ;;
    "*")
      ;;
esac

build "${CONFIG}" "${REL_PATH}" CMAKE_FLAGS[@]

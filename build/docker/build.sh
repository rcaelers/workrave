#!/bin/bash -x
echo Called: $*

WORKSPACE=/workspace
SOURCEDIR=${WORKSPACE}/source

CMAKE_FLAGS=()
MAKE_FLAGS="-j4 VERBOSE=1"

build()
{
  config=$1
  cmake_args=("${!2}")

  BUILDDIR=${WORKSPACE}/${config}/build
  INSTALLDIR=${WORKSPACE}/${config}/install

  mkdir -p ${BUILDDIR} ${INSTALLDIR}
  cd ${BUILDDIR}

  cmake ${SOURCEDIR} -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=${INSTALLDIR} ${cmake_args[@]}
  make ${MAKE_FLAGS}
  make ${MAKE_FLAGS} install
}

parse_arguments()
{
  while getopts "6C:D:" o; do
      case "${o}" in
          C)
              CONFIG=${OPTARG}
                  ;;
          D)
              CMAKE_FLAGS+=("-D${OPTARG}")
              echo "Adding CMake options ${OPTARG}"
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
    mingw-vs-*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    mingw-*)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw32.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/noui64/install")

        CMAKE_FLAGS64=()
        CMAKE_FLAGS64+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw64.cmake")
        CMAKE_FLAGS64+=("-DWITH_UI=None")
        CMAKE_FLAGS64+=("-DCMAKE_BUILD_TYPE=Release")

        build noui64 CMAKE_FLAGS64[@]
        ;;
    "*")
      ;;
esac

build ${CONFIG} CMAKE_FLAGS[@]

#!/bin/bash -x

WORKSPACE=/build
PLATFORM=$1
UI=$2
BUILD=$3

CONFIG=${PLATFORM}-${UI}-${BUILD}

SOURCEDIR=${WORKSPACE}/source
BUILDDIR=${WORKSPACE}/build/${CONFIG}
INSTALLDIR=${WORKSPACE}/install/${CONFIG}

CMAKE_FLAGS=()
MAKE_FLAGS=-j4

rm -rf $BUILDDIR $INSTALLDIR
mkdir -p $BUILDDIR $INSTALLDIR

case "$PLATFORM" in
   "linux-gcc") 
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++" "-DCMAKE_C_COMPILER=gcc")
        ;;
    "linux-clang")
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=clang++" "-DCMAKE_C_COMPILER=clang")
        ;;
    "windows")
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw32.cmake" "-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        ;;
    "windows64")
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw64.cmake" "-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
esac

case "$UI" in
   "Gtk+2") 
        CMAKE_FLAGS+=("-DWITH_INDICATOR=OFF")
        ;;
esac

case "$BUILD" in
   "Debug") 
        CMAKE_FLAGS+=("-DWITH_TRACING=ON")
        ;;
esac

rm -rf ${BUILDDIR} ${INSTALLDIR}
mkdir ${BUILDDIR} ${INSTALLDIR}

cd ${BUILDDIR}
cmake ${SOURCEDIR} -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="${INSTALLDIR}" -DCMAKE_BUILD_TYPE=$BUILD -DWITH_UI=$UI -DLOCALINSTALL=ON "${CMAKE_FLAGS[@]}"
make $MAKE_FLAGS -k VERBOSE=1
make install

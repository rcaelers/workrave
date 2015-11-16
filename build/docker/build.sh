#!/bin/bash -x

WORKSPACE=/build
PLATFORM=$1
UI=$2
BUILD=$3

CONFIG=${PLATFORM}-${UI}-${BUILD}

SOURCEDIR=${WORKSPACE}/source
BUILDDIR=${WORKSPACE}/build-${CONFIG}
INSTALLDIR=${WORKSPACE}/install-${CONFIG}

CMAKE_FLAGS=()
MAKE_FLAGS=

case "$PLATFORM" in
   "linux-gcc") 
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++" "-DCMAKE_C_COMPILER=gcc")
        MAKE_FLAGS=-j2
	;;
    "linux-clang")
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=clang++" "-DCMAKE_C_COMPILER=clang")
        MAKE_FLAGS=-j2
	;;
    "windows")
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCEDIR}/build/cmake/mingw.cmake" "-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        MAKE_FLAGS=-j2
	;;
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
make -j4 $MAKE_FLAGS -k VERBOSE=1
make install

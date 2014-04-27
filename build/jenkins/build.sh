#!/bin/bash -x

CMAKE_FLAGS=()

case "$PLATFORM" in
   "linux-gcc") 
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++" "-DCMAKE_C_COMPILER=gcc")
        MAKE_FLAGS=-j8
	;;
    "linux-clang")
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=clang++" "-DCMAKE_C_COMPILER=clang")
        MAKE_FLAGS=-j8
	;;
    "windows")
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=$WORKSPACE/build/cmake/mingw.cmake" "-DPREBUILT_PATH=/home/jenkins/prebuilt")
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

mkdir "$WORKSPACE/Build"
cd "$WORKSPACE/Build"

cmake "$WORKSPACE" -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$WORKSPACE/Install" -DCMAKE_BUILD_TYPE=$BUILD -DWITH_UI=$UI -DLOCALINSTALL=ON "${CMAKE_FLAGS[@]}"

make $MAKE_FLAGS -k
make install

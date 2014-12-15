export CC=i686-w64-mingw32-gcc
export CXX=i686-w64-mingw32-g++

export PKG_CONFIG_DIR=
export PKG_CONFIG_LIBDIR=$SYSROOT/lib/pkgconfig
export PKG_CONFIG_PATH=$SYSROOT/share/pkgconfig
#export PKG_CONFIG_SYSROOT_DIR=${SYSROOT}
export PKG_CONFIG="pkg-config --define-variable=prefix=$SYSROOT"

export CFLAGS="-I$SYSROOT/include"
export CXXFLAGS="-I$SYSROOT/include"
export CPPFLAGS="-I$SYSROOT/include"
export LDFLAGS="-L$SYSROOT/lib"

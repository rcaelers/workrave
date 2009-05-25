#!/bin/bash

# This is my script for building a complete cross-compiler toolchain.
# It is based partly on Ray Kelm's script, which in turn was built on
# Mo Dejong's script for doing the same, but with some added fixes.
# The intent with this script is to build a cross-compiled version
# of the current MinGW environment.
#
# Updated by Sam Lantinga <slouken@libsdl.org>
#
# Updated by Rob Caelers for workrave cross compilation.
#
# what flavor are we building?

TARGET=i386-mingw32msvc

# where does it go?

PREFIX=/usr/local/cross-tools-gcc421

# you probably don't need to change anything from here down

TOPDIR=`pwd`
SRCDIR="$TOPDIR/source"

MINGW_URL=http://surfnet.dl.sourceforge.net/sourceforge/mingw
GCC=gcc-4.2.1-2-src
GCC_ARCHIVE=gcc-4.2.1-2-src.tar.gz  

BINUTILS=binutils-2.19
BINUTILS_ARCHIVE=$BINUTILS-src.tar.gz

MINGW=mingw-runtime-3.15.2
MINGW_ARCHIVE=mingwrt-3.15.2-mingw32-dev.tar.gz

W32API=w32api-3.13
W32API_ARCHIVE=$W32API-mingw32-dev.tar.gz

UTILS=mingw-utils-0.3
UTILS_PATCH=mingw-utils-0.3.diff
UTILS_ARCHIVE=mingw-utils-0.3-src.tar.gz

# need install directory first on the path so gcc can find binutils

PATH="$PREFIX/bin:$PATH"

#
# download a file from a given url, only if it is not present
#

download_file()
{
	cd "$SRCDIR"
	if test ! -f $1 ; then
		echo "Downloading $1"
		wget "$2/$1"
		if test ! -f $1 ; then
			echo "Could not download $1"
			exit 1
		fi
	else
		echo "Found $1 in the srcdir $SRCDIR"
	fi
  	cd "$TOPDIR"
}

download_files()
{
	mkdir -p "$SRCDIR"
	
	# Make sure wget is installed
	if test "x`which wget`" = "x" ; then
		echo "You need to install wget."
		exit 1
	fi
	download_file "$GCC_ARCHIVE" "$MINGW_URL"
	#download_file "$GXX_ARCHIVE" "$MINGW_URL"
	download_file "$BINUTILS_ARCHIVE" "$MINGW_URL"
	download_file "$MINGW_ARCHIVE" "$MINGW_URL"
	download_file "$W32API_ARCHIVE" "$MINGW_URL"
	download_file "$UTILS_ARCHIVE" "$MINGW_URL"
	download_file "$HEADERS_ARCHIVE" "$MINGW_URL"
	download_file "$CRT_ARCHIVE" "$MINGW_URL"
}

install_libs()
{
	echo "Installing cross libs and includes"
	mkdir -p "$PREFIX/$TARGET"
	cd "$PREFIX/$TARGET"
	gzip -dc "$SRCDIR/$MINGW_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$W32API_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

extract_binutils()
{
	cd "$SRCDIR"
	rm -rf "$BINUTILS"
	echo "Extracting binutils"
	gzip -dc "$SRCDIR/$BINUTILS_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

configure_binutils()
{
	cd "$TOPDIR"
	rm -rf "binutils-$TARGET"
	mkdir "binutils-$TARGET"
	cd "binutils-$TARGET"
	echo "Configuring binutils"
	"$SRCDIR/$BINUTILS/configure" --prefix="$PREFIX" --target=$TARGET --disable-werror &> configure.log
	cd "$TOPDIR"
}

build_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Building binutils"
	make &> make.log
	if test $? -ne 0; then
		echo "make failed - log available: binutils-$TARGET/make.log"
		exit 1
	fi
	cd "$TOPDIR"
}

install_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Installing binutils"
	make install &> make-install.log
	if test $? -ne 0; then
		echo "install failed - log available: binutils-$TARGET/make-install.log"
		exit 1
	fi
	cd "$TOPDIR"
}

extract_gcc()
{
	cd "$SRCDIR"
	rm -rf "$GCC"
	echo "Extracting gcc"
	gzip -dc "$SRCDIR/$GCC_ARCHIVE" | tar xf -
	##gzip -dc "$SRCDIR/$GXX_ARCHIVE" | tar xf -

	#cd "$SRCDIR/$GCC/libstdc++-v3/config"
        #ln -s . config
	cd "$TOPDIR"
}

patch_gcc()
{
	if [ "$GCC_PATCH" != "" ]; then
		echo "Patching gcc"
		cd "$SRCDIR/$GCC"
		patch -p1 < "$TOPDIR/$GCC_PATCH"
		cd "$TOPDIR"
	fi
}

configure_gcc()
{
	cd "$TOPDIR"
	rm -rf "gcc-$TARGET"
	mkdir "gcc-$TARGET"
	cd "gcc-$TARGET"
	echo "Configuring gcc"

	"$SRCDIR/$GCC/configure" -v \
	        --with-gcc --with-gnu-as --with-gnu-ld --disable-libgomp \
		--with-arch=i486 --with-tune=generic \
		--disable-werror \
		--enable-threads=win32 \
		--disable-nls \
		--enable-languages=c,c++ \
                --disable-win32-registry  \
		--enable-sjlj-exceptions \
	        --enable-libstdcxx-debug \
		--enable-cxx-flags='-fno-function-sections -fno-data-sections' \
		--enable-version-specific-runtime-libs \
	        --disable-bootstrap \
		--prefix="$PREFIX" --target=$TARGET \
		--with-headers="$PREFIX/$TARGET/include" \
		--disable-multilib \
		  &> configure.log
	cd "$TOPDIR"
}

#		--prefix="$PREFIX" --target=$TARGET \
#		--with-headers="$PREFIX/$TARGET/include" \
#		--with-gcc --with-gnu-as --with-gnu-ld \
#	        --enable-threads=win32 --disable-win32-registry --enable-languages=c,c++ \
#		--enable-cxx-flags='-fno-function-sections -fno-data-sections' \
#		--disable-sjlj-exceptions --enable-libstdcxx-debug \
#		--enable-version-specific-runtime-libs --disable-bootstrap \
#		--disable-libgomp \
#		--with-tune=generic --disable-werror \
#		--disable-nls  \

#		--program-suffix=-dw2 --with-arch=i486 --with-tune=generic --disable-werror \
#		--disable-nls  \

build_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Building gcc"
	make BOOTCFLAGS="-O2 -D__USE_MINGW_ACCESS" CFLAGS="-O2 -D__USE_MINGW_ACCESS" CXXFLAGS="-mthreads -O2" LDFLAGS="-s" &> make.log

	if test $? -ne 0; then
		echo "make failed - log available: gcc-$TARGET/make.log"
		exit 1
	fi
	cd "$TOPDIR"
}

install_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Installing gcc"
	make install &> make-install.log
#	make -C gcc install-shared-libgcc &> make-install-gcc.log
#	make -C mingw32/libstdc++-v3/  install-shared-libstdc++ &> make-install-stdc.log

	if test $? -ne 0; then
		echo "install failed - log available: gcc-$TARGET/make-install.log"
		exit 1
	fi
	cd "$TOPDIR"
}

extract_utils()
{
	cd "$SRCDIR"
	rm -rf "$UTILS"
	echo "Extracting utils"
	gzip -dc "$SRCDIR/$UTILS_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

patch_utils()
{
	if [ "$UTILS_PATCH" != "" ]; then
		echo "Patching mingw-utils"
		cd "$SRCDIR/$UTILS"
		patch -p1 < "$TOPDIR/$UTILS_PATCH"
		cd "$TOPDIR"
	fi
}

configure_utils()
{
	cd "$TOPDIR"
	rm -rf "utils-$TARGET"
	mkdir "utils-$TARGET"
	cd "utils-$TARGET"
	echo "Configuring utils"
	"$SRCDIR/$UTILS/configure" --prefix="$PREFIX" --target=$TARGET --enable-install-libiberty &> configure.log
	cd "$TOPDIR"
}

build_utils()
{
	cd "$TOPDIR/utils-$TARGET/pexports"
	echo "Building utils"
	make &> make.log
	if test $? -ne 0; then
		echo "make failed - log available: utils-$TARGET/make.log"
		exit 1
	fi
	cd "$TOPDIR"
}

install_utils()
{
	cd "$TOPDIR/utils-$TARGET/pexports"
	echo "Installing utils"
	make install &> make-install.log
	if test $? -ne 0; then
		echo "install failed - log available: utils-$TARGET/make-install.log"
		exit 1
	fi
	cd "$TOPDIR"
}


final_tweaks()
{
	echo "Finalizing installation"

	# remove gcc build headers
	rm -rf "$PREFIX/$TARGET/sys-include"

        # Add extra binary links
	if [ ! -f "$PREFIX/$TARGET/bin/objdump" ]; then
		ln "$PREFIX/bin/$TARGET-objdump" "$PREFIX/$TARGET/bin/objdump"
	fi

	# make cc and c++ symlinks to gcc and g++
	if [ ! -f "$PREFIX/$TARGET/bin/g++" ]; then
		ln "$PREFIX/bin/$TARGET-g++" "$PREFIX/$TARGET/bin/g++"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/cc" ]; then
		ln -s "gcc" "$PREFIX/$TARGET/bin/cc"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/c++" ]; then
		ln -s "g++" "$PREFIX/$TARGET/bin/c++"
	fi

	# strip all the binaries
	ls "$PREFIX"/bin/* "$PREFIX/$TARGET"/bin/* | egrep -v '.dll$' |
	while read file; do
		strip "$file"
	done

        cp -a mingw32 $PREFIX/bin
        
        # i386-mingw32msvc-dlltool --as=i386-mingw32msvc-as -k --dllname crtdll.dll --output-lib libcrtdll.a --def crtdll.def        
	echo "Installation complete!"
}

download_files

install_libs

extract_binutils
configure_binutils
build_binutils
install_binutils

extract_gcc
patch_gcc
configure_gcc
build_gcc
install_gcc

extract_utils
patch_utils
configure_utils
build_utils
install_utils

final_tweaks

#!/bin/bash

# Compilation script for workrave runtime.
# Based on work by Ray Kelm's script, Mo Dejong's and Sam Lantinga.
#
# Changed by Rob Caelers for workrave cross compilation.
#

TOOLS=/usr/local/cross-tools-gcc450
POSTFIX=-gtk2.14
PREFIX=/usr/local/cross-packages${POSTFIX}
TARGET=i686-w64-mingw32

export PATH="$TOOLS/bin:$PATH"

TOPDIR=`pwd`
SRCDIR="$TOPDIR/source"
BUILDDIR="$TOPDIR/build"

SF_URL="http://surfnet.dl.sourceforge.net/sourceforge"
GNU_URL="ftp://ftp.gnu.org/gnu"

GLIB_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.28/"
GLIB_FILES="glib_2.28.1-1_win32.zip glib-dev_2.28.1-1_win32.zip"

PIXBUF_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/gdk-pixbuf/2.22"
PIXBUF_FILES="gdk-pixbuf_2.22.1-1_win32.zip gdk-pixbuf-dev_2.22.1-1_win32.zip"

#GTK_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.16/"
#GTK_FILES="gtk+_2.16.6-2_win32.zip gtk+-dev_2.16.6-2_win32.zip"

GTK_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.24/"
GTK_FILES="gtk+_2.24.0-1_win32.zip gtk+-dev_2.24.0-1_win32.zip"

PANGO_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/pango/1.28"
PANGO_FILES="pango_1.28.3-1_win32.zip pango-dev_1.28.3-1_win32.zip"

ATK_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/atk/1.32/"
ATK_FILES="atk_1.32.0-1_win32.zip atk-dev_1.32.0-1_win32.zip"

DBUS_URL="http://dbus.freedesktop.org/releases/dbus/"
DBUS_FILES="dbus-1.5.6.tar.gz"

# http://www.dgrigoriadis.net/post/2004/06/26/DirectXDevPak-for-Dev-Cpp.aspx
# http://www.dgrigoriadis.net/file.axd?file=2009%2f2%2fDirectX90c.DevPak

DIRECTX_URL="http://www.g-productions.net/files/devpak/"
DIRECTX_FILE="DirectX90c.DevPak"

DEP_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/"
DEP_FILES="cairo_1.10.2-1_win32.zip cairo-dev_1.10.2-1_win32.zip jpeg_7-1_win32.zip jpeg-dev_7-1_win32.zip libpng_1.4.3-1_win32.zip libpng-dev_1.4.3-1_win32.zip zlib_1.2.5-2_win32.zip zlib-dev_1.2.5-2_win32.zip libtiff-dev_3.9.1-1_win32.zip libtiff_3.9.1-1_win32.zip libiconv-1.9.1.bin.woe32.zip gettext-runtime-dev_0.18.1.1-2_win32.zip gettext-runtime_0.18.1.1-2_win32.zip freetype-dev_2.4.4-1_win32.zip freetype_2.4.4-1_win32.zip fontconfig_2.8.0-1_win32.zip fontconfig-dev_2.8.0-1_win32.zip expat-dev_2.0.1-1_win32.zip expat_2.0.1-1_win32.zip"

PKGCONFIG_URL="http://pkgconfig.freedesktop.org/releases/"
PKGCONFIG_FILES="pkg-config-0.26.tar.gz"

GNETSRC_URL="http://ftp.gnome.org/pub/GNOME/sources/gnet/2.0/"
GNOME_URL="http://ftp.gnome.org/pub/GNOME/sources/"

# UUID_URL=$SF_URL/e2fsprogs/
SIGCPPSRC_URL=$GNOME_URL/libsigc++/2.2/
GLIBMMSRC_URL=$GNOME_URL/glibmm/2.28/
GTKMMSRC_URL=$GNOME_URL/gtkmm/2.24/
PANGOMMSRC_URL=$GNOME_URL/pangomm/2.28/
ATKMMSRC_URL=$GNOME_URL/atkmm/2.22/
CAIROMMSRC_URL=http://cairographics.org/releases/

SIGCPPSRC_FILES="libsigc++-2.2.10.tar.bz2"
GTKMMSRC_FILES="gtkmm-2.24.2.tar.bz2"
GLIBMMSRC_FILES="glibmm-2.28.2.tar.bz2"
PANGOMMSRC_FILES="pangomm-2.28.2.tar.bz2"
ATKMMSRC_FILES="atkmm-2.22.5.tar.bz2"
CAIROMMSRC_FILES="cairomm-1.10.0.tar.gz"

#GNETSRC_FILES="gnet-2.0.8.tar.gz"
#UUID_FILES="e2fsprogs-libs-1.40.5.tar.gz"

BINUTILS=binutils-2.16.91-20060119-1
BINUTILS_ARCHIVE=$BINUTILS-src.tar.gz

download_files()
{
	cd "$SRCDIR"
        base=$1
        shift

        for a in $*; do
        	if test ! -f $a ; then
        		echo "Downloading $a"
        		wget "$base/$a"
        		if test ! -f $a ; then
        			echo "Could not download $a"
        			exit 1
        		fi
        	else
        		echo "Found $a."
        	fi
        done
  	cd "$TOPDIR"
}

unzip_files()
{
	cd "$SRCDIR"

        for a in $*; do
        	if test -f $a ; then
        		echo "Unpacking $a"
                        cd $PREFIX
                        unzip -qo $SRCDIR/$a
                        cd $SRCDIR
        	else
        		echo "Could not find $a."
        	fi
        done
  	cd "$TOPDIR"
}

download()
{
	mkdir -p "$SRCDIR"

	# Make sure wget is installed
	if test "x`which wget`" = "x" ; then
		echo "You need to install wget."
		exit 1
	fi

        download_files $GLIB_URL $GLIB_FILES
	download_files $PIXBUF_URL $PIXBUF_FILES
        download_files $GTK_URL $GTK_FILES
        download_files $PANGO_URL $PANGO_FILES
        download_files $ATK_URL $ATK_FILES
        download_files $DEP_URL $DEP_FILES
        
        download_files $GNETSRC_URL $GNETSRC_FILES
        download_files $GLIBMMSRC_URL $GLIBMMSRC_FILES
        download_files $GTKMMSRC_URL $GTKMMSRC_FILES
        download_files $PANGOMMSRC_URL $PANGOMMSRC_FILES
        download_files $SIGCPPSRC_URL $SIGCPPSRC_FILES
        download_files $ATKMMSRC_URL $ATKMMSRC_FILES
        download_files $CAIROMMSRC_URL $CAIROMMSRC_FILES
        download_files $UUID_URL $UUID_FILES
        download_files $DBUS_URL $DBUS_FILES
        download_files $PKGCONFIG_URL $PKGCONFIG_FILES

        download_files $DIRECTX_URL $DIRECTX_FILE
}

unpack()
{
        rm -rf $PREFIX
        mkdir $PREFIX
        	
        unzip_files $GTK_FILES
        unzip_files $GLIB_FILES
	unzip_files $PIXBUF_FILES
        unzip_files $PANGO_FILES
        unzip_files $GTK_DEP_FILES
        unzip_files $DEP_FILES
	unzip_files $ATK_FILES
}

fix_theme()
{
    cat $PREFIX/share/themes/MS-Windows/gtk-2.0/gtkrc | sed -e 's/gtk-button-images = 0/gtk-button-images = 1/' > gtkrc.tmp
    cp -a gtkrc.tmp $PREFIX/share/themes/MS-Windows/gtk-2.0/gtkrc
    rm gtkrc.tmp
}

fix_pkgconfig()
{
    cd $PREFIX/lib/pkgconfig

    for a in $PREFIX/lib/pkgconfig/*.pc; do
        sed -e "s|c:/devel/target/.*$|$PREFIX|g" -e "s|$||g" < $a > $a.new
        mv -f $a.new $a
        sed -e "s|/devel/target/.*$|$PREFIX|g" -e "s|$||g" < $a > $a.new
        mv -f $a.new $a
        sed -e "s|/target/.*$|$PREFIX|g" -e "s|$||g" < $a > $a.new
        mv -f $a.new $a
    done

    sed -e "s|c:/progra~1/.*$|$PREFIX|g" -e "s|$||g" < libpng.pc > libpng.pc.new
    mv -f libpng.pc.new libpng.pc
    
    sed -e "s|c:/progra~1/.*$|$PREFIX|g" -e "s|$||g" < libpng12.pc > libpng12.pc.new
    mv -f libpng12.pc.new libpng12.pc
    
    sed -e "s|c:/progra~1/.*$|$PREFIX|g" -e "s|$||g" < libpng13.pc > libpng13.pc.new
    mv -f libpng13.pc.new libpng13.pc

    cd "$TOPDIR"
}

fix_lib()
{
    cd $PREFIX/bin
    cp zlib1.dll zlib.dll

    cd $PREFIX/lib
    rm -f intl.lib libintl.dll.a
    rm -f iconv.lib libiconv.dll.a
    #rm -f atk-1.0.lib libatk-1.0.dll.a

    cd "$BUILDDIR"

    # | sed "s/^\([a-z]\)/_\1/"
    pexports $PREFIX/bin/intl.dll  > intl.def
    pexports $PREFIX/bin/iconv.dll  > iconv.def
    #pexports $PREFIX/bin/libatk-1.0-0.dll | sed "s/libatk-1.0-0/libatk/" > atk.def
    
    i386-mingw32msvc-dlltool -d intl.def -l $PREFIX/lib/libintl.dll.a
    i386-mingw32msvc-dlltool -U -d iconv.def -l $PREFIX/lib/libiconv.dll.a
    #i386-mingw32msvc-dlltool -d atk.def -l $PREFIX/lib/libatk-1.0.dll.a
    i386-mingw32msvc-ranlib $PREFIX/lib/libintl.dll.a
    i386-mingw32msvc-ranlib $PREFIX/lib/libiconv.dll.a
    #i386-mingw32msvc-ranlib $PREFIX/lib/libatk-1.0.dll.a
}

extract_directx()
{
    extract_package "DirectX 90c" $DIRECTX_FILE
    cp -a $BUILDDIR/DirectX90c/include/* $PREFIX/include
    cp -a $BUILDDIR/DirectX90c/lib/* $PREFIX/lib
    ln -s $PREFIX/include/dxerr8.h $PREFIX/include/dxerr.h
}

extract_package()
{
	cd "$BUILDDIR"
	rm -rf "$1"
	echo "Extracting $1"
        case $2 in
            *.tar.gz)
                 tar xzf $SRCDIR/$2
                 ;;
            *.tgz)
                 tar xzf $SRCDIR/$2
                 ;;
            *.tar.bz2)
                 tar xjf $SRCDIR/$2
                 ;;
            *.DevPak)
                 tar xjf $SRCDIR/$2
                 ;;
            *.tar)
                 tar xf $SRCDIR/$2
                 ;;
            *.gz)
                 gunzip $SRCDIR/$2
                 ;;
            *.bz2)
                 bunzip2 $SRCDIR/$2
                 ;;
        esac
	cd "$TOPDIR"

	if [ -f "$TOPDIR/$1.diff" ]; then
		echo "Patching $1"
		cd "$BUILDDIR/$1"
		patch -p1 < "$TOPDIR/$1.diff"
		cd "$TOPDIR"
	fi
}

build_pkgconfig()
{
	cd "$BUILDDIR"
	rm -rf "pkgconfig-$TARGET"
	mkdir "pkgconfig-$TARGET"

	cd "$BUILDDIR/pkgconfig-$TARGET"

        echo "Configuring pkgconfig"
        (  . $TOPDIR/mingw32-x -gtk2.14
	    "$BUILDDIR/$1/configure" -v \
		--prefix="$TOOLS" --disable-shared --enable-static \
                --target=$TARGET --host=i586-linux --build=i586-linux \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	
	if test $? -ne 0; then
		echo "configure failed - log available: pkgconfig-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building pkgconfig"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: pkgconfig-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/pkgconfig-$TARGET"
	echo "Installing pkgconfig"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: pkgconfig-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}


build_sigcpp()
{
	cd "$BUILDDIR"
	rm -rf "libsigc++-$TARGET"
	mkdir "libsigc++-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $BUILDDIR/$1
        #for a in sigc++/config/sigcconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

        cd "$BUILDDIR/libsigc++-$TARGET"

        echo "Configuring Libsigc++"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --disable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libsigc++-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libsigc++"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libsigc++-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libsigc++-$TARGET"
	echo "Installing Libsigc++"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make -k install &> make-install.log
        )

        if test $? -ne 0; then
            echo "install failed - log available: libsigc++-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_glibmm()
{
	cd "$BUILDDIR"
	rm -rf "libglibmm-$TARGET"
	mkdir "libglibmm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        cd $BUILDDIR/$1
        for a in glib/glibmmconfig.h.in; do
            echo Patching $a...
            sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
            mv $a.new $a
        done

	cd "$BUILDDIR/libglibmm-$TARGET"

        echo "Configuring Libglibmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libglibmm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libglibmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libglibmm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libglibmm-$TARGET"
	echo "Installing Libglibmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libglibmm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_cairomm()
{
	cd "$BUILDDIR"
	rm -rf "libcairomm-$TARGET"
	mkdir "libcairomm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $BUILDDIR/$1
        #for a in cairo/cairommconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

	cd "$BUILDDIR/libcairomm-$TARGET"

        echo "Configuring Libcairomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
#                --enable-use-deprecations \
#                --disable-api-properties \
#                --disable-api-vfuncs \
#                --disable-api-exceptions \
#                --disable-deprecated-api \
#                --disable-api-default-signal-handlers \
	
	if test $? -ne 0; then
		echo "configure failed - log available: libcairomm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libcairomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libcairomm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libcairomm-$TARGET"
	echo "Installing Libcairomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libcairomm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_pangomm()
{
	cd "$BUILDDIR"
	rm -rf "libpangomm-$TARGET"
	mkdir "libpangomm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $BUILDDIR/$1
        #for a in pango/pangommconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

	cd "$BUILDDIR/libpangomm-$TARGET"

        echo "Configuring Libpangomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
#                --enable-use-deprecations \
#                --disable-api-properties \
#                --disable-api-vfuncs \
#                --disable-api-exceptions \
#                --disable-deprecated-api \
#                --disable-api-default-signal-handlers \
	
	if test $? -ne 0; then
		echo "configure failed - log available: libpangomm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libpangomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libpangomm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libpangomm-$TARGET"
	echo "Installing Libpangomm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libpangomm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_atkmm()
{
	cd "$BUILDDIR"
	rm -rf "libatkmm-$TARGET"
	mkdir "libatkmm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $BUILDDIR/$1
        #for a in atk/atkmmconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

	cd "$BUILDDIR/libatkmm-$TARGET"

        echo "Configuring Libatkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
#                --enable-use-deprecations \
#                --disable-api-properties \
#                --disable-api-vfuncs \
#                --disable-api-exceptions \
#                --disable-deprecated-api \
#                --disable-api-default-signal-handlers \
	
	if test $? -ne 0; then
		echo "configure failed - log available: libatkmm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libatkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libatkmm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libatkmm-$TARGET"
	echo "Installing Libatkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libatkmm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_gtkmm()
{
	cd "$BUILDDIR"
	rm -rf "libgtkmm-$TARGET"
	mkdir "libgtkmm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        cd $BUILDDIR/$1
        for a in gdk/gdkmmconfig.h.in gtk/gtkmmconfig.h.in; do
            echo Patching $a...
            sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
            mv $a.new $a
        done

        # This example does not build..
        #cd examples/book/clipboard
        #sed -e "s|^SUBDIRS.*$|SUBDIRS=|g" < Makefile.in > Makefile.in.new
        #mv -f Makefile.in.new Makefile.in
        
	cd "$BUILDDIR/libgtkmm-$TARGET"

        echo "Configuring Libgtkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            $BUILDDIR/$1/autogen.sh
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
		--with-headers="$PREFIX/$TARGET/include" \
                --enable-deprecated-api \
                --disable-examples \
		--with-gnu-as --with-gnu-ld \
                --disable-demo &> configure.log
        )

##                --disable-use-deprecations \
##                --disable-deprecated-api \
	
	if test $? -ne 0; then
		echo "configure failed - log available: libgtkmm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libgtkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libgtkmm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libgtkmm-$TARGET"
	echo "Installing Libgtkmm"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libgtkmm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_gnet()
{
	cd "$BUILDDIR/$1/"

        echo "Configuring Libgnet"
        (       PKG_CONFIG_PATH=/usr/lib/pkgconfig ./configure -v --prefix=$PREFIX --disable-pthreads \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libgnet-$TARGET/configure.log"
		exit 1
	fi

        cp -a config.h.win32 config.h
        
	echo "Building Libgnet"
        (   . $TOPDIR/mingw32-x -gtk2.14
            export CC='i686-w64-mingw32-gcc -mms-bitfields -mno-cygwin'
            export CXX='i686-w64-mingw32-g++ -mms-bitfields -mno-cygwin'
            cd src
            make -f makefile.mingw &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libgnet-$TARGET/make.log"
		exit 1
	fi

        
	echo "Installing Libgnet"
        mkdir -p $PREFIX/include/gnet-2.0
        cp -a src/*.h $PREFIX/include/gnet-2.0
        cp -a src/libgnet-2.0.a $PREFIX/lib
        cp -a src/gnet-2.0.dll $PREFIX/bin
        cp -a gnet-2.0.m4 $PREFIX/share/aclocal
        cp -a gnetconfig.h $PREFIX/include/gnet-2.0

	cat gnet-2.0.pc | sed -e 's/-pthread//' -e 's/-lrt//' > $PREFIX/lib/pkgconfig/gnet-2.0.pc
	cd "$TOPDIR"
}


extract_bfd()
{
	cd "$BUILDDIR"
	rm -rf "$BINUTILS"
	echo "Extracting bfd"
	gzip -dc "$SRCDIR/$BINUTILS_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

configure_bfd()
{
	cd "$BUILDDIR"
	rm -rf "binutils-$TARGET"-bfd
	mkdir "binutils-$TARGET"-bfd
	cd "binutils-$TARGET"-bfd
	echo "Configuring bfd"
        (   . $TOPDIR/mingw32-x -gtk2.14
	    "$BUILDDIR/$BINUTILS/bfd/configure" --prefix="$PREFIX" --host=$TARGET --target=$TARGET --enable-install-libbfd --enable-install-libiberty=yes CFLAGS=-g &> configure.log
        )
	cd "$TOPDIR"
}

build_bfd()
{
	cd "$BUILDDIR/binutils-$TARGET"-bfd
	echo "Building bfd"
        (   . $TOPDIR/mingw32-x -gtk2.14
	    make &> make.log
        )
        if test $? -ne 0; then
	    echo "make failed - log available: binutils-$TARGET/make.log"
	    exit 1
	fi
	cd "$TOPDIR"
}

install_bfd()
{
	cd "$BUILDDIR/binutils-$TARGET"-bfd
	echo "Installing bfd"
        (   . $TOPDIR/mingw32-x -gtk2.14
	    make install &> make-install.log
        )
	if test $? -ne 0; then
	    echo "install failed - log available: binutils-$TARGET/make-install.log"
	    exit 1
	fi
            
	cd "$TOPDIR"
}




build_uuid()
{
	cd "$BUILDDIR"
	rm -rf "libuuid-$TARGET"
	mkdir "libuuid-$TARGET"

	cd "$BUILDDIR/libuuid-$TARGET"

        echo "Configuring e2fsprogs-libs"
        (   . $TOPDIR/mingw32-x -gtk2.14
            "$BUILDDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i586-linux \
                --with-cc=i686-w64-mingw32-gcc \
                --with-linker=i686-w64-mingw32-ld \
		--with-headers="$PREFIX/$TARGET/include" \
                &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libuuid-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libuuid"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make -C lib/uuid &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libuuid-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/libuuid-$TARGET"
	echo "Installing Libuuid"
        (   . $TOPDIR/mingw32-x -gtk2.14
            #make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libuuid-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}


build_dbus()
{
	cd "$BUILDDIR"

        if [ ! -e dbus-git ]; then
	    git clone git://anongit.freedesktop.org/dbus/dbus dbus-git
	fi
	
	cd "$BUILDDIR"
	rm -rf "dbus-$TARGET"
	mkdir "dbus-$TARGET"

	cd "$BUILDDIR/dbus-$TARGET"

        echo "Configuring Dbus"
        (   . $TOPDIR/mingw32-x -gtk2.14
            cmake $BUILDDIR/dbus-git/cmake \
                -DCMAKE_SYSTEM_NAME="Windows" \
                -DCMAKE_VERBOSE_MAKEFILE=ON \
                -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX \
                -DCMAKE_INSTALL_LIBDIR:PATH=$PREFIX/bin \
                -DINCLUDE_INSTALL_DIR:PATH=$PREFIX/include \
                -DLIB_INSTALL_DIR:PATH=$PREFIX/bin \
                -DSYSCONF_INSTALL_DIR:PATH=$PREFIX/etc \
                -DSHARE_INSTALL_PREFIX:PATH=$PREFIX/share \
                -DBUILD_SHARED_LIBS:BOOL=ON \
                -DCMAKE_C_COMPILER="i686-w64-mingw32-gcc" \
                -DCMAKE_CXX_COMPILER="i686-w64-mingw32-g++" \
                -DCMAKE_RC_COMPILER_INIT="i686-w64-mingw32-windres" \
                -DCMAKE_FIND_ROOT_PATH="$PREFIX" \
                -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
                -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
                -DCMAKE_FIND_ROOT_PATH="$PREFIX" \
                -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
                -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
                -DDBUS_USE_EXPAT=ON \
		-DLIBEXPAT_INCLUDE_DIR:PATH=$PREFIX/include -DLIBEXPAT_LIBRARIES:PATH=$PREFIX/lib/libexpat.dll.a  \
                -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
		-DDBUS_BUILD_TESTS=OFF \
                -DDBUS_ENABLE_XML_DOCS=OFF \
                -DDBUS_ENABLE_DOXYGEN_DOCS=OFF \
                -DDBUS_REPLACE_LOCAL_DIR=ON \
                -DDBUS_ENABLE_VERBOSE_MODE=OFF \
                -DDBUS_DISABLE_ASSERTS=ON \
                -DDBUS_SESSION_BUS_DEFAULT_ADDRESS:STRING=autolaunch: \
                -DDBUS_USE_OUTPUT_DEBUG_STRING=OFF
	)
	if test $? -ne 0; then
		echo "configure failed - log available: dbus-$TARGET/configure.log"
		exit 1
	fi

	echo "Building Dbus"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: dbus-$TARGET/make.log"
		exit 1
	fi

        
	cd "$BUILDDIR/dbus-$TARGET"
	echo "Installing Dbus"
        (   . $TOPDIR/mingw32-x -gtk2.14
            make install &> make-install.log
        )
	cp -a bin/libdbus-1.dll.a $PREFIX/lib

        sed -e "s|@prefix@|$PREFIX|g" \
            -e "s|@exec_prefix@|$PREFIX/bin|g" \
            -e "s|@libdir@|$PREFIX/lib|g" \
            -e "s|@includedir@|$PREFIX/include|g" \
            -e "s|@VERSION@|1.5.4|g" \
	< $BUILDDIR/dbus-git/dbus-1.pc.in > $PREFIX/lib/pkgconfig/dbus-1.pc

        if test $? -ne 0; then
            echo "install failed - log available: dbus-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

download
unpack
extract_directx

fix_theme
fix_pkgconfig

extract_package "libsigc++-2.2.10" "libsigc++-2.2.10.tar.bz2"
build_sigcpp "libsigc++-2.2.10"

extract_package "glibmm-2.28.2" "glibmm-2.28.2.tar.bz2"
build_glibmm "glibmm-2.28.2"

extract_package "cairomm-1.10.0" "cairomm-1.10.0.tar.gz"
build_cairomm "cairomm-1.10.0"

extract_package "pangomm-2.28.2" "pangomm-2.28.2.tar.bz2"
build_pangomm "pangomm-2.28.2"

extract_package "atkmm-2.22.5" "atkmm-2.22.5.tar.bz2"
build_atkmm "atkmm-2.22.5"

extract_package "gtkmm-2.24.2" "gtkmm-2.24.2.tar.bz2"
build_gtkmm "gtkmm-2.24.2"

#extract_package "gnet-2.0.8" "gnet-2.0.8.tar.gz"
#build_gnet "gnet-2.0.8"

build_dbus

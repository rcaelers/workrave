#!/bin/bash

# Compilation script for workrave runtime.
# Based on work by Ray Kelm's script, Mo Dejong's and Sam Lantinga.
#
# Changed by Rob Caelers for workrave cross compilation.
#

TOOLS=/usr/local/cross-tools
POSTFIX=-gtk2.8
PREFIX=/usr/local/cross-packages${POSTFIX}
TARGET=i386-mingw32msvc

export PATH="$TOOLS/bin:$TOOLS/$TARGET/bin:$PATH"

TOPDIR=`pwd`
SRCDIR="$TOPDIR/source"

SF_URL="http://surfnet.dl.sourceforge.net/sourceforge"
GNU_URL="ftp://ftp.gnu.org/gnu"
GLIB_URL="ftp://ftp.gtk.org/pub/glib/2.12/win32/"
GTK_URL="ftp://ftp.gtk.org/pub/gtk/v2.8/win32/"
PANGO_URL="ftp://ftp.gtk.org/pub/pango/1.12/win32/"
GNETSRC_URL="http://www.gnetlibrary.org/src/"
GNOME_URL="http://ftp.gnome.org/pub/GNOME/sources/"
TOR_URL="http://www.gimp.org/~tml/gimp/win32/"


MINGW_URL=$SF_URL/mingw
GNUWIN_URL=$SF_URL/gnuwin32/
ICONV_URL=$GTK_URL
GETTEXT_URL=$GTK_URL
SIGCPPSRC_URL=$GNOME_URL/libsigc++/2.0/
GLIBMMSRC_URL=$GNOME_URL/glibmm/2.10/
GTKMMSRC_URL=$GNOME_URL/gtkmm/2.8/
CAIROMMSRC_URL=http://cairographics.org/releases/

GLIB_FILES="glib-2.12.1.zip glib-dev-2.12.1.zip"
GTK_FILES="gtk+-2.8.20.zip gtk+-dev-2.8.20.zip atk-1.10.3.zip atk-dev-1.10.3.zip cairo-1.2.2.zip cairo-dev-1.2.2.zip"
PANGO_FILES="pango-1.12.3.zip pango-dev-1.12.3.zip"
GNUWIN_FILES="zlib-1.2.3-bin.zip zlib-1.2.3-lib.zip libpng-1.2.8-bin.zip libpng-1.2.8-lib.zip"
ICONV_FILES="libiconv-1.9.1.bin.woe32.zip"
GETTEXT_FILES=" gettext-0.14.5.zip gettext-dev-0.14.5.zip"
#GTKMM_FILES="gtkmm-2.2.7-mingw32-lib.zip gtkmm-2.2.7-mingw32-bin.zip libsigc++-1.2.5-mingw32-bin.zip libsigc++-1.2.5-mingw32-lib.zip"
GNETSRC_FILES="gnet-2.0.7.tar.gz"
GTKMMSRC_FILES="gtkmm-2.8.8.tar.bz2"
GLIBMMSRC_FILES="glibmm-2.10.5.tar.bz2"
SIGCPPSRC_FILES="libsigc++-2.0.17.tar.gz"
CAIROMMSRC_FILES="cairomm-1.2.2.tar.gz"

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
        download_files $GTK_URL $GTK_FILES
        download_files $PANGO_URL $PANGO_FILES
        download_files $GNUWIN_URL $GNUWIN_FILES
        download_files $TOR_URL $ICONV_FILES
        download_files $TOR_URL $GETTEXT_FILES

        download_files $GNETSRC_URL $GNETSRC_FILES
        download_files $GLIBMMSRC_URL $GLIBMMSRC_FILES
        download_files $GTKMMSRC_URL $GTKMMSRC_FILES
        download_files $SIGCPPSRC_URL $SIGCPPSRC_FILES
        download_files $CAIROMMSRC_URL $CAIROMMSRC_FILES
}

unpack()
{
        rm -rf $PREFIX
        mkdir $PREFIX
        	
        unzip_files $GTK_FILES
        unzip_files $GLIB_FILES
        unzip_files $PANGO_FILES
        unzip_files $GNUWIN_FILES
        unzip_files $ICONV_FILES
        unzip_files $GETTEXT_FILES
}


fix_pkgconfig()
{
    cd $PREFIX/lib/pkgconfig

    for a in $PREFIX/lib/pkgconfig/*.pc; do
        sed -e "s|c:/devel/target/.*$|$PREFIX|g" -e "s|
$||g" < $a > $a.new
        mv -f $a.new $a
        sed -e "s|/devel/target/.*$|$PREFIX|g" -e "s|
$||g" < $a > $a.new
        mv -f $a.new $a
        sed -e "s|/target/.*$|$PREFIX|g" -e "s|
$||g" < $a > $a.new
        mv -f $a.new $a
    done

    sed -e "s|c:/progra~1/.*$|$PREFIX|g" -e "s|
$||g" < libpng.pc > libpng.pc.new
    mv -f libpng.pc.new libpng.pc
    
    sed -e "s|c:/progra~1/.*$|$PREFIX|g" -e "s|
$||g" < libpng13.pc > libpng13.pc.new
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

    cd "$TOPDIR"

    # | sed "s/^\([a-z]\)/_\1/"
    pexports $PREFIX/bin/intl.dll  > intl.def
    pexports $PREFIX/bin/iconv.dll  > iconv.def
    pexports $PREFIX/bin/libatk-1.0-0.dll | sed "s/libatk-1.0-0/libatk/" > atk.def
    
    i386-mingw32msvc-dlltool -d intl.def -l $PREFIX/lib/libintl.dll.a
    i386-mingw32msvc-dlltool -U -d iconv.def -l $PREFIX/lib/libiconv.dll.a
    #i386-mingw32msvc-dlltool -d atk.def -l $PREFIX/lib/libatk-1.0.dll.a
    i386-mingw32msvc-ranlib $PREFIX/lib/libintl.dll.a
    i386-mingw32msvc-ranlib $PREFIX/lib/libiconv.dll.a
    #i386-mingw32msvc-ranlib $PREFIX/lib/libatk-1.0.dll.a
}

extract_package()
{
	cd "$SRCDIR"
	rm -rf "$1"
	echo "Extracting $1"
        case $2 in
            *.tar.gz)
                 tar xzvf $SRCDIR/$2
                 ;;
            *.tgz)
                 tar xzvf $SRCDIR/$2
                 ;;
            *.tar.bz2)
                 tar xjvf $SRCDIR/$2
                 ;;
            *.tar)
                 tar xvf $SRCDIR/$2
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
		cd "$SRCDIR/$1"
		patch -p1 < "$TOPDIR/$1.diff"
		cd "$TOPDIR"
	fi
}

build_sigcpp()
{
	cd "$TOPDIR"
	rm -rf "libsigc++-$TARGET"
	mkdir "libsigc++-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $SRCDIR/$1
        #for a in sigc++/config/sigcconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

        cd "$TOPDIR/libsigc++-$TARGET"

        echo "Configuring Libsigc++"
        (   . $TOPDIR/mingw32 -gtk2.8
            "$SRCDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --disable-static \
                --target=$TARGET --host=$TARGET --build=i386-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libsigc++-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libsigc++"
        (   . $TOPDIR/mingw32
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libsigc++-$TARGET/make.log"
		exit 1
	fi

        
	cd "$TOPDIR/libsigc++-$TARGET"
	echo "Installing Libsigc++"
        (   . $TOPDIR/mingw32
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
	cd "$TOPDIR"
	rm -rf "libglibmm-$TARGET"
	mkdir "libglibmm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        cd $SRCDIR/$1
        for a in glib/glibmmconfig.h.in; do
            echo Patching $a...
            sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
            mv $a.new $a
        done

	cd "$TOPDIR/libglibmm-$TARGET"

        echo "Configuring Libglibmm"
        (   . $TOPDIR/mingw32
            "$SRCDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i386-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libglibmm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libglibmm"
        (   . $TOPDIR/mingw32
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libglibmm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$TOPDIR/libglibmm-$TARGET"
	echo "Installing Libglibmm"
        (   . $TOPDIR/mingw32
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
	cd "$TOPDIR"
	rm -rf "libcairomm-$TARGET"
	mkdir "libcairomm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        #cd $SRCDIR/$1
        #for a in cairo/cairommconfig.h.in; do
        #    echo Patching $a...
        #    sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
        #    mv $a.new $a
        #done

	cd "$TOPDIR/libcairomm-$TARGET"

        echo "Configuring Libcairomm"
        (   . $TOPDIR/mingw32
            "$SRCDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i386-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libcairomm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libcairomm"
        (   . $TOPDIR/mingw32
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libcairomm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$TOPDIR/libcairomm-$TARGET"
	echo "Installing Libcairomm"
        (   . $TOPDIR/mingw32
            make install &> make-install.log
        )
        if test $? -ne 0; then
            echo "install failed - log available: libcairomm-$TARGET/make-install.log"
            exit 1
        fi
	cd "$TOPDIR"
}

build_gtkmm()
{
	cd "$TOPDIR"
	rm -rf "libgtkmm-$TARGET"
	mkdir "libgtkmm-$TARGET"

        # We want statis libs... remove #define XXX_DLL
        cd $SRCDIR/$1
        for a in gdk/gdkmmconfig.h.in gtk/gtkmmconfig.h.in; do
            echo Patching $a...
            sed -e "s|\(#define.*_DLL\)|//\1|g" < $a > $a.new
            mv $a.new $a
        done

        # This example does not build..
        #cd examples/book/clipboard
        #sed -e "s|^SUBDIRS.*$|SUBDIRS=|g" < Makefile.in > Makefile.in.new
        #mv -f Makefile.in.new Makefile.in
        
	cd "$TOPDIR/libgtkmm-$TARGET"

        echo "Configuring Libgtkmm"
        (   . $TOPDIR/mingw32
            "$SRCDIR/$1/configure" -v \
		--prefix="$PREFIX" --disable-shared --enable-static \
                --target=$TARGET --host=$TARGET --build=i386-linux \
		--with-headers="$PREFIX/$TARGET/include" \
                --disable-examples \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libgtkmm-$TARGET/configure.log"
		exit 1
	fi
        
	echo "Building Libgtkmm"
        (   . $TOPDIR/mingw32
            make &> make.log
        )
	if test $? -ne 0; then
		echo "make failed - log available: libgtkmm-$TARGET/make.log"
		exit 1
	fi

        
	cd "$TOPDIR/libgtkmm-$TARGET"
	echo "Installing Libgtkmm"
        (   . $TOPDIR/mingw32
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
	cd "$SRCDIR/$1/"

        echo "Configuring Libgnet"
        (   . $TOPDIR/mingw32
                ./configure -v --prefix=$PREFIX --disable-pthreads \
               --target=$TARGET --host=$TARGET --build=i386-linux \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld &> configure.log
        )
	if test $? -ne 0; then
		echo "configure failed - log available: libgnet-$TARGET/configure.log"
		exit 1
	fi

        cp -a config.h.win32 config.h
        
	echo "Building Libgnet"
        (   . $TOPDIR/mingw32
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
        cp -a gnet-2.0.pc $PREFIX/lib/pkgconfig
        cp -a gnet-2.0.m4 $PREFIX/share/aclocal
        cp -a gnetconfig.h $PREFIX/include/gnet-2.0
	cd "$TOPDIR"
}

download
unpack

fix_pkgconfig
fix_lib

extract_package "libsigc++-2.0.17" "libsigc++-2.0.17.tar.gz"
build_sigcpp "libsigc++-2.0.17"

extract_package "glibmm-2.10.5" "glibmm-2.10.5.tar.bz2"
build_glibmm "glibmm-2.10.5"

extract_package "cairomm-1.2.2" "cairomm-1.2.2.tar.gz"
build_cairomm "cairomm-1.2.2"

extract_package "gtkmm-2.8.8" "gtkmm-2.8.8.tar.bz2"
build_gtkmm "gtkmm-2.8.8"

extract_package "gnet-2.0.7" "gnet-2.0.7.tar.gz"
build_gnet "gnet-2.0.7"

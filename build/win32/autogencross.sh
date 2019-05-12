#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`/../..
test -z "$srcdir" && srcdir=.

autopoint --force
AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install -I/usr/local/share/aclocal/

conf_flags=" --target=i686-w64-mingw32 --host=i686-w64-mingw32 --build=i386-linux  --enable-maintainer-mode --enable-debug --without-x --enable-distribution --enable-exercises --disable-gstreamer --disable-dbus"
 
if test x$NOCONFIGURE = x; then
    echo Running $srcdir/configure $conf_flags "$@" ...
    $srcdir/configure $conf_flags "$@"  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
    echo Skipping configure process.
fi

#! /bin/sh

set -e

export CONFIG_SHELL=/bin/bash

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

echo "Adding libtools."
# libtoolize --automake --copy --force

echo "Building macros."
aclocal

#echo "Building config header."
autoheader

echo "Building makefiles."
automake --add-missing -f --copy

echo "Building configure."
autoconf

rm -f config.cache

bash ./configure --build=i686-pc-mingw32  --host=i686-pc-mingw32 --enable-maintainer-mode --prefix=/local --enable-debug --without-x --enable-distribution "$@"

make

##make install

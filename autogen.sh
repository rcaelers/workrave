#!/bin/sh

set -e
export CONFIG_SHELL=/bin/bash

#echo "Adding libtools."
# NOT-QUITE-YET
# libtoolize --automake --copy --force

echo "Building macros."
aclocal

echo "Building config header."
autoheader --force

echo "Building makefiles."
automake --add-missing --gnu --copy

echo "Building configure."
autoconf --force

rm -f config.cache

./configure --enable-maintainer-mode --enable-debug --enable-gnome --enable-distribution "$@"

echo type \`make\' to build Workrave

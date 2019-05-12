#!/bin/sh

set -e

autopoint --force
AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install -I/usr/local/share/aclocal/

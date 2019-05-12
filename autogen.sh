#!/bin/sh

set -e

autopoint --force
AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install

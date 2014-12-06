#!/bin/sh

export BUILD_FLAVOUR=dbg

jhbuild --file=mingw.jhbuildrc $*

#!/bin/sh

export BUILD_FLAVOUR=rls

jhbuild --file=mingw.jhbuildrc $*

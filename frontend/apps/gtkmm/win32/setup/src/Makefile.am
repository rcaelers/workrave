# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)

if PLATFORM_OS_WIN32

lib_LTLIBRARIES		= libzapper.la
libzapper_la_SOURCES    = zapper.c
libzapper_la_LDFLAGS    = -no-undefined -version-info 0:0:0
include_HEADERS         = zapper.h

endif

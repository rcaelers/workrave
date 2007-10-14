# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

if PLATFORM_OS_WIN32

noinst_LIBRARIES = 	libworkrave-common-win32.a

libworkrave_common_win32_a_SOURCES = \
			crashlog.c

libworkrave_common_win32_a_LIBADD =

libworkrave_common_win32_a_CXXFLAGS = @GLIB_CFLAGS@

endif

INCLUDES = 		-W -D_XOPEN_SOURCE=600 -I. -I../../include \
			-I../../include/win32 -I../../win32/harpoon/include

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.c) \
			$(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-common-win32.la

libworkrave_common_win32_la_SOURCES = \
			crashlog.c

libworkrave_common_win32_la_LIBADD =

libworkrave_common_win32_la_CXXFLAGS = @GLIB_CFLAGS@

endif

INCLUDES = 		-W -D_XOPEN_SOURCE=600 -I. -I$(top_srcdir)/common/include \
			-I$(top_srcdir)/common/include/win32 -I$(top_srcdir)/common/win32/harpoon/include

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.c) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

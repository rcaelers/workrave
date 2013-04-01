# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2007, 2008, 2011, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-utils-win32.la

libworkrave_utils_win32_la_SOURCES = \
			W32ActiveSetup.cc \
			W32CriticalSection.cc \
			crashlog.c

libworkrave_utils_win32_la_LIBADD =

libworkrave_utils_win32_la_CXXFLAGS = @GLIB_CFLAGS@

endif

AM_CPPFLAGS  = 		-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/hooks/harpoon/include

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.c) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

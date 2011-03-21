# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-backend-osx.la

INCLUDES=-xobjective-c++

if PLATFORM_OS_OSX
sourcesosx = 		OSXInputMonitor.cc OSXInputMonitorFactory.cc OSXConfigurator.cc
endif

libworkrave_backend_osx_la_SOURCES = \
			${sourcesosx}

libworkrave_backend_osx_la_CXXFLAGS = \
			-W -I$(top_srcdir)/backend/src -I$(top_srcdir)/backend/include \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@WR_COMMON_INCLUDES@ \
			-D_XOPEN_SOURCE=600 @GLIB_CFLAGS@ @OSX_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc)\
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

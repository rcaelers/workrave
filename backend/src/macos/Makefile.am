# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-backend-macos.la

AM_CPPFLAGS=-xobjective-c++

if PLATFORM_OS_MACOS
sourcesmacos = 		MacOSInputMonitor.cc MacOSInputMonitorFactory.cc MacOSConfigurator.cc
endif

libworkrave_backend_macos_la_SOURCES = \
			${sourcesmacos}

libworkrave_backend_macos_la_CXXFLAGS = \
			-W -I$(top_srcdir)/backend/src -I$(top_srcdir)/backend/include \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@WR_COMMON_INCLUDES@ \
			-D_XOPEN_SOURCE=600 @GLIB_CFLAGS@ @MacOS_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc)\
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-backend-input-monitor-osx.la

AM_CPPFLAGS=-xobjective-c++

if PLATFORM_OS_OSX
sourcesosx = 		OSXInputMonitor.cc OSXInputMonitorFactory.cc
endif

libworkrave_backend_input_monitor_osx_la_SOURCES = \
			${sourcesosx}

libworkrave_backend_input_monitor_osx_la_CXXFLAGS = \
			-W \
			-D_XOPEN_SOURCE=600 @GLIB_CFLAGS@ @OSX_CFLAGS@ \
			-I${top_srcdir}/backend/include \
			-I${top_srcdir}/backend/src/input-monitor

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc)\
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

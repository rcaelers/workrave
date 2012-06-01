# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-input-monitor-unix.la

if PLATFORM_OS_UNIX
sourcesxinput = 	UnixInputMonitorFactory.cc \
			X11InputMonitor.cc \
			RecordInputMonitor.cc \
			XScreenSaverMonitor.cc

X11LIBS = 		@X_LIBS@
endif

if PLATFORM_OS_OSX
sourcesdummy =          dummy.c
endif

libworkrave_input_monitor_unix_la_SOURCES = \
			${sourcesxinput} ${sourcesdummy}

libworkrave_input_monitor_unix_la_CXXFLAGS = \
			-W @X_CFLAGS@ -D_XOPEN_SOURCE=600 \
			@GLIB_CFLAGS@ @GTK_CFLAGS@ \
			-I${top_srcdir}/libs/input-monitor/src \
			-I${top_srcdir}/libs/input-monitor/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/config/include

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc)\
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

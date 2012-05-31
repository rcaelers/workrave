# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS	=		unix win32 osx

noinst_LTLIBRARIES = 	libworkrave-backend-input-monitor.la

sources	 =		InputMonitorFactory.cc \
			InputMonitor.cc

if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-backend-input-monitor-unix.la
cflags_platform =	-I$(top_srcdir)/backend/src/input-monitor/unix
endif

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-backend-input-monitor-win32.la
cflags_platform =	-I$(top_srcdir)/backend/src/input-monitor/win32
endif

if PLATFORM_OS_OSX
ldadd_platform = 	win32/libworkrave-backend-input-monitor-osx.la
cflags_platform =	-I$(top_srcdir)/backend/src/input-monitor/osx
endif

libworkrave_backend_input_monitor_la_LIBADD = \
			${ldadd_platform}

libworkrave_backend_input_monitor_la_SOURCES = ${sources}

libworkrave_backend_input_monitor_la_CXXFLAGS = \
			-W ${cflags_platform} \
			-I${top_srcdir}/libs/utils/include \
			-I$(top_srcdir)/libs/config/include \
			-I${top_srcdir}/backend/include \
			@X_CFLAGS@ \
			-D_XOPEN_SOURCE=600 @GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

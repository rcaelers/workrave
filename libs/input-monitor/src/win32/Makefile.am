# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-input-monitor-win32.la

sources =		W32InputMonitor.cc \
			W32AlternateMonitor.cc \
			W32LowLevelMonitor.cc \
			W32InputMonitorFactory.cc \
			Harpoon.cc
endif

libworkrave_input_monitor_win32_la_SOURCES = ${sources} 

libworkrave_input_monitor_win32_la_CXXFLAGS = \
			-I${top_srcdir}/libs/input-monitor/src \
			-I${top_srcdir}/libs/input-monitor/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/libs/hooks/harpoonHelper/include/ \
		  	@GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) \
			$(wildcard $(srcdir)/*.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-backend-win32.la

sources = 		W32Configurator.cc \
			W32InputMonitor.cc \
			W32AlternateMonitor.cc \
			W32LowLevelMonitor.cc \
			W32InputMonitorFactory.cc \
			Harpoon.cc
endif

libworkrave_backend_win32_la_SOURCES = ${sources} 

libworkrave_backend_win32_la_CXXFLAGS = \
			-W -I$(top_srcdir)/backend/src -I$(top_srcdir)/backend/include \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 -D_WIN32_WINNT=0x0501 \
			-I$(top_srcdir)/common/win32/harpoon/include/ \
			-I$(top_srcdir)/common/win32/harpoonHelper/include/ \
		  	@WR_COMMON_INCLUDES@ @GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) \
			$(wildcard $(srcdir)/*.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
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
			-W -I.. -I../../include \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-I$(top_srcdir)/common/win32/harpoon/include/ \
		  	@WR_COMMON_INCLUDES@ @GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc) \
			$(wildcard *.hh) $(wildcard *.h) \
			$(wildcard *.icc)

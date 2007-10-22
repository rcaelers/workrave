# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-frontend-common-win32.la

libworkrave_frontend_common_win32_la_SOURCES = \
			W32SoundPlayer.cc \
			Sound.cc

libworkrave_frontend_common_win32_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-W -I.. -I../../include \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
			@GLIB_CFLAGS@

endif
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_WIN32

noinst_LIBRARIES = 	libworkrave-frontend-common-win32.a

libworkrave_frontend_common_win32_a_SOURCES = \
			Win32SoundPlayer.cc


libworkrave_frontend_common_win32_a_CXXFLAGS = \
			-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-W -I.. -I../../include @WR_COMMON_CFLAGS@ \
			@WR_BACKEND_INCLUDES@  

endif
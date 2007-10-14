# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LIBRARIES = 	libworkrave-backend-osx.a

if PLATFORM_OS_OSX
sourcesosx = 		OSXInputMonitor.cc
endif

libworkrave_backend_osx_a_SOURCES = \
			${sourcesosx}

libworkrave_backend_osx_a_CXXFLAGS = \
			-W -I.. -I../../include \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
		  	@WR_COMMON_CFLAGS@ @GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc)\
			$(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES =	Makefile.in

noinst_LIBRARIES = 	libworkrave-backend-win32.a

if HAVE_WIN32
sources = 		Win32Configurator.cc Win32InputMonitor.cc
endif

libworkrave_backend_win32_a_SOURCES = \
				${sources} 

libworkrave_backend_win32_a_CXXFLAGS = \
			-W -I.. -I../../include \
			-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-I$(top_srcdir)/common/win32/harpoon/include/ \
		  	@WR_COMMON_CFLAGS@ @GLIB_CFLAGS@

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc) \
			$(wildcard *.hh) $(wildcard *.h) \
			$(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES 		= Makefile.in

noinst_LIBRARIES			= libcorewin32.a

if HAVE_WIN32
sources 			= Win32Configurator.cc Win32InputMonitor.cc
endif

libcorewin32_a_SOURCES 		= ${sources} 

libcorewin32_a_CXXFLAGS 		= -W -I.. -I../intf -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
				  -D_XOPEN_SOURCE=600 \
				  -I$(top_srcdir)/src/shared/win32/harpoon/ \
			  	  @SHARED_CFLAGS@ @GLIB_CFLAGS@

EXTRA_DIST 			= $(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

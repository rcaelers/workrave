# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES 	= Makefile.in

noinst_LIBRARIES		= libcorewin32.a

if HAVE_REGISTRY
sourcesregistry 		= Win32Configurator.cc
endif

libcorewin32_a_SOURCES 	= ${sourcesxinput} ${sourcesgconf}

libcorewin32_a_CXXFLAGS 	= -W -I.. -I../intf -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -D_XOPEN_SOURCE=600 \
			  @SHARED_CFLAGS@ @GLIB_CFLAGS@

EXTRA_DIST 		= $(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

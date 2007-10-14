# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		win32 unix

noinst_LIBRARIES = 	libworkrave-common.a

libworkrave_common_a_SOURCES = \
			debug.cc \
			GlibThread.cc \
			Util.cc \
			StringUtil.cc \
			glibnls.c 


libworkrave_common_a_LIBADD =

INCLUDES = 		-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_CFLAGS@ @GLIB_CFLAGS@ @GNOME_CFLAGS@

#@GNOMEUI_CFLAGS@ 
#			 -DLIBGNOME_DATADIR=\""$(datadir)"\"

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

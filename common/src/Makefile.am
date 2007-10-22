# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		win32 unix

noinst_LTLIBRARIES = 	libworkrave-common.la

libworkrave_common_la_SOURCES = \
			debug.cc \
			GlibThread.cc \
			Util.cc \
			StringUtil.cc \
			glibnls.c

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-common-win32.la
endif
if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-common-unix.la
endif

libworkrave_common_la_LIBADD = ${ldadd_platform}

INCLUDES = 		-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GNOME_CFLAGS@

#@GNOMEUI_CFLAGS@ 
#			 -DLIBGNOME_DATADIR=\""$(datadir)"\"

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

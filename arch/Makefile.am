# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = win32 unix

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	debug.cc \
	Util.cc \
	nls.c

libarch_a_LIBADD =

INCLUDES = -W -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -D_XOPEN_SOURCE=600 @GLIB_CFLAGS@ @GDOME_CFLAGS@

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

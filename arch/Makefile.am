# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2002-09-20 18:27:01 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = win32 unix

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	debug.cc \
	Util.cc

libarch_a_LIBADD =

INCLUDES = -W -DDATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -D_XOPEN_SOURCE=600 @GLIB_CFLAGS@ @GDOME_CFLAGS@

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2002-09-10 18:18:32 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libarchunix.a

libarchunix_a_SOURCES = \
	Condition.cc \
	Thread.cc

libarchunix_a_LIBADD =

INCLUDES = -W -D_XOPEN_SOURCE=600 -I..

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

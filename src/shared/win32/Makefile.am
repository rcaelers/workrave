# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2003-08-01 22:19:19 pennersr>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

if HAVE_WIN32

noinst_LIBRARIES = libsharedwin32.a

libsharedwin32_a_SOURCES = \
	gettimeofday.c \
	Thread.cc

libsharedwin32_a_LIBADD =

endif

INCLUDES = -W -D_XOPEN_SOURCE=600 -I..

EXTRA_DIST = $(wildcard *.cc) $(wildcard *.c) $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2003-06-24 23:20:14 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

if HAVE_UNIX

noinst_LIBRARIES = libsharedunix.a

libsharedunix_a_SOURCES = \
	Condition.cc \
	Thread.cc \
	Sound.cc

libsharedunix_a_LIBADD =

endif

INCLUDES = -W -D_XOPEN_SOURCE=600 -I..

EXTRA_DIST = $(wildcard *.cc) $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

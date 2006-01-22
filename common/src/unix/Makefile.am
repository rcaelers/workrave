# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2003-06-24 23:20:14 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_UNIX

noinst_LIBRARIES = 	libworkrave-common-unix.a

libworkrave_common_unix_a_SOURCES = \
			Condition.cc \
			Thread.cc \
			Sound.cc

libworkrave_common_unix_a_LIBADD =

libworkrave_common_unix_a_CXXFLAGS = @X_CFLAGS@

endif

INCLUDES = 		-W -D_XOPEN_SOURCE=600 -I. -I../../include \
			-I../../include/unix 

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.hh) \
			$(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

if HAVE_APP_TEXT

noinst_LIBRARIES = libapptext.a

libapptext_a_SOURCES = \
	DailyLimitWindow.cc \
	APP.cc \
	Apptext.cc \
	MicroPauseWindow.cc \
	PreludeWindow.cc \
	RestBreakWindow.cc

libapptext_a_LIBADD = 

libapptext_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app -D_XOPEN_SOURCE=600 @GCONF_CFLAGS@

libapptext_a_CFLAGS	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

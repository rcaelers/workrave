# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# Time-stamp: <2002-09-01 19:35:33 robc>
#
# $Id$
#

MAINTAINERCLEANFILES 	= aclocal.m4 config.h.in configure stamp-h.in Makefile.in \
        install-sh missing mkinstalldirs config.cache config.log config.status \
	config.guess config.sub depcomp ltmain.sh

SUBDIRS 	= arch gui src images debian

EXTRA_DIST	= \
	config.xml \
	autogen.sh \
	autogenwin32.sh \
	depcomp

INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/arch $(GTKMM_CFLAGS) $(GTK_CFLAGS) $(SIGC_CFLAGS)

LDADD = $(GTKMM_LIBS) $(GTK_LIBS) $(SIGC_LIBS)

noinst_PROGRAMS = 

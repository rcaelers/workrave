# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

DEFS 			= @DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/intl -I..

if HAVE_APP_TEXT

bin_PROGRAMS 		= workrave

workrave_SOURCES =


workrave_LDADD 		=  @APP_LIBS@ @GNET_LIBS@ @GTHREAD_LIBS@ \
		            @PTHREAD_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			   ${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}


workrave_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app -D_XOPEN_SOURCE=600 @GCONF_CFLAGS@

workrave_CFLAGS		= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app -D_XOPEN_SOURCE=600


endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

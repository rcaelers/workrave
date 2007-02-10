# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LIBRARIES = 	libworkrave-backend-unix.a

if HAVE_X
sourcesxinput = 	X11InputMonitor.cc 
X11LIBS = 		@X_LIBS@ -lXmu
endif

if HAVE_GCONF
sourcesgconf = 		GConfConfigurator.cc 
endif

libworkrave_backend_unix_a_SOURCES = \
			${sourcesxinput} ${sourcesgconf}

libworkrave_backend_unix_a_CXXFLAGS = \
			-W -I.. -I../../include @X_CFLAGS@ \
			-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_CFLAGS@ @GLIB_CFLAGS@ @GDOME_CFLAGS@ \
			@GCONF_CFLAGS@ @GDK_CFLAGS@

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc)\
			$(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
#
# $Id$
#

SUBDIRS = 		
MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_APP_TEXT

bin_PROGRAMS = 		workrave

if HAVE_X
X11LIBS = 		-lXmu
endif

workrave_SOURCES = 	GUI.cc main.cc

workrave_CXXFLAGS = 	-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W \
			@WR_FRONTEND_CFLAGS@ @GLIB_CFLAGS@ @GCONF_CFLAGS@ \
			@GNET_CFLAGS@ -I$(top_srcdir)/src \
			 -I. -D_XOPEN_SOURCE=600

workrave_CFLAGS	= 	-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			@WR_FRONTEND_CFLAGS@ @GLIB_CFLAGS@ @GCONF_CFLAGS@ \
			@GNET_CFLAGS@ -I$(top_srcdir)/src \
			-I. -D_XOPEN_SOURCE=600 -I${DISTRIBUTION_HOME}/gtkmm/src

$(bin_PROGRAMS): @WR_FRONTEND_LIBS@

workrave_LDADD =  	@WR_FRONTEND_LIBS@ @GLIB_LIBS@ @GNET_LIBS@ @GCONF_LIBS@ @PTHREAD_LIBS@ @X_LIBS@ \
			${X11LIBS}
endif

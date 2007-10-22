# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if HAVE_GNOME

noinst_LTLIBRARIES = 	libworkrave-frontend-common-gnome.la

libworkrave_frontend_common_gnome_la_SOURCES = \
			GnomeSoundPlayer.cc


libworkrave_frontend_common_gnome_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-DDATADIR="\"${datadir}\"" \
			@X_CFLAGS@ \
			-D_XOPEN_SOURCE=600 -W -I.. -I../../include \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
			@GNOME_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ 

endif

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST 		= $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if HAVE_GNOME

noinst_LIBRARIES 	= libappgnome.a

libappgnome_a_SOURCES 	= GnomeSoundPlayer.cc


libappgnome_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -D_XOPEN_SOURCE=600 \
			  -W -I.. -I../intf @SHARED_CFLAGS@ @CORE_INTF@ @GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ 

endif
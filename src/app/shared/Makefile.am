# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES 	= libapp.a

SUBDIRS = images sounds exercises win32 gnome intf

if HAVE_EXERCISES
sourcesexercises 	= Exercise.cc
endif

libapp_a_SOURCES 	= Text.cc \
			  SoundPlayer.cc \
			  $(sourcesexercises)


EXTRA_DIST 		= $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

libapp_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -D_XOPEN_SOURCE=600 \
			  -W -Iintf @SHARED_CFLAGS@ @CORE_INTF@ @GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ 

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES 	= Makefile.in

SUBDIRS 		= win32 unix

noinst_LIBRARIES 	= libshared.a

libshared_a_SOURCES 	= debug.cc \
	 		  Util.cc \
			  DayTimePred.cc \
			  WeekTimePred.cc \
			  TimePredFactory.cc \
			  nls.c

libshared_a_LIBADD 	=

INCLUDES 		= -W -DWORKRAVE_DATADIR="\"${pkgdatadir}\""  -D_XOPEN_SOURCE=600 \
			  @SHARED_CFLAGS@ @GLIB_CFLAGS@ @GNOMEUI_CFLAGS@ -DLIBGNOME_DATADIR=\""$(datadir)"\"

EXTRA_DIST 		= $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

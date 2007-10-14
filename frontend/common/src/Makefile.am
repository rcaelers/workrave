# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES =	Makefile.in

noinst_LIBRARIES = 	libworkrave-frontend-common.a

SUBDIRS = 		gnome kde win32 x11 osx

libworkrave_frontend_common_a_SOURCES = \
			Text.cc \
			SoundPlayer.cc \
			System.cc \
			TimerBoxControl.cc

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

libworkrave_frontend_common_a_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			-W -I. -I../include -Iwin32 -Ignome -Ikde -Iosx\
			@WR_COMMON_CFLAGS@ @WR_FRONTEND_COMMON_CFLAGS@\
			@WR_BACKEND_INCLUDES@ @GNOME_CFLAGS@ @GTK_CFLAGS@ \
			@GLIB_CFLAGS@ \
			-I$(top_srcdir)/common/win32/harpoon/include

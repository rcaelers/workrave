# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = gtkmm

noinst_LIBRARIES = libgui.a

libgui_a_SOURCES = \
	BreakControl.cc \
	GUIControl.cc \
	Statistics.cc \
	SoundPlayer.cc

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

libgui_a_CXXFLAGS = -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui -I$(top_srcdir)/arch @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ -D_XOPEN_SOURCE=600

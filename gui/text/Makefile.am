# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

if HAVE_GUI_TEXT

noinst_LIBRARIES = libtextgui.a

libtextgui_a_SOURCES = \
	DailyLimitWindow.cc \
	GUI.cc \
	TextGUI.cc \
	MicroPauseWindow.cc \
	PreludeWindow.cc \
	RestBreakWindow.cc

libtextgui_a_LIBADD = 

libtextgui_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui -D_XOPEN_SOURCE=600 @GCONF_CFLAGS@

libtextgui_a_CFLAGS	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

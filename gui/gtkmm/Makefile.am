# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

if HAVE_GUI_GTKMM

noinst_LIBRARIES = libgtkmmgui.a

if HAVE_GNOME
else
sourcesgnome = gnome-about.c
endif

libgtkmmgui_a_SOURCES = \
	BreakControl.cc \
	BreakWindow.cc \
	GUI.cc \
	GUIControl.cc \
	GtkmmGUI.cc \
	MainWindow.cc \
	MicroPauseWindow.cc \
	PreferencesDialog.cc \
	PreludeWindow.cc \
	RestBreakWindow.cc \
	Statistics.cc \
	StatisticsDialog.cc \
	TimeBar.cc \
	TimeEntry.cc \
	TimerPreferencesPanel.cc \
	WindowHints.cc \
	gnome-winhints.c \
	Frame.cc \
	WmSpec.cc \
	$(sourcesgnome)



libgtkmmgui_a_LIBADD = 

libgtkmmgui_a_CXXFLAGS 	= -DDATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

libgtkmmgui_a_CFLAGS	= -DDATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

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
# FIXME: Raymond. have a look at this...
sourcesgnome = GnomeSoundPlayer.cc
else
sourcesgnome = gnome-about.c GnomeSoundPlayer.cc
endif

if PLATFORM_WIN32
sourceswin32 = Win32SoundPlayer.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = CollectiveJoinDialog.cc CollectivePreferencePage.cc
endif

libgtkmmgui_a_SOURCES = \
	BreakWindow.cc \
	GUI.cc \
	GtkmmGUI.cc \
	MainWindow.cc \
	MicroPauseWindow.cc \
	PreferencesDialog.cc \
	PreludeWindow.cc \
	RestBreakWindow.cc \
	StatisticsDialog.cc \
	TimeBar.cc \
	TimeEntry.cc \
	TimerPreferencesPanel.cc \
	WindowHints.cc \
	gnome-winhints.c \
	Frame.cc \
	WmSpec.cc \
	$(sourcesgnome) $(sourceswin32) ${sourcesdistribution}



libgtkmmgui_a_LIBADD = 

libgtkmmgui_a_CXXFLAGS 	= -DDATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@

libgtkmmgui_a_CFLAGS	= -DDATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# $Id$
#

IDL_GENERATED =				\
	Workrave-Applet.h	\
	Workrave-Applet-common.c	\
	Workrave-Applet-skels.c	\
	Workrave-Applet-stubs.c

MAINTAINERCLEANFILES = Makefile.in ${IDL_GENERATED}

if HAVE_GUI_GTKMM

noinst_LIBRARIES = libgtkmmgui.a

if HAVE_GNOME
sourcesgnome = GnomeSoundPlayer.cc AppletWindow.cc eggtrayicon.c

## CORBA stuff
IDLS =	../gnome/Workrave-Applet.idl

$(IDL_GENERATED): $(IDLS) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) ../gnome/Workrave-Applet.idl
## End CORBA stuff
else
sourcesgnome = gnome-about.c
endif

if PLATFORM_WIN32
sourceswin32 = Win32SoundPlayer.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = NetworkJoinDialog.cc NetworkLogDialog.cc NetworkPreferencePage.cc
endif

libgtkmmgui_a_SOURCES = \
	BreakWindow.cc \
	DailyLimitWindow.cc \
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
	${IDL_GENERATED} \
	$(sourcesgnome) $(sourceswin32) ${sourcesdistribution}



libgtkmmgui_a_LIBADD = 

libgtkmmgui_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@

libgtkmmgui_a_CFLAGS	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# $Id$
#

IDL_A 		= ../gnome/Workrave-Applet.idl

IDL_A_GENERATED = Workrave-Applet.h			\
		  Workrave-Applet-common.c		\
	  	  Workrave-Applet-skels.c		\
		  Workrave-Applet-stubs.c

IDL_C 		= ../gnome/Workrave-Control.idl

IDL_C_GENERATED = Workrave-Control.h			\
		  Workrave-Control-common.c		\
	  	  Workrave-Control-skels.c		\
		  Workrave-Control-stubs.c

MAINTAINERCLEANFILES = Makefile.in ${IDL_A_GENERATED} ${IDL_C_GENERATED}

if HAVE_GUI_GTKMM

noinst_LIBRARIES = libgtkmmgui.a

if HAVE_GNOME
sourcesgnome = GnomeSoundPlayer.cc RemoteControl.cc
sourcesidl = ${IDL_C_GENERATED} ${IDL_A_GENERATED}

## CORBA stuff

$(IDL_A_GENERATED): $(IDL_A) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) ../gnome/Workrave-Applet.idl

$(IDL_C_GENERATED): $(IDL_C) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) ../gnome/Workrave-Control.idl

## End CORBA stuff
else
sourcesgnome = gnome-about.c
endif

if HAVE_X
sourcesx = AppletWindow.cc AppletPreferencePage.cc eggtrayicon.c
endif

if PLATFORM_WIN32
sourceswin32 = Win32SoundPlayer.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = NetworkJoinDialog.cc NetworkLogDialog.cc NetworkPreferencePage.cc
endif

libgtkmmgui_a_SOURCES = \
	$(sourcesidl) \
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
	TimerWindow.cc \
	Menus.cc \
	$(sourcesgnome) $(sourcesx) $(sourceswin32) ${sourcesdistribution}



libgtkmmgui_a_LIBADD = 

libgtkmmgui_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@

libgtkmmgui_a_CFLAGS	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/gui @gnomeui_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

endif

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

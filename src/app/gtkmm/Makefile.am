# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

SUBDIRS 		= gnome_applet
MAINTAINERCLEANFILES 	= Makefile.in ${IDL_A_GENERATED} ${IDL_C_GENERATED}


bin_PROGRAMS 		= workrave

IDL_A 			= gnome_applet/Workrave-Applet.idl

IDL_A_GENERATED 		= Workrave-Applet.h			\
			  Workrave-Applet-common.c		\
		  	  Workrave-Applet-skels.c			\
			  Workrave-Applet-stubs.c

IDL_C 			= gnome_applet/Workrave-Control.idl

IDL_C_GENERATED 		= Workrave-Control.h			\
			  Workrave-Control-common.c		\
		  	  Workrave-Control-skels.c		\
			  Workrave-Control-stubs.c

DEFS 			= @DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/intl -I..

if HAVE_APP_GTKMM

if HAVE_GNOME
sourcesgnome 		= RemoteControl.cc
sourcesidl 		= ${IDL_C_GENERATED} ${IDL_A_GENERATED}

## CORBA stuff

$(IDL_A_GENERATED): $(IDL_A) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) gnome_applet/Workrave-Applet.idl

$(IDL_C_GENERATED): $(IDL_C) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) gnome_applet/Workrave-Control.idl

## End CORBA stuff
else
sourcesgnome 		= gnome-about.c
endif

if HAVE_X
sourcesx 		= AppletWindow.cc eggtrayicon.c
endif

if PLATFORM_WIN32
sourceswin32 		= resource.rc
includeswin32 		= -I$(top_srcdir)/shared/win32/harpoon/
endif

if HAVE_DISTRIBUTION
sourcesdistribution 	= NetworkJoinDialog.cc NetworkLogDialog.cc NetworkPreferencePage.cc
endif

if HAVE_EXERCISES
sourcesexercises 	= ExercisesDialog.cc ExercisesPanel.cc
endif

if PLATFORM_WIN32
WINDRES ?= "windres"
.rc.o:
	${WINDRES} -o $@ $<

includesinput 		= -I$(top_srcdir)/src/shared/win32/harpoon/
WIN32LIBS 		= $(top_srcdir)/src/shared/win32/harpoon/Release/harpoon.lib \
			  -lshell32 -lole32 -lwinmm
endif

if HAVE_X
X11LIBS    = -lXmu
endif

workrave_SOURCES = \
		$(sourcesidl) \
		BreakWindow.cc \
		DailyLimitWindow.cc \
		GUI.cc \
		GtkUtil.cc \
		EventLabel.cc \
		Hig.cc \
		IconListNotebook.cc \
		IconListCellRenderer.cc \
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
		TimerBox.cc \
		TimerBoxPreferencePage.cc \
		Menus.cc \
		Dispatcher.cc \
		main.cc \
		$(sourcesgnome) \
		$(sourcesx) \
		$(sourceswin32) \
		${sourcesdistribution} \
		${sourcesexercises}

workrave_CXXFLAGS = -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W @APP_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app @GNOMEUIMM_CFLAGS@ @GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ @GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ $(includeswin32) $(includesinput)

workrave_CFLAGS	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @APP_CFLAGS@ -I$(top_srcdir)/src -I$(top_srcdir)/app @GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600

endif

$(bin_PROGRAMS): @APP_LIBS@

workrave_LDADD 		=  @APP_LIBS@ @GNOMEUI_LIBS@ @GNOMEUIMM_LIBS@ @GTK_LIBS@ \
			   @GTKMM_LIBS@ @GNET_LIBS@ @GTHREAD_LIBS@ \
		            @PTHREAD_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			   ${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}

# workrave_LINK		= $(LINK) \
# 			  nm - nos > workrave.dmp \
# 			  strip workrave$(EXEEXT)

#workrave$(EXEEXT): $(workrave_OBJECTS) $(workrave_DEPENDENCIES) 
#	@rm -f workrave$(EXEEXT)
#	$(CXXLINK) $(workrave_LDFLAGS) $(workrave_OBJECTS) $(workrave_LDADD) $(LIBS)

EXTRA_DIST = $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

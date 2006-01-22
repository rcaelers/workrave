# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
#
# $Id$
#

SUBDIRS = 		gnome_applet kde_applet win32 unix
MAINTAINERCLEANFILES = 	Makefile.in ${IDL_A_GENERATED} ${IDL_C_GENERATED}

if HAVE_APP_GTKMM

bin_PROGRAMS = 		workrave

endif

# Additional IDL sources.
IDL_A = 		gnome_applet/Workrave-Applet.idl

IDL_A_GENERATED = 	Workrave-Applet.h			\
			Workrave-Applet-common.c		\
			Workrave-Applet-skels.c			\
			Workrave-Applet-stubs.c

IDL_C = 		gnome_applet/Workrave-Control.idl

IDL_C_GENERATED = 	Workrave-Control.h			\
			Workrave-Control-common.c		\
		  	Workrave-Control-skels.c		\
			Workrave-Control-stubs.c

DEFS = 			@DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
			-I$(top_srcdir)/intl -I..

extrasources =
extranodistsources = 

# Additional Gnome sources
sourcesgnome = 		RemoteControl.cc
sourcesgnome_idl = 	${IDL_C_GENERATED} ${IDL_A_GENERATED}

# Additional non-Gnome sources.
sourcesnognome = 	gnome-about.c

# Additional KDE sources.
if HAVE_KDE
sourceskde = 		KdeAppletWindow.cc KdeWorkraveControl.cc
sourcesnodistkde = 	kde_applet/kworkraveapplet_stub.cpp \
			kde_applet/kworkravecontroliface_stub.cpp \
			kde_applet/kworkravecontroliface_skel.cpp \
			KdeWorkraveControl.moc
else
sourcesnodistkde =
endif

# Additional X11 sources.
sourcesx = 		AppletWindow.cc eggtrayicon.c

# Additional Win32 sources.
sourceswin32 = 		resource.rc

# Additional distribution sources.
DISTRIBUTION_HOME = 	../../plugin/distribution
sourcesdistribution = 	${DISTRIBUTION_HOME}/gtkmm/src/NetworkJoinDialog.cc \
			${DISTRIBUTION_HOME}/gtkmm/src/NetworkLogDialog.cc \
			${DISTRIBUTION_HOME}/gtkmm/src/NetworkPreferencePage.cc

# Additional exercises sources.
EXERCISES_HOME =	../../plugin/exercises
sourcesexercises = 	${EXERCISES_HOME}/gtkmm/src/ExercisesDialog.cc \
			${EXERCISES_HOME}/gtkmm/src/ExercisesPanel.cc \
			${EXERCISES_HOME}/common/src/Exercise.cc

# Statistics.
STATISTICS_HOME = 	../../plugin/statistics


if HAVE_APP_GTKMM

if HAVE_GNOME
extrasources +=		$(sourcesgnome)
extranodistsources +=	$(sourcesgnome_idl)

AppletWindow.cc:	$(sourcesgnome_idl)
GUI.cc:			$(sourcesgnome_idl)
else
extrasources +=		$(sourcesnognome)
endif

if HAVE_KDE
extrasources +=		$(sourceskde)
extranodistsources +=	$(sourcesnodistkde)
endif

if HAVE_X
extrasources +=		$(sourcesx)
includesx = 		-Iunix/
workrave_LDFLAGS = 	-Lunix/
endif

if PLATFORM_WIN32
extrasources +=		$(sourceswin32)
includeswin32 = 	-I$(top_srcdir)/common/win32/harpoon/include -Iwin32/ \
			-I../../win32/applet/include
workrave_LDFLAGS = 	-Lwin32/
endif

if HAVE_DISTRIBUTION
extrasources +=		$(sourcesdistribution)
endif

if HAVE_EXERCISES
extrasources +=		$(sourcesexercises)
endif

if PLATFORM_WIN32
WINDRES ?= 		"windres"
.rc.o:
	${WINDRES} -o $@ $<

includesinput = 	-I$(top_srcdir)/common/win32/harpoon/include
WIN32LIBS = 		-L$(top_srcdir)/common/win32/harpoon/src/Release/ -lharpoon \
			-lworkrave-frontend-gtkmm-win32 \
			-lshell32 -lole32 -lwinmm
endif

if HAVE_X
X11LIBS = 		-lworkrave-frontend-gtkmm-unix -lX11 -lXmu
endif

if HAVE_WIN32
win32cflags		= -D_WIN32_IE=0x0500
endif

#if HAVE_KDE
#
#KdeWorkraveControl.kidl: $(srcdir)/KdeWorkraveControl.hh $(DCOP_DEPENDENCIES)
#	$(DCOPIDL) $(srcdir)/KdeWorkraveControl.hh > KdeWorkraveControl.kidl || ( rm -f KdeWorkraveControl.kidl ; false )
#KdeWorkraveControl_skel.cpp: KdeWorkraveControl.kidl
#	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-stub KdeWorkraveControl.kidl
#KdeWorkraveControl_stub.cpp: KdeWorkraveControl_stub.h 
#KdeWorkraveControl_stub.h: KdeWorkraveControl.kidl
#	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-skel KdeWorkraveControl.kidl
#KdeWorkraveControl.moc.cpp: $(srcdir)/KdeWorkraveControl.hh
#	$(MOC) $(srcdir)/KdeWorkraveControl.hh -o KdeWorkraveControl.moc.cpp
#
#extrasources 	 +=	KdeWorkraveControl.moc.cpp KdeWorkraveControl_skel.cpp KdeWorkraveControl_stub.cpp
##extranodistsources +=	
#
#endif

workrave_SOURCES = \
			BreakWindow.cc \
			DailyLimitWindow.cc \
			GUI.cc \
			GtkUtil.cc \
			EventLabel.cc \
			EventImage.cc \
			HeadInfo.cc \
			Hig.cc \
			IconListNotebook.cc \
			IconListCellRenderer.cc \
			MainWindow.cc \
			MicroBreakWindow.cc \
			PreferencesDialog.cc \
			PreludeWindow.cc \
			RestBreakWindow.cc \
			${STATISTICS_HOME}/gtkmm/src/StatisticsDialog.cc \
			TimeBar.cc \
			TimeEntry.cc \
			TimerPreferencesPanel.cc \
			WindowHints.cc \
			gnome-winhints.c \
			Frame.cc \
			WmSpec.cc \
			TimerBoxGtkView.cc \
			TimerBoxPreferencePage.cc \
			Menus.cc \
			Dispatcher.cc \
			main.cc \
			$(extrasources)

nodist_workrave_SOURCES = \
			$(extranodistsources)


workrave_CXXFLAGS = 	-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -W \
			@WR_FRONTEND_CFLAGS@ -I$(top_srcdir)/src \
			 -I. @X_CFLAGS@ @GNOMEUIMM_CFLAGS@ \
			@GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GTKMM_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@KDE_INCLUDES@ @QT_INCLUDES@\
			$(includeswin32) $(win32cflags) $(includesinput) \
			$(includesx) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src 

workrave_CFLAGS	= 	-DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			@WR_FRONTEND_CFLAGS@ -I$(top_srcdir)/src \
			-I. @X_CFLAGS@ @GNOMEUI_CFLAGS@ \
			@GTK_CFLAGS@ @GTKMM_CFLAGS@ -D_XOPEN_SOURCE=600 \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)

endif

$(bin_PROGRAMS): @WR_FRONTEND_LIBS@

workrave_LDADD =  	@WR_FRONTEND_LIBS@ @X_LIBS@ @GNOMEUI_LIBS@ @GNOMEUIMM_LIBS@ \
			@GTK_LIBS@ @GTKMM_LIBS@ @GNET_LIBS@ @GTHREAD_LIBS@ \
			@PTHREAD_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			@KDE_LDFLAGS@ @LIB_KDEUI@ @LIB_KDECORE@ \
			@QT_LDFLAGS@ @LIB_QT@ \
			${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}

# workrave_LINK		= $(LINK) \
# 			  nm - nos > workrave.dmp \
# 			  strip workrave$(EXEEXT)

#workrave$(EXEEXT): $(workrave_OBJECTS) $(workrave_DEPENDENCIES) 
#	@rm -f workrave$(EXEEXT)
#	$(CXXLINK) $(workrave_LDFLAGS) $(workrave_OBJECTS) $(workrave_LDADD) $(LIBS)


$(IDL_A_GENERATED): $(IDL_A) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) \
		gnome_applet/Workrave-Applet.idl

$(IDL_C_GENERATED): $(IDL_C) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) \
		gnome_applet/Workrave-Control.idl


EXTRA_DIST =      ${sourcesgnome} \
		${sourcesnognome} \
		${sourceskde} \
		${sourcesx} \
		${sourceswin32} \
		${sourcesdistribution} \
		${sourcesexercises} \
		*.hh *.h

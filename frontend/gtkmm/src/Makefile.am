
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
#

SUBDIRS = 		gnome_applet gnome_applet3 kde_applet win32 unix osx

MAINTAINERCLEANFILES = 	Makefile.in \
			org.workrave.Workrave.service \
			workrave.desktop \
			resource.rc

DISTCLEANFILES = 	workrave.desktop

if HAVE_APP_GTKMM

bin_PROGRAMS = 		workrave

endif


eggcflags = 

DEFS = 			@DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
			-I$(top_srcdir)/intl

extrasources =
extranodistsources = 

# Additional Gnome sources
sourcesgnome = 		GnomeAppletWindow.cc GnomeAppletMenu.cc

# Additional non-Gnome sources.
sourcesnognome =

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
if HAVE_GTK3
sourcesx = 		X11SystrayAppletWindow.cc gtktrayicon3.c
else
sourcesx = 		X11SystrayAppletWindow.cc gtktrayicon.c
endif

# Additional Win32 sources.
sourceswin32 = 		resource.rc

resource.rc: resource.rc.in
	sed -e 's/@WORKRAVE_VERSION@/$(VERSION)/g' \
	    -e 's/@WORKRAVE_RESOURCE_VERSION@/$(RESOURCE_VERSION)/g' \
		$(srcdir)/$@.in > $@

# Additional distribution sources.
DISTRIBUTION_HOME = 	$(top_srcdir)/frontend/plugin/distribution
sourcesdistribution = 	${DISTRIBUTION_HOME}/gtkmm/src/NetworkJoinDialog.cc \
			${DISTRIBUTION_HOME}/gtkmm/src/NetworkLogDialog.cc \
			${DISTRIBUTION_HOME}/gtkmm/src/NetworkPreferencePage.cc

# Additional exercises sources.
EXERCISES_HOME =	$(top_srcdir)/frontend/plugin/exercises
sourcesexercises = 	${EXERCISES_HOME}/gtkmm/src/ExercisesDialog.cc \
			${EXERCISES_HOME}/gtkmm/src/ExercisesPanel.cc \
			${EXERCISES_HOME}/common/src/Exercise.cc

# Statistics.
STATISTICS_HOME = 	$(top_srcdir)/frontend/plugin/statistics


if HAVE_PYTHON_CHEETAH

DBusGnomeApplet.cc:	gnome_applet/gnome-applet.xml \
			$(top_srcdir)/common/bin/dbusgen.py \
		    	$(top_srcdir)/common/bin/DBus-client-template.cc \
		    	$(top_srcdir)/common/bin/DBus-client-template.hh
			python $(top_srcdir)/common/bin/dbusgen.py --language=C++ -c $(srcdir)/gnome_applet/gnome-applet.xml DBUSGnomeApplet

endif

DBusGnomeApplet.hh:	DBusGnomeApplet.cc
GnomeAppletWindow.cc:   DBusGnomeApplet.hh

if HAVE_APP_GTKMM

if HAVE_GNOMEAPPLET
extrasources +=		$(sourcesgnome)
else
extrasources +=		$(sourcesnognome)
endif

if HAVE_KDE
extrasources +=		$(sourceskde)
extranodistsources +=	$(sourcesnodistkde)
endif

if PLATFORM_OS_UNIX
extrasources +=		$(sourcesx) eggdesktopfile.c eggsmclient-xsmp.c
eggcflags += 		-DEGG_SM_CLIENT_BACKEND_XSMP

includesx = 		-I$(top_srcdir)/frontend/gtkmm/src/unix/
ldflags = 	        -L$(top_builddir)/frontend/gtkmm/src/unix/
endif

if PLATFORM_OS_WIN32
extrasources +=		$(sourceswin32) eggsmclient-win32.c
includeswin32 = 	-I$(top_srcdir)/common/win32/harpoon/include \
			-I$(top_srcdir)/frontend/gtkmm/src/win32/ \
			-I$(top_srcdir)/frontend/win32/applet/include
ldflags = 		-L$(top_builddir)/frontend/gtkmm/src/win32/
win32cflags		= -D_WIN32_IE=0x0600 -D_WIN32_WINNT=0x0600
endif

if HAVE_DISTRIBUTION
extrasources +=		$(sourcesdistribution)
endif

if HAVE_EXERCISES
extrasources +=		$(sourcesexercises)
endif

if PLATFORM_OS_WIN32
WINDRES ?= 		"windres"
.rc.o:
	${WINDRES} -o $@ $<

includesinput = 	-I$(top_srcdir)/common/win32/harpoon/include
WIN32LIBS = 		$(top_builddir)/common/win32/harpoon/src/Release/harpoon.dll \
			$(top_builddir)/frontend/gtkmm/src/win32/libworkrave-frontend-gtkmm-win32.la \
			-lshell32 -lole32 -lwinmm -lshlwapi -lwtsapi32
endif

if PLATFORM_OS_UNIX
X11LIBS = 		-lworkrave-frontend-gtkmm-unix -lX11
endif

if PLATFORM_OS_OSX
includesosx  = 		-I$(top_srcdir)/frontend/gtkmm/src/osx
ldflags =	 	-L$(top_builddir)/frontend/gtkmm/src/osx
OSXLIBS	=		$(top_builddir)/frontend/gtkmm/src/osx/libworkrave-frontend-gtkmm-osx.la
extrasources +=	        eggsmclient-osx.c
endif


# DBus service file
if HAVE_DBUS
servicedir = $(DBUS_SERVICES_DIR)
service_in_files = org.workrave.Workrave.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	sed -e "s|@workravebindir@|$(bindir)|" $< > $@

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = org.workrave.Workrave.service

dbussources = DBusGUI.cc DBusGUI.hh DBusGnomeApplet.cc DBusGnomeApplet.hh

BUILT_SOURCES = $(dbussources)

# broken: 
# if PLATFORM_OS_UNIX
# extrasources +=		eggsmclient-dbus.c
# eggcflags += -DEGG_SM_CLIENT_BACKEND_DBUS
# endif

if HAVE_PYTHON_CHEETAH

DBusGUI.cc: 		workrave-gui.xml \
			$(top_srcdir)/common/bin/dbusgen.py \
			$(top_srcdir)/common/bin/DBus-template.cc \
			$(top_srcdir)/common/bin/DBus-template.hh
			python $(top_srcdir)/common/bin/dbusgen.py --language=C++ -s $(srcdir)/workrave-gui.xml DBusGUI

DBusGUI.hh: 		DBusGUI.cc ; test -f $@

endif # HAVE_PYTHON_CHEETAH

else # HAVE_DBUS

dbussources =

endif # HAVE_DBUS

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

iconsdir = $(datadir)/pixmaps/workrave

desktopdir = ${datadir}/applications
desktop_in_files = workrave.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

workrave_SOURCES = \
			AppletWindow.cc \
			AppletControl.cc \
			BreakWindow.cc \
			DailyLimitWindow.cc \
			DataConnector.cc \
			GUI.cc \
			GUIConfig.cc \
			GtkUtil.cc \
			EventLabel.cc \
			EventImage.cc \
			EventButton.cc \
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
			Session.cc \
			TimeBar.cc \
			TimeEntry.cc \
			TimerPreferencesPanel.cc \
			WindowHints.cc \
			Frame.cc \
			TimerBoxGtkView.cc \
			TimerBoxPreferencePage.cc \
			Menus.cc \
			main.cc \
			MainGtkMenu.cc \
			StatusIcon.cc \
			eggsmclient.c \
			${dbussources} \
			$(extrasources)


nodist_workrave_SOURCES = \
			$(extranodistsources)


workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DWORKRAVE_DATADIR="\"${datadir}\""  \
			-I$(top_srcdir)/frontend/gtkmm/src/ \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@KDE_INCLUDES@ @QT_INCLUDES@ @DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@  @IGE_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) $(includesosx) \
			$(includesx) $(eggcflags) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src

if PLATFORM_OS_OSX
INCLUDES =		-xobjective-c++
endif

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I$(top_srcdir)/frontend/gtkmm/src/ @GLIB_CFLAGS@ \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GTK_CFLAGS@ @DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@ @X11SM_CFLAGS@\
			-D_XOPEN_SOURCE=600  $(eggcflags) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)

endif

$(bin_PROGRAMS):	$(top_builddir)/backend/src/libworkrave-backend.la \
			$(top_builddir)/common/src/libworkrave-common.la \
			$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la

workrave_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags}

workrave_LDADD =        @WR_LDADD@ @X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
			@GTK_LIBS@ @GNET_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@KDE_LDFLAGS@ @LIB_KDEUI@ @LIB_KDECORE@ @PULSE_LIBS@ \
			@QT_LDFLAGS@ @LIB_QT@ @DBUS_LIBS@ @DBUSGLIB_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
			${X11LIBS} ${WIN32LIBS} @GTK_LIBS@ ${OSXLIBS} ${WIN32CONSOLE} \
			${DIRECTSOUNDLIBS} 

# workrave_LINK		= $(LINK) \
# 			  nm - nos > workrave.dmp \
# 			  strip workrave$(EXEEXT)

#workrave$(EXEEXT): $(workrave_OBJECTS) $(workrave_DEPENDENCIES) 
#	@rm -f workrave$(EXEEXT)
#	$(CXXLINK) $(workrave_LDFLAGS) $(workrave_OBJECTS) $(workrave_LDADD) $(LIBS)

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h) \
	     workrave.desktop.in org.workrave.Workrave.service.in workrave-gui.xml resource.rc.in


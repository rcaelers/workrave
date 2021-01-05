
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
#

SUBDIRS = 		win32 unix macos

MAINTAINERCLEANFILES = 	Makefile.in \
			org.workrave.Workrave.service \
			workrave.desktop \
			resource.rc

DISTCLEANFILES = 	workrave.desktop org.workrave.gui.gschema.xml

if HAVE_APP_GTKMM

bin_PROGRAMS = 		workrave

endif

DEFS = @DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
			-I$(top_srcdir)/intl

extrasources =
extranodistsources = 

# Additional Indicator applet sources
sourcesindicator = 	IndicatorAppletMenu.cc

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
sourcesdistribution = 	../../plugin/distribution/gtkmm/src/NetworkJoinDialog.cc \
			../../plugin/distribution/gtkmm/src/NetworkLogDialog.cc \
			../../plugin/distribution/gtkmm/src/NetworkPreferencePage.cc

# Additional exercises sources.
EXERCISES_HOME =	$(top_srcdir)/frontend/plugin/exercises
sourcesexercises = 	../../plugin/exercises/gtkmm/src/ExercisesDialog.cc \
			../../plugin/exercises/gtkmm/src/ExercisesPanel.cc \
			../../plugin/exercises/common/src/Exercise.cc

# Statistics.
STATISTICS_HOME = 	$(top_srcdir)/frontend/plugin/statistics
sourcesstatistics =	../../plugin/statistics/gtkmm/src/StatisticsDialog.cc

if HAVE_APP_GTKMM

if HAVE_INDICATOR
extrasources += 	$(sourcesindicator)
indicatorflags = 	-I$(top_srcdir)/frontend/applets/indicator/include
endif

if PLATFORM_OS_UNIX
extrasources +=		$(sourcesx)
includesx = 		-I$(top_srcdir)/frontend/gtkmm/src/unix/
ldflags = 	        -L$(top_builddir)/frontend/gtkmm/src/unix/
endif

if PLATFORM_OS_WINDOWS
extrasources +=		eggsmclient-win32.c eggsmclient.c
includeswin32 = 	-I$(top_srcdir)/common/win32/harpoon/include \
					-I$(top_srcdir)/frontend/gtkmm/src/win32/ \
					-I$(top_srcdir)/frontend/applets/win32/include \
					-I$(top_srcdir)/backend/src/win32
ldflags = 		-L$(top_builddir)/frontend/gtkmm/src/win32/
win32cflags		= -D_WIN32_IE=0x0600 -D_WIN32_WINNT=0x0600
endif

if HAVE_DISTRIBUTION
extrasources +=		$(sourcesdistribution)
endif

if HAVE_EXERCISES
extrasources +=		$(sourcesexercises) 
endif

extrasources +=		$(sourcesstatistics) 


if PLATFORM_OS_WINDOWS
WINDRES ?= 		"windres"
.rc.o:
	${WINDRES} -o $@ $<

extrasources +=		$(sourceswin32)
includesinput = 	-I$(top_srcdir)/common/win32/harpoon/include
WIN32LIBS = 		$(top_builddir)/common/win32/harpoon/src/Release/harpoon.dll \
			$(top_builddir)/frontend/gtkmm/src/win32/libworkrave-frontend-gtkmm-win32.la \
			-lshell32 -lole32 -lwinmm -lshlwapi -lwtsapi32

WIN32LDFLAGS = 		-static-libgcc -static-libstdc++
endif

if PLATFORM_OS_UNIX
X11LIBS = 		-lworkrave-frontend-gtkmm-unix -lX11
endif

if PLATFORM_OS_MACOS
includesmacos  = 		-I$(top_srcdir)/frontend/gtkmm/src/macos
ldflags =	 	-L$(top_builddir)/frontend/gtkmm/src/macos
MacOSLIBS	=		$(top_builddir)/frontend/gtkmm/src/macos/libworkrave-frontend-gtkmm-macos.la
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

dbussources = DBusGUI.cc DBusGUI.hh

if HAVE_DBUS
dbussources += 	GenericDBusApplet.cc
endif

BUILT_SOURCES = DBusGUI.cc DBusGUI.hh

DBusGUI.cc: workrave-gui.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
		  $(top_srcdir)/libs/dbus/data/gio-cc.jinja
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-gui.xml $(top_srcdir)/libs/dbus/data/gio-cc.jinja DBusGUI.cc

DBusGUI.hh:  workrave-gui.xml \
		  $(top_srcdir)/libs/dbus/bin/dbusgen.py \
	    $(top_srcdir)/libs/dbus/data/gio-hh.jinja
	    $(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-gui.xml $(top_srcdir)/libs/dbus/data/gio-hh.jinja DBusGUI.hh

else # HAVE_DBUS

dbussources =

endif # HAVE_DBUS

if HAVE_GSETTINGS

gsettings_ENUM_NAMESPACE = 
gsettings_ENUM_FILES =

gsettings_SCHEMAS = org.workrave.gui.gschema.xml

@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@

endif

iconsdir = $(datadir)/pixmaps/workrave

desktopdir = ${datadir}/applications
desktop_in_files = workrave.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datarootdir)/metainfo
dist_appdata_DATA = workrave.appdata.xml

workrave_SOURCES = \
			AppletControl.cc \
			AppletWindow.cc \
			BreakWindow.cc \
			DailyLimitWindow.cc \
			DataConnector.cc \
			DebugDialog.cc \
			EventButton.cc \
			EventImage.cc \
			EventLabel.cc \
			Frame.cc \
			GUI.cc \
			GUIConfig.cc \
			GtkUtil.cc \
			HeadInfo.cc \
			Hig.cc \
			IconListCellRenderer.cc \
			IconListNotebook.cc \
			MainGtkMenu.cc \
			MainWindow.cc \
			Menus.cc \
			MicroBreakWindow.cc \
			PreferencesDialog.cc \
			PreludeWindow.cc \
			RestBreakWindow.cc \
			Session.cc \
			StatusIcon.cc \
			TimeBar.cc \
			TimeEntry.cc \
			TimerBoxGtkView.cc \
			TimerBoxPreferencePage.cc \
			TimerPreferencesPanel.cc \
			WindowHints.cc \
			main.cc \
			$(extrasources) \
			${dbussources}

nodist_workrave_SOURCES = \
			$(extranodistsources)


workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DWORKRAVE_DATADIR="\"${datadir}\""  \
			-I$(top_srcdir)/frontend/gtkmm/src/ \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@IGE_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) $(includesmacos) \
			$(includesx) $(INDICATOR_CFLAGS) $(indicatorflags) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src

if PLATFORM_OS_MACOS
AM_CFLAGS = 		-xobjective-c
AM_CPPFLAGS =		-xobjective-c++
endif

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I$(top_srcdir)/frontend/gtkmm/src/ @GLIB_CFLAGS@ \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GTK_CFLAGS@ @X11SM_CFLAGS@\
			-D_XOPEN_SOURCE=600 \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)

endif

$(bin_PROGRAMS):	$(top_builddir)/backend/src/libworkrave-backend.la \
			$(top_builddir)/common/src/libworkrave-common.la \
			$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la

workrave_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags} ${WIN32LDFLAGS}

workrave_LDADD =        @WR_LDADD@ @X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
			@GTK_LIBS@ @GNET_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@PULSE_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
			${X11LIBS} ${WIN32LIBS} @GTK_LIBS@ ${MacOSLIBS} ${WIN32CONSOLE} \
			${DIRECTSOUNDLIBS} $(INDICATOR_LIBS)

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h) \
	     workrave.desktop.in org.workrave.Workrave.service.in workrave-gui.xml resource.rc.in


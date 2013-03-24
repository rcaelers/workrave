
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
#

SUBDIRS = 		win32 unix osx

MAINTAINERCLEANFILES = 	Makefile.in \
			org.workrave.Workrave.service \
			workrave.desktop \
			resource.rc

DISTCLEANFILES = 	workrave.desktop org.workrave.gui.gschema.xml

if HAVE_APP_GTKMM

bin_PROGRAMS = 		workrave

extraflags =
extrasources =
extranodistsources = 

## DEFS = 			@DEFS@ 

# Additional distribution sources.
DISTRIBUTION_HOME = 	$(top_srcdir)/frontend/plugin/distribution
sourcesdistribution = 	${DISTRIBUTION_HOME}/gtkmm/src/NetworkLogDialog.cc

# Additional exercises sources.
EXERCISES_HOME =	$(top_srcdir)/frontend/plugin/exercises
sourcesexercises = 	${EXERCISES_HOME}/gtkmm/src/ExercisesDialog.cc \
			${EXERCISES_HOME}/gtkmm/src/ExercisesPanel.cc \
			${EXERCISES_HOME}/common/src/Exercise.cc

# Additional Statistics sources.
STATISTICS_HOME = 	$(top_srcdir)/frontend/plugin/statistics
sourcesstatistics =	${STATISTICS_HOME}/gtkmm/src/StatisticsDialog.cc

if HAVE_PYTHON_CHEETAH
GNOME_APPLET_HOME =	$(top_srcdir)/frontend/applets/gnome2/src

DBusGnomeApplet.cc:	$(GNOME_APPLET_HOME)/gnome-applet.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
		    	$(top_srcdir)/libs/dbus/data/DBus-client-template-freedesktop.cc \
		    	$(top_srcdir)/libs/dbus/data/DBus-client-template-freedesktop.hh
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py --language=C++ -c $(GNOME_APPLET_HOME)/gnome-applet.xml DBUSGnomeApplet

endif

DBusGnomeApplet.hh:	DBusGnomeApplet.cc
GnomeAppletWindow.cc:   DBusGnomeApplet.hh

## Gnome Panel

if HAVE_PANELAPPLET
extrasources +=		GnomeAppletWindow.cc GnomeAppletMenu.cc Plug.cc 

if !HAVE_DBUS_GIO
extrasources += 	DBusGnomeApplet.cc
endif
endif

if HAVE_INDICATOR
extrasources += 	IndicatorAppletMenu.cc
extraflags +=	 	-I$(top_srcdir)/frontend/applets/indicator/include
endif

if PLATFORM_OS_UNIX
if HAVE_GTK3
extrasources +=		X11SystrayAppletWindow.cc gtktrayicon3.c
else
extrasources +=		X11SystrayAppletWindow.cc gtktrayicon.c
endif

extraflags += 		-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	 		-I$(top_srcdir)/frontend/gtkmm/src/unix/
ldadd = 		$(top_builddir)/frontend/gtkmm/src/unix/libworkrave-frontend-gtkmm-unix.la -lX11
endif

if PLATFORM_OS_WIN32
extrasources +=		resource.rc 
extraflags  += 		-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/libs/input-monitor/include \
			-I$(top_srcdir)/frontend/gtkmm/src/win32/ \
			-I$(top_srcdir)/frontend/applets/win32/include \
			-I$(top_srcdir)/libs/hooks/harpoon/include

ldadd =  		$(top_builddir)/libs/hooks/harpoon/src/Release/harpoon.dll \
			$(top_builddir)/frontend/gtkmm/src/win32/libworkrave-frontend-gtkmm-win32.la

WINDRES ?= 		"windres"
.rc.o:
			${WINDRES} -o $@ $<

endif

resource.rc: resource.rc.in
			sed -e 's/@WORKRAVE_VERSION@/$(VERSION)/g' \
	    		    -e 's/@WORKRAVE_RESOURCE_VERSION@/$(RESOURCE_VERSION)/g' \
				$(srcdir)/$@.in > $@

if PLATFORM_OS_OSX
extraflags  += 		-I$(top_srcdir)/frontend/gtkmm/src/osx
ldadd =			$(top_builddir)/frontend/gtkmm/src/osx/libworkrave-frontend-gtkmm-osx.la

AM_CPPFLAGS =		-xobjective-c++
endif

if HAVE_DISTRIBUTION
extrasources +=		$(sourcesdistribution)
endif

if HAVE_EXERCISES
extrasources +=		$(sourcesexercises) 
endif

extrasources +=		$(sourcesstatistics) 

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

extrasources += 	DBusGUI.cc DBusGUI.hh

if HAVE_DBUS_GIO
extrasources += 	GenericDBusApplet.cc
endif

BUILT_SOURCES = DBusGUI.cc DBusGUI.hh

if HAVE_PYTHON_CHEETAH

if HAVE_DBUS_GIO
DBusGUI.cc: 		workrave-gui.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.cc \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.hh
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py --server --gio --language=C++ $(srcdir)/workrave-gui.xml DBusGUI
else
DBusGUI.cc: 		workrave-gui.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
			$(top_srcdir)/libs/dbus/data/DBus-template-freedesktop.cc \
			$(top_srcdir)/libs/dbus/data/DBus-template-freedesktop.hh
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py --server --language=C++ $(srcdir)/workrave-gui.xml DBusGUI
endif

DBusGUI.hh: 		DBusGUI.cc ; test -f $@

endif # HAVE_PYTHON_CHEETAH

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

workrave_SOURCES = 	AppletControl.cc \
			AppletWindow.cc \
			BreakWindow.cc \
			DailyLimitWindow.cc \
			DataConnector.cc \
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
			$(extrasources) 

nodist_workrave_SOURCES = \
			$(extranodistsources)

workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-DWORKRAVE_DATADIR="\"${datadir}\""  \
			-I$(top_srcdir)/frontend/gtkmm/src/ \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/common/include \
			@X_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ @GCONF_CFLAGS@ \
			@DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@  @IGE_CFLAGS@ \
			@INDICATOR_CFLAGS@ ${extraflags} \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/dbus/include \
			-I$(top_srcdir)/libs/utils/include

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I$(top_srcdir)/frontend/gtkmm/src/ \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/common/include \
			@X_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@ @X11SM_CFLAGS@ \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			-I$(top_srcdir)/libs/utils/include \
			$(extraflags)

workrave_LDFLAGS = 	${WIN32LDFLAGS}

workrave_LDADD =	$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/backend/src/libworkrave-backend.la \
			${ldadd} \
			@X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
			@GTK_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@PULSE_LIBS@ @DBUS_LIBS@ @DBUSGLIB_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
			@BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ \
			${X11LIBS} ${WIN32LIBS} ${OSXLIBS} ${WIN32CONSOLE} \
			${DIRECTSOUNDLIBS} $(INDICATOR_LIBS)

$(bin_PROGRAMS):	$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la \
			$(top_builddir)/backend/src/libworkrave-backend.la
endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h) \
	     		workrave.desktop.in org.workrave.Workrave.service.in workrave-gui.xml resource.rc resource.rc.in



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

# Additional exercises sources.
sourcesexercises = 	ExercisesDialog.cc \
			ExercisesPanel.cc

# Additional Statistics sources.
sourcesstatistics =	StatisticsDialog.cc

extrasources +=		$(sourcesexercises) 
extrasources +=		$(sourcesstatistics) 

if PLATFORM_OS_UNIX
extraflags += 		-I$(top_srcdir)/frontend/gtkmm/src/unix/ -DGNOMELOCALEDIR=\""$(datadir)/locale"\"
ldadd = 		$(top_builddir)/frontend/gtkmm/src/unix/libworkrave-frontend-gtkmm-unix.la -lX11
endif

if PLATFORM_OS_OSX
extraflags  += 		-I$(top_srcdir)/frontend/gtkmm/src/osx
ldadd =			$(top_builddir)/frontend/gtkmm/src/osx/libworkrave-frontend-gtkmm-osx.la

AM_CPPFLAGS =		-xobjective-c++
endif

if PLATFORM_OS_WIN32
extrasources +=		resource.rc 
extraflags  += 		-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/libs/input-monitor/include \
			-I$(top_srcdir)/frontend/gtkmm/src/win32/ \
			-I$(top_srcdir)/frontend/applets/win32/include

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

extrasources += 	GenericDBusApplet.cc

BUILT_SOURCES = DBusGUI.cc DBusGUI.hh

if HAVE_PYTHON_CHEETAH

DBusGUI.cc: 		workrave-gui.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.cc \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.hh
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py --server --gio --language=C++ $(srcdir)/workrave-gui.xml DBusGUI

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
			@DBUS_CFLAGS@ @INDICATOR_CFLAGS@ \
			${extraflags} \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/dbus/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/audio/include

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I$(top_srcdir)/frontend/gtkmm/src/ \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/common/include \
			@X_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @DBUS_CFLAGS@ @X11SM_CFLAGS@ \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			-I$(top_srcdir)/libs/utils/include \
			$(extraflags)

workrave_LDFLAGS = 	${WIN32LDFLAGS}

workrave_LDADD =	$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la \
			$(top_builddir)/libs/audio/src/libworkrave-audio.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/backend/src/libworkrave-backend.la \
			${ldadd} \
			@X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
			@GTK_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@PULSE_LIBS@ @DBUS_LIBS@ @GSTREAMER_LIBS@ \
			@BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ \
			${X11LIBS} ${WIN32LIBS} ${OSXLIBS} ${WIN32CONSOLE} \
			${DIRECTSOUNDLIBS} $(INDICATOR_LIBS)

$(bin_PROGRAMS):	$(top_builddir)/frontend/common/src/libworkrave-frontend-common.la \
			$(top_builddir)/backend/src/libworkrave-backend.la
endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h) \
	     		workrave.desktop.in org.workrave.Workrave.service.in workrave-gui.xml resource.rc resource.rc.in


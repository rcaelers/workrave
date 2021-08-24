SUBDIRS = 		win32 unix macos

MAINTAINERCLEANFILES = 	Makefile.in \
			org.workrave.Workrave.service \
			workrave.desktop \
			resource.rc

DISTCLEANFILES = 	workrave.desktop org.workrave.gui.gschema.xml

if HAVE_APP_GTK

bin_PROGRAMS = 		workrave

ldflags =
cflags 	=
sources =   AppletControl.cc \
			AppletWindow.cc \
			BreakWindow.cc \
			DailyLimitWindow.cc \
			DataConnector.cc \
			DebugDialog.cc \
			EventButton.cc \
			EventImage.cc \
			EventLabel.cc \
			Frame.cc \
			Grab.cc \
			GUI.cc \
			GtkUtil.cc \
			HeadInfo.cc \
			Hig.cc \
			IconListCellRenderer.cc \
			IconListNotebook.cc \
			Locale.cc \
			MainGtkMenu.cc \
			MainWindow.cc \
			Menus.cc \
			MicroBreakWindow.cc \
			PreferencesDialog.cc \
			PreludeWindow.cc \
			RestBreakWindow.cc \
			StatisticsDialog.cc \
			StatusIcon.cc \
			Text.cc \
			TimeBar.cc \
			TimeEntry.cc \
			TimerBoxGtkView.cc \
			TimerBoxPreferencePage.cc \
			TimerPreferencesPanel.cc \
			Ui.cc \
			WindowHints.cc \
			main.cc

if PLATFORM_OS_UNIX
cflags 	+=	-I$(srcdir)/unix/
X11LIBS =	-lX11
sources +=	unix/X11SystrayAppletWindow.cc  unix/desktop-window.c
sources +=	unix/gtktrayicon.c unix/UnixGrab.cc
endif

resource.rc: resource.rc.in
	sed -e 's/@WORKRAVE_VERSION@/$(WORKRAVE_VERSION)/g' \
	    -e 's/@WORKRAVE_RESOURCE_VERSION@/$(WORKRAVE_RESOURCE_VERSION)/g' \
		-e 's|@WORKRAVE_TOP_SOURCE_DIR@|$(abs_top_srcdir)|g' \
		$(srcdir)/$@.in > $@


if PLATFORM_OS_WINDOWS
cflags 	+=	-I$(srcdir)/win32/
cflags  +=	-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/ui/apps/gtkmm/src/win32/ \
			-I$(top_srcdir)/ui/applets/win32/include
cflags	+=	-D_WIN32_IE=0x0600 -D_WIN32_WINNT=0x0600 -Wno-attributes -Wno-deprecated-declarations -Wno-deprecated-copy
ldflags +=	-L$(srcdir)/win32 -lworkrave-ui-gtkmm-win32

WINDRES ?= 		"windres"
.rc.o:
	${WINDRES} -o $@ $<

sources +=	resource.rc
WIN32LIBS =	$(top_builddir)/libs/hooks/harpoon/src/Release/harpoon.dll \
			-lshell32 -lole32 -lwinmm -lshlwapi -lwtsapi32
WIN32LDFLAGS = -static-libgcc -static-libstdc++
endif

if PLATFORM_OS_MACOS
cflags  +=	-I$(srcdir)/macos
ldflags +=	-L$(srcdir)/macos -lworkrave-ui-macos
endif

if HAVE_INDICATOR
sources +=	unix/IndicatorAppletMenu.cc
cflags  +=	-I$(top_srcdir)/ui/applets/indicator/include
endif

if HAVE_GSETTINGS

gsettings_ENUM_NAMESPACE =
gsettings_ENUM_FILES =
gsettings_SCHEMAS = org.workrave.gui.gschema.xml

org.workrave.gui.gschema.xml: org.workrave.gui.gschema.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

@GSETTINGS_RULES@

endif

if HAVE_DISTRIBUTION
sources +=	NetworkJoinDialog.cc \
			NetworkLogDialog.cc \
			NetworkPreferencePage.cc
endif

sources +=	ExercisesDialog.cc \
			ExercisesPanel.cc

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

sources += DBusGUI.cc DBusGUI.hh GenericDBusApplet.cc

BUILT_SOURCES = DBusGUI.cc DBusGUI.hh

DBusGUI.cc: workrave-gui.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
		    $(top_srcdir)/libs/dbus/data/gio-cc.jinja
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-gui.xml $(top_srcdir)/libs/dbus/data/gio-cc.jinja DBusGUI.cc

DBusGUI.hh:  workrave-gui.xml \
		  $(top_srcdir)/libs/dbus/bin/dbusgen.py \
	    $(top_srcdir)/libs/dbus/data/gio-hh.jinja
	    $(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-gui.xml $(top_srcdir)/libs/dbus/data/gio-hh.jinja DBusGUI.hh

endif # HAVE_DBUS

iconsdir = $(datadir)/pixmaps/workrave

desktopdir = ${datadir}/applications
desktop_in_files = workrave.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

workrave.desktop: workrave.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

appdatadir = $(datarootdir)/metainfo
dist_appdata_DATA = workrave.appdata.xml

workrave_SOURCES = ${sources}

workrave_CXXFLAGS =	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
					-DWORKRAVE_DATADIR="\"${datadir}\""  \
					-I$(srcdir) \
					@WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@ \
					@X_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ \
					-D_XOPEN_SOURCE=600 \
					@IGE_CFLAGS@ ${DEFS} \
					$(cflags) $(INDICATOR_CFLAGS)

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
					-I$(srcdir) @GLIB_CFLAGS@ \
					@WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@ \
					@X_CFLAGS@ @GTK_CFLAGS@ @X11SM_CFLAGS@\
					-D_XOPEN_SOURCE=600 \
				    $(win32cflags)

if PLATFORM_OS_MACOS
AM_CFLAGS = 		-xobjective-c
AM_CPPFLAGS =		-xobjective-c++
endif

$(bin_PROGRAMS):	$(top_builddir)/libs/audio/src/libworkrave-audio.la \
					$(top_builddir)/libs/config/src//libworkrave-config.la \
					$(top_builddir)/ui/common/src/libworkrave-ui-common.la

workrave_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags} ${WIN32LDFLAGS}

workrave_LDADD =	-lworkrave-ui-common -lworkrave-audio -lworkrave-core -lworkrave-config  \
					-lworkrave-input-monitor -lworkrave-session -lworkrave-dbus \
					-lworkrave-utils @WR_LDADD@ -lm \
					@X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
					@GTK_LIBS@ @X_LIBS@ \
					@PULSE_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
					${X11LIBS} ${WIN32LIBS} @GTK_LIBS@ ${WIN32CONSOLE} \
					${DIRECTSOUNDLIBS} $(INDICATOR_LIBS)
endif

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h) \
	     	workrave.desktop.in org.workrave.Workrave.service.in workrave-gui.xml \
			resource.rc.in locale.inc workrave.manifest org.workrave.gui.gschema.xml.in

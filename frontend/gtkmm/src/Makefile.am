
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
#
# $Id$
#

SUBDIRS = 		gnome_applet kde_applet win32 unix osx
MAINTAINERCLEANFILES = 	Makefile.in ${IDL_A_GENERATED} ${IDL_C_GENERATED} \
			org.workrave.Workrave.service \
			DBusGUI.cc \
			DBusGUI.hh

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
sourcesgnome = 		RemoteControl.cc GnomeAppletWindow.cc GnomeAppletMenu.cc
sourcesgnome_idl = 	${IDL_C_GENERATED} ${IDL_A_GENERATED}

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
sourcesx = 		X11SystrayAppletWindow.cc eggtrayicon.c

# Additional Win32 sources.
sourceswin32 = 		resource.rc StatusIcon.cc

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

Menus.cc:               $(sourcesgnome_idl) 
AppletControl.cc:       $(sourcesgnome_idl)
RemoteControl.cc:       $(sourcesgnome_idl)
GnomeAppletWindow.cc:   $(sourcesgnome_idl)
GUI.cc:			$(sourcesgnome_idl)
else
extrasources +=		$(sourcesnognome)
endif

if HAVE_KDE
extrasources +=		$(sourceskde)
extranodistsources +=	$(sourcesnodistkde)
endif

if PLATFORM_OS_UNIX
extrasources +=		$(sourcesx)
includesx = 		-Iunix/
ldflags = 	        -Lunix/
endif

if PLATFORM_OS_WIN32
extrasources +=		$(sourceswin32)
includeswin32 = 	-I$(top_srcdir)/common/win32/harpoon/include -Iwin32/ \
			-I../../win32/applet/include
ldflags = 		-Lwin32/
win32cflags		= -D_WIN32_IE=0x0500
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
WIN32LIBS = 		-L$(top_srcdir)/common/win32/harpoon/src/Release/ -lharpoon \
			${top_srcdir}/frontend/gtkmm/src/win32/libworkrave-frontend-gtkmm-win32.la \
			-lshell32 -lole32 -lwinmm -lshlwapi
endif

if PLATFORM_OS_UNIX
X11LIBS = 		-lworkrave-frontend-gtkmm-unix -lX11 -lXmu
endif

if PLATFORM_OS_OSX
includesosx  = 		-Iosx
ldflags =	 	-Losx
OSXLIBS	=		${top_srcdir}/frontend/gtkmm/src/osx/libworkrave-frontend-gtkmm-osx.la
extrasources +=		StatusIcon.cc
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
#CLEANFILES = $(BUILT_SOURCES)

dbussources=DBusGUI.cc

DBusGUI.cc: 		workrave-gui.xml
			python ../../../common/bin/dbusgen.py --language=C++ workrave-gui.xml DBusGUI

DBusGUI.cc: 		../../../common/bin/dbusgen.py \
		    	../../../common/bin/DBus-template.cc \
		    	../../../common/bin/DBus-template.hh

DBusGUI.hh: 		DBusGUI.cc

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
			Frame.cc \
			TimerBoxGtkView.cc \
			TimerBoxPreferencePage.cc \
			Menus.cc \
			main.cc \
			MainGtkMenu.cc \
			${dbussources} \
			$(extrasources)


##.PHONY: version.h
##version.h: 
##			echo "#ifndef WORKRAVE_VERSION_H" > version.h
##			echo "#define WORKRAVE_VERSION_H" >> version.h
##			if [ -e ".svn" ] ; then 											\
##				PACKAGE_VERSION=`grep PACKAGE_VERSION ../../../config.h | cut -d' ' -f 3 | sed "s/\"//g"`;		\
##				PACKAGE_SVN_VERION=`svn info . | grep Revision: | sed -e 's/Revision: \([0-9]*\)/\1/'` ;		\
##				echo "#define WORKRAVE_VERSION_STR \"$$PACKAGE_VERSION (build-$$PACKAGE_SVN_VERION)\"" >> version.h;    \
##			else 														\
##				PACKAGE_VERSION=`grep PACKAGE_VERSION ../../../config.h | cut -d' ' -f 3 | sed "s/\"//g"`;		\
##				echo "#define WORKRAVE_VERSION_STR \"$$PACKAGE_VERSION\"" >> version.h;					\
##	        	fi
##			echo "#endif // WORKRAVE_VERSION_H" >> version.h


nodist_workrave_SOURCES = \
			$(extranodistsources)


workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DATADIR="\"${datadir}\""  \
			-I. @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GNOMEMM_CFLAGS@ \
			@GNOME_CFLAGS@ @GTK_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@KDE_INCLUDES@ @QT_INCLUDES@ @DBUS_CFLAGS@  @IGE_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) $(includesosx) \
			$(includesx) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src

if PLATFORM_OS_OSX
INCLUDES =		-xobjective-c++
endif

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I. @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GNOME_CFLAGS@ @GTK_CFLAGS@ @DBUS_CFLAGS@ \
			-D_XOPEN_SOURCE=600 \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)

endif

$(bin_PROGRAMS):	${top_srcdir}/backend/src/libworkrave-backend.la \
			${top_srcdir}/common/src/libworkrave-common.la \
			${top_srcdir}/frontend/common/src/libworkrave-frontend-common.la

workrave_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags}

workrave_LDADD =        @WR_LDADD@ @X_LIBS@ @GNOME_LIBS@ @GNOMEMM_LIBS@ \
			@GTK_LIBS@ @GNET_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			@KDE_LDFLAGS@ @LIB_KDEUI@ @LIB_KDECORE@ \
			@QT_LDFLAGS@ @LIB_QT@ @DBUS_LIBS@ @IGE_LIBS@ \
			${X11LIBS} ${WIN32LIBS} @GTK_LIBS@ ${OSXLIBS} ${WIN32CONSOLE}

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


EXTRA_DIST = $(wildcard *.cc) $(wildcard *.hh) $(wildcard *.c) $(wildcard *.h)

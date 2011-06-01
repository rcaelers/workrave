# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007, 2009, 2011 Rob Caelers & Raymond Penners
#

SUBDIRS = 		
MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_APP_TEXT

bin_PROGRAMS = 		workrave

workrave_SOURCES = 	GUI.cc PreludeWindow.cc BreakWindow.cc TimerBoxTextView.cc MainWindow.cc \
			main.cc

workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DATADIR="\"${datadir}\""  \
			-I. @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ \
			@GTK_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@KDE_INCLUDES@ @QT_INCLUDES@ @DBUS_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) $(includesosx) \
			$(includesx) \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src 

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I. @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
			@X_CFLAGS@ @GTK_CFLAGS@ @DBUS_CFLAGS@  @GSTREAMER_CFLAGS@\
			-D_XOPEN_SOURCE=600 \
			-I${DISTRIBUTION_HOME}/gtkmm/src \
			-I${STATISTICS_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/gtkmm/src \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)

$(bin_PROGRAMS):	${top_srcdir}/backend/src/libworkrave-backend.la \
			${top_srcdir}/common/src/libworkrave-common.la \
			${top_srcdir}/frontend/common/src/libworkrave-frontend-common.la

workrave_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags}

workrave_LDADD =        @WR_LDADD@ @X_LIBS@ \
			@GTK_LIBS@ @GNET_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@KDE_LDFLAGS@ @LIB_KDEUI@ @LIB_KDECORE@ \
			@QT_LDFLAGS@ @LIB_QT@ @DBUS_LIBS@ @GSTREAMER_LIBS@ \
			${X11LIBS} ${WIN32LIBS} ${OSXLIBS} ${WIN32CONSOLE}
endif

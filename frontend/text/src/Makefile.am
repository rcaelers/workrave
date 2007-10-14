# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

SUBDIRS = 		
MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_APP_TEXT

bin_PROGRAMS = 		workrave

if HAVE_X
X11LIBS = 		-lXmu
endif

workrave_SOURCES = 	GUI.cc PreludeWindow.cc BreakWindow.cc TimerBoxTextView.cc MainWindow.cc \
			main.cc

workrave_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DATADIR="\"${datadir}\""  \
			@WR_FRONTEND_CFLAGS@ -I$(top_srcdir)/src \
			 -I. @X_CFLAGS@ @GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 @GNET_CFLAGS@ \
			@KDE_INCLUDES@ @QT_INCLUDES@ @GNOME_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) \
			$(includesx) \
			-I${EXERCISES_HOME}/common/src 

workrave_CFLAGS	= 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@WR_FRONTEND_CFLAGS@ -I$(top_srcdir)/src \
			-I. @X_CFLAGS@ @GTK_CFLAGS@ -D_XOPEN_SOURCE=600 \
			-I${EXERCISES_HOME}/common/src \
			$(win32cflags)


$(bin_PROGRAMS): @WR_FRONTEND_LIBS@

workrave_LDADD =        @WR_FRONTEND_LIBS@ @X_LIBS@ \
			@GTK_LIBS@ @GNET_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			@KDE_LDFLAGS@ @LIB_KDEUI@ @LIB_KDECORE@ @GNOME_LIBS@ \
			@QT_LDFLAGS@ @LIB_QT@ @DBUS_LIBS@ @UUID_LIBS@ \
			${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}
endif

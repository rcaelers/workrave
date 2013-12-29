# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if HAVE_APP_GTKMM
if PLATFORM_OS_UNIX

extrasources =
extraflags =

if HAVE_PANELAPPLET4
extrasources +=		GnomeAppletWindow.cc GnomeAppletMenu.cc Plug.cc 
endif

if HAVE_INDICATOR
extrasources += 	IndicatorAppletMenu.cc
extraflags +=	 	-I$(top_srcdir)/frontend/applets/indicator/include @INDICATOR_CFLAGS@
endif

noinst_LTLIBRARIES = 	libworkrave-frontend-gtkmm-unix.la

libworkrave_frontend_gtkmm_unix_la_SOURCES = \
			X11SystrayAppletWindow.cc \
			desktop-window.c \
			gtktrayicon.c \
			$(extrasources)

libworkrave_frontend_gtkmm_unix_la_CFLAGS = $(extraflags) \
			@X_CFLAGS@  @GTK_CFLAGS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
			-I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/frontend/apps/common/include \
			-I$(top_srcdir)/frontend/apps/gtkmm/src \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/audio/include \
			-I$(top_srcdir)/libs/dbus/include

libworkrave_frontend_gtkmm_unix_la_CXXFLAGS = $(libworkrave_frontend_gtkmm_unix_la_CFLAGS)


endif
endif

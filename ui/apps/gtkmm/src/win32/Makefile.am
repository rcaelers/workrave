# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_APP_GTKMM
if PLATFORM_OS_WINDOWS
noinst_LTLIBRARIES = 	libworkrave-ui-gtkmm-win32.la
libworkrave_ui_gtkmm_win32_la_SOURCES = \
			dll_hell.c \
			DesktopWindow.cc \
			W32AppletWindow.cc \
			W32Compat.cc \
			W32AppletMenu.cc \
			W32TrayMenu.cc \
			W32StatusIcon.cc \
			W32ForceFocus.cc \
			eggsmclient.c \
			eggsmclient-win32.c

libworkrave_ui_gtkmm_win32_la_CFLAGS = \
			-I$(top_srcdir)/ui/common/include \
			-I$(top_srcdir)/ui/applets/win32/include \
			-I$(top_srcdir)/ui/apps/gtkmm/src \
			@GTK_CFLAGS@ @WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@

libworkrave_ui_gtkmm_win32_la_CXXFLAGS = \
			$(libworkrave_ui_gtkmm_win32_la_CFLAGS)

endif
endif

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

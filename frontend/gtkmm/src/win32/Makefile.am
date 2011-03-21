# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if HAVE_APP_GTKMM
if PLATFORM_OS_WIN32
noinst_LTLIBRARIES = 	libworkrave-frontend-gtkmm-win32.la
libworkrave_frontend_gtkmm_win32_la_SOURCES = \
			dll_hell.c \
			DesktopWindow.cc \
			W32AppletWindow.cc \
			W32Compat.cc \
			W32AppletMenu.cc \
			W32TrayMenu.cc \
			W32StatusIcon.cc

libworkrave_frontend_gtkmm_win32_la_CFLAGS = \
			-W  -I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/win32/applet/include \
			-I $(top_srcdir)/frontend/gtkmm/src @GTK_CFLAGS@ \
                        -I $(top_srcdir)/common/include/win32 \
                        -I $(top_srcdir)/common/include


libworkrave_frontend_gtkmm_win32_la_CXXFLAGS = \
			$(libworkrave_frontend_gtkmm_win32_la_CFLAGS)

endif
endif

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
#
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

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
			Win32

libworkrave_frontend_gtkmm_win32_la_CFLAGS = \
			-W  -I../../../common/include \
			-I../../../../backend/include \
			-I../../../win32/applet/include \
			-I .. @GTK_CFLAGS@ \
                        -I ../../../../common/include/win32 \
                        -I ../../../../common/include


libworkrave_frontend_gtkmm_win32_la_CXXFLAGS = \
			$(libworkrave_frontend_gtkmm_win32_la_CFLAGS)

endif
endif

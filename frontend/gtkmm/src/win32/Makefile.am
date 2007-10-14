# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
#
# Time-stamp: <2007-10-08 22:54:20 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if HAVE_APP_GTKMM
if PLATFORM_OS_WIN32
noinst_LIBRARIES = 	libworkrave-frontend-gtkmm-win32.a
libworkrave_frontend_gtkmm_win32_a_SOURCES = \
			dll_hell.c \
			DesktopWindow.cc \
			W32AppletWindow.cc \
			W32Compat.cc

libworkrave_frontend_gtkmm_win32_a_CFLAGS = \
			-W  -I../../../common/include \
			-I../../../../backend/include \
			-I../../../win32/applet/include \
			-I .. @GTK_CFLAGS@ \
                        -I ../../../../common/include/win32 \
                        -I ../../../../common/include


libworkrave_frontend_gtkmm_win32_a_CXXFLAGS = \
			$(libworkrave_frontend_gtkmm_win32_a_CFLAGS)

endif
endif

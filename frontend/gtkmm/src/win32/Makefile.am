# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
#
# Time-stamp: <2004-03-03 13:27:00 pennersr>
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_WIN32
noinst_LIBRARIES = 	libworkrave-frontend-gtkmm-win32.a
libworkrave_frontend_gtkmm_win32_a_SOURCES = \
			dll_hell.c \
			TimerBoxAppletView.cc \
			DesktopWindow.cc \
			W32Compat.cc

libworkrave_frontend_gtkmm_win32_a_CFLAGS = \
			-W  -I../../../common/include \
			-I../../../../backend/include \
			-I../../../win32/applet/include \
			-I .. @GTKMM_CFLAGS@


libworkrave_frontend_gtkmm_win32_a_CXXFLAGS = \
			$(libworkrave_frontend_gtkmm_win32_a_CFLAGS)

endif
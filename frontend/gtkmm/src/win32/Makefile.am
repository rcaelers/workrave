# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
#
# Time-stamp: <2004-02-03 22:14:20 pennersr>
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
			TimerBoxAppletView.cc

libworkrave_frontend_gtkmm_win32_a_CFLAGS = \
			-W  -I../../../common/include \
			-I../../../../backend/include \
			-I../../../win32/applet/include

libworkrave_frontend_gtkmm_win32_a_CXXFLAGS = \
			$(libworkrave_frontend_gtkmm_win32_a_CFLAGS)

endif
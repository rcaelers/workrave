# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# Time-stamp: <2003-10-12 23:39:22 pennersr>
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_WIN32
noinst_LIBRARIES = 	libworkrave-frontend-gtkmm-win32.a
libworkrave_frontend_gtkmm_win32_a_SOURCES = \
			dll_hell.c
libworkrave_frontend_gtkmm_win32_a_CFLAGS = \
			-W 
endif
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
#
# Time-stamp: <2005-11-03 19:45:59 robc>
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if HAVE_APP_GTKMM
if HAVE_UNIX

noinst_LIBRARIES = 	libworkrave-frontend-gtkmm-unix.a
libworkrave_frontend_gtkmm_unix_a_SOURCES = \
			desktop-window.c


libworkrave_frontend_gtkmm_unix_a_CFLAGS = \
			-W  -I../../../common/include \
			-I../../../../backend/include \
			-I .. @GTK_CFLAGS@ \
                        -I ../../../../common/include/unix \
                        -I ../../../../common/include


endif
endif

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-frontend-common.la

SUBDIRS = 		gnome kde win32 x11 osx

libworkrave_frontend_common_la_SOURCES = \
			Text.cc \
			SoundPlayer.cc \
			System.cc \
			TimerBoxControl.cc

ldadd_platform=

if PLATFORM_OS_WIN32
ldadd_platform += 	win32/libworkrave-frontend-common-win32.la
endif
if PLATFORM_OS_OSX
ldadd_platform += 	osx/libworkrave-frontend-common-osx.la
endif
if PLATFORM_OS_UNIX
ldadd_platform += 	x11/libworkrave-frontend-common-x11.la
endif
if HAVE_GNOME
ldadd_platform += 	gnome/libworkrave-frontend-common-gnome.la
endif
if HAVE_KDE
ldadd_platform += 	kde/libworkrave-frontend-common-kde.la
endif

libworkrave_frontend_common_la_LIBADD=${ldadd_platform}

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

libworkrave_frontend_common_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			-W -I. -I../include -Iwin32 -Ignome -Ikde -Iosx\
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
			@GNOME_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/common/win32/harpoon/include

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007, 2008, 2010, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-frontend-common.la

SUBDIRS = 		win32 x11 osx

libworkrave_frontend_common_la_SOURCES = \
			CoreFactory.cc \
			Text.cc \
			SoundPlayer.cc \
			GstSoundPlayer.cc \
			PulseMixer.cc \
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

libworkrave_frontend_common_la_LIBADD=${ldadd_platform}

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

libworkrave_frontend_common_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@X_CFLAGS@ @PULSE_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ @GSTREAMER_CFLAGS@ \
			-I$(top_srcdir)/backend/include  \
			-I$(top_srcdir)/frontend/common/src/ \
			-I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/frontend/common/src/win32 \
			-I$(top_srcdir)/frontend/common/src/osx \
			-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/utils/include

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-audio-win32.la

if HAVE_DSOUND
dsound_sources = 	W32DirectSoundPlayer.cc
endif

libworkrave_audio_win32_la_SOURCES = \
			W32SoundPlayer.cc \
			W32Mixer.cc \
			$(dsound_sources)

libworkrave_audio_win32_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@GLIB_CFLAGS@ \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/common/src \
			-I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/dbus/include

endif

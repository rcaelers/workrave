# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-frontend-common-win32.la

if HAVE_DSOUND
dsound_sources = 	W32DirectSoundPlayer.cc
endif

libworkrave_frontend_common_win32_la_SOURCES = \
			W32SoundPlayer.cc \
			W32Mixer.cc \
			Sound.cc \
			$(dsound_sources)

libworkrave_frontend_common_win32_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-W -I$(top_srcdir)/frontend/common/src \
			-I$(top_srcdir)/frontend/common/include \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
			@GLIB_CFLAGS@

endif
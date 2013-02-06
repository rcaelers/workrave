# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if PLATFORM_OS_OSX

noinst_LTLIBRARIES = 	libworkrave-frontend-common-osx.la

libworkrave_frontend_common_osx_la_SOURCES = \
			Sound.cc \
			OSXSoundPlayer.cc

libworkrave_frontend_common_osx_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/frontend/common/src \
			-I$(top_srcdir)/frontend/common/include \
			@GTK_CFLAGS@ @GLIB_CFLAGS@

endif

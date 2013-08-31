# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007, 2008, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-frontend-apps-common.la

libworkrave_frontend_apps_common_la_SOURCES = \
			Application.cc \
			MenuModelItem.cc \
			MenuModel.cc

libworkrave_frontend_apps_common_la_LIBADD = 
			$(top_builddir)/libs/audio/src/libworkrave-audio.la

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

libworkrave_frontend_apps_common_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@X_CFLAGS@ @PULSE_CFLAGS@ @GTK_CFLAGS@ @GLIB_CFLAGS@ @GSTREAMER_CFLAGS@ \
			-I$(top_srcdir)/backend/include  \
			-I$(top_srcdir)/frontend/apps/common/src/ \
			-I$(top_srcdir)/frontend/apps/common/include \
			-I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/libs/hooks/harpoon/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/dbus/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/audio/include

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if HAVE_KDE

noinst_LTLIBRARIES = 	libworkrave-frontend-common-kde.la

libworkrave_frontend_common_kde_la_SOURCES = \
			KdeSoundPlayer.cc


libworkrave_frontend_common_kde_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 -W -I$(top_srcdir)/common/src/ \
			-I$(top_srcdir)/frontend/common/include \
			@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
			$(KDE_CXXFLAGS) @ALL_INCLUDES@ 

endif
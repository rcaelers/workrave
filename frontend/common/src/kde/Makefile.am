# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if HAVE_KDE

noinst_LIBRARIES = 	libworkrave-frontend-common-kde.a

libworkrave_frontend_common_kde_a_SOURCES = \
			KdeSoundPlayer.cc


libworkrave_frontend_common_kde_a_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 -W -I.. -I../../include \
			@WR_COMMON_CFLAGS@ @WR_BACKEND_INCLUDES@ \
			$(KDE_CXXFLAGS) @all_includes@ 

endif
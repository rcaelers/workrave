# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if PLATFORM_OS_UNIX

noinst_LTLIBRARIES = 	libworkrave-frontend-common-x11.la

libworkrave_frontend_common_x11_la_SOURCES = \
			Sound.cc

libworkrave_frontend_common_x11_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@WR_BACKEND_INCLUDES@ \
			-I$(top_srcdir)/frontend/common/src \
			-I$(top_srcdir)/frontend/common/include

endif
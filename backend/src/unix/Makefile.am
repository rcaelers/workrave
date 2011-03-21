# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-backend-unix.la

if PLATFORM_OS_UNIX
sourcesxinput = 	X11InputMonitor.cc UnixInputMonitorFactory.cc
X11LIBS = 		@X_LIBS@
endif

if HAVE_GCONF
sourcesgconf = 		GConfConfigurator.cc 
endif

if PLATFORM_OS_OSX
sourcesdummy =          dummy.c
endif

libworkrave_backend_unix_la_SOURCES = \
			${sourcesxinput} ${sourcesgconf} ${sourcesdummy}

libworkrave_backend_unix_la_CXXFLAGS = \
			-W -I${top_srcdir}/backend/src -I${top_srcdir}/backend/include @X_CFLAGS@ \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GTK_CFLAGS@ @GCONF_CFLAGS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc)\
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

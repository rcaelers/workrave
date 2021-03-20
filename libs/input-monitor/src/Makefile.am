# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in $(gsettings_SCHEMAS:.xml=.valid)

SUBDIRS = 		unix win32 macos

noinst_LTLIBRARIES = 	libworkrave-backend.la

sources = 		ActivityMonitor.cc \
			InputMonitor.cc \
			InputMonitorFactory.cc


DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I $(top_srcdir)/backend/include

if PLATFORM_OS_WINDOWS
platform_ldadd = 	win32/libworkrave-backend-win32.la
platform_cflags = 	-I$(top_srcdir)/backend/src/win32
endif
if PLATFORM_OS_MACOS
platform_ldadd = 	macos/libworkrave-backend-macos.la
platform_cflags = 	-I$(top_srcdir)/backend/src/macos
endif
if PLATFORM_OS_UNIX
platform_ldadd = 	unix/libworkrave-backend-unix.la
platform_cflags = 	-I$(top_srcdir)/backend/src/unix
endif

libworkrave_backend_la_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesgnet} ${sourcesdistribution} \
			${dbussources} 

libworkrave_backend_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${platform_cflags} @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GDOME_CFLAGS@ @GNET_CFLAGS@ \
			@GCONF_CFLAGS@

libworkrave_backend_la_CXXFLAGS = ${libworkrave_backend_la_CFLAGS}

libworkrave_backend_la_LIBADD=${platform_ldadd}

DISTCLEANFILES = org.workrave.gschema.xml

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
			org.workrave.Workrave.service.in $(gsettings_SCHEMAS:.xml=.xml.in.in)

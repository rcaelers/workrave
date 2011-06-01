# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		win32 unix

noinst_LTLIBRARIES = 	libworkrave-common.la

if HAVE_DBUS
dbussources = 		DBus.cc \
			DBusBinding.cc
endif

if HAVE_LANGUAGE_SELECTION
localesources = 	Locale.cc
endif

libworkrave_common_la_SOURCES =  \
			debug.cc \
			GlibThread.cc \
			Util.cc \
			StringUtil.cc \
			WRID.cc \
			${dbussources} \
			${localesources}

##			glibnls.c \

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-common-win32.la
endif
if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-common-unix.la
endif

libworkrave_common_la_LIBADD = ${ldadd_platform}

INCLUDES = 		-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @DBUS_CFLAGS@ 

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) SConscript locale.inc

Locale.cc:		locale.inc

locale.inc:
			python $(top_srcdir)/common/bin/create_locale.py > locale.inc

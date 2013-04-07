# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

SUBDIRS = 		win32

noinst_LTLIBRARIES = 	libworkrave-utils.la

if HAVE_LANGUAGE_SELECTION
localesources = 	Locale.cc
endif

libworkrave_utils_la_SOURCES = \
			GlibThread.cc \
			StringUtil.cc \
			TimeSource.cc \
			Timer.cc \
			Util.cc \
			debug.cc \
			Platform.cc \
			${localesources}

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-utils-win32.la
endif

libworkrave_utils_la_LIBADD = ${ldadd_platform}

AM_CPPFLAGS = 		@GLIB_CFLAGS@ -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
			-I$(top_srcdir)/libs/utils/include

Locale.cc:		locale.inc

locale.inc:
			$(PYTHON) $(top_srcdir)/libs/utils/bin/create_locale.py > locale.inc

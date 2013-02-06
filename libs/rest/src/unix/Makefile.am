# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_UNIX

noinst_LTLIBRARIES = 	libworkrave-rest-osx.la

libworkrave_rest_osx_la_SOURCES = UnixKeyChain.cc

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/rest/include
endif

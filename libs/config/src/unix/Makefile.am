# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_UNIX
if HAVE_GCONF

sources = 		GConfConfigurator.cc 

endif

noinst_LTLIBRARIES = 	libworkrave-config-unix.la

libworkrave_config_unix_la_SOURCES = ${sources}

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ @GCONF_CFLAGS@ \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/config/src

endif

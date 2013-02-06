# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

if HAVE_GCONF

noinst_LTLIBRARIES = 	libworkrave-config-unix.la

sources = 		GConfConfigurator.cc 

libworkrave_config_unix_la_SOURCES = ${sources}

endif

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ @GCONF_CFLAGS@ \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/config/src

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

noinst_LTLIBRARIES = 	libworkrave-config-unix.la

if HAVE_GCONF
sources = 		GConfConfigurator.cc 
endif

libworkrave_config_unix_la_SOURCES = ${sources}

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ @GCONF_CFLAGS@ \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/config/src \
			-I$(top_srcdir)/common/include


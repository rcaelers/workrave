# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_OSX

noinst_LTLIBRARIES = 	libworkrave-config-osx.la

libworkrave_config_osx_la_SOURCES = OSXConfigurator.cc

AM_CPPFLAGS = 		-xobjective-c++ @GLIB_CFLAGS@ @SIGC_CFLAGS@ \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/config/src \
			-I$(top_srcdir)/common/include

endif
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_WIN32

noinst_LTLIBRARIES = 	libworkrave-config-win32.la

libworkrave_config_win32_la_SOURCES = W32Configurator.cc 

libworkrave_config_win32_la_LIBADD =

AM_CPPFLAGS = 		@GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/config/src \
			-I$(top_srcdir)/common/include

endif
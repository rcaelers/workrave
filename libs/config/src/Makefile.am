# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

SUBDIRS = 		unix win32 osx

noinst_LTLIBRARIES = 	libworkrave-config.la


sources =		Configurator.cc \
			ConfiguratorFactory.cc \
			GlibIniConfigurator.cc \
			GSettingsConfigurator.cc

if HAVE_GDOME
sourcesgdome = 		XMLConfigurator.cc
endif

libworkrave_config_la_SOURCES =  ${sources} ${sourcesgdome}

if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-config-unix.la
cflags_platform =	-I$(top_srcdir)/libs/config/src/unix
endif

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-config-win32.la
cflags_platform =	-I$(top_srcdir)/libs/config/src/win32
endif

if PLATFORM_OS_OSX
ldadd_platform = 	win32/libworkrave-config-osx.la
cflags_platform =	-I$(top_srcdir)/libs/config/src/osx
endif

libworkrave_config_la_LIBADD = \
			${ldadd_platform}

AM_CPPFLAGS = 		@GLIB_CFLAGS@  @SIGC_CFLAGS@ @GDOME_CFLAGS@ \
			${cflags_platform} \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/common/include




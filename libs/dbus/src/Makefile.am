# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

SUBDIRS = 		

if HAVE_DBUS

noinst_LTLIBRARIES = 	libworkrave-dbus.la

dbussources =

if HAVE_DBUS
dbussources += 		DBus-gio.cc \
			DBusBinding-gio.cc
endif

libworkrave_dbus_la_SOURCES = ${dbussources}

endif

AM_CPPFLAGS =		@GLIB_CFLAGS@  @DBUS_CFLAGS@  \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/dbus/include




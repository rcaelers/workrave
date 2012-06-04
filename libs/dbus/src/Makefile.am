# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

SUBDIRS = 		

noinst_LTLIBRARIES = 	libworkrave-dbus.la

dbussources =
if HAVE_DBUS
if HAVE_DBUS_GIO
dbussources += 		DBus-gio.cc \
			DBusBinding-gio.cc
else
dbussources += 		DBus-freedesktop.cc \
			DBusBinding-freedesktop.cc
endif
endif

libworkrave_dbus_la_SOURCES = ${dbussources}

libworkrave_dbus_la_LIBADD = 

AM_CPPFLAGS =		@GLIB_CFLAGS@  @DBUS_CFLAGS@  \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/dbus/include




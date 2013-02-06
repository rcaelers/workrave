# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_UNIX

noinst_LTLIBRARIES = 	libworkrave-network-unix.la

libworkrave_network_unix_la_SOURCES =  \
			NetlinkNetworkInterfaceMonitor.cc \
			UnixNetworkInterfaceMonitor.cc


AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ @GIO_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/network/include \
			-I$(top_srcdir)/libs/network/src

endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)

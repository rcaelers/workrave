# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

noinst_LTLIBRARIES = 	libworkrave-network-unix.la

libworkrave_network_unix_la_SOURCES =  \
			NetlinkNetworkInterfaceMonitor.cc \
			UnixNetworkInterfaceMonitor.cc

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ \
			-I$(top_srcdir)/libs/network/include \
			-I$(top_srcdir)/libs/network/src \
			-I$(top_srcdir)/common/include


# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012 Rob Caelers & Raymond Penners
#

SUBDIRS = 		unix win32

noinst_LTLIBRARIES = 	libworkrave-network.la

libworkrave_network_la_SOURCES =  \
			GIOMulticastSocketServer.cc \
			GIONetworkAddress.cc \
			GIOSocket.cc \
			GIOSocketServer.cc \
			NetworkAddress.cc \
			NetworkInterfaceMonitor.cc \
			Socket.cc \
			SocketServer.cc \
	 		MulticastSocketServer.cc

if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-network-unix.la
cflags_platform =	-I$(top_srcdir)/libs/network/src/unix
endif

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-network-win32.la
cflags_platform =	-I$(top_srcdir)/libs/network/src/win32
endif

libworkrave_network_la_LIBADD = \
			${ldadd_platform}

AM_CPPFLAGS = 		@GLIB_CFLAGS@  @SIGC_CFLAGS@ \
			${cflags_platform} \
			-I$(top_srcdir)/libs/network/include \
			-I$(top_srcdir)/common/include




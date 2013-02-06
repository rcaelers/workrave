# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if !PLATFORM_OS_OSX

noinst_LTLIBRARIES = 	libworkrave-backend-networking.la

if HAVE_DISTRIBUTION
sources	 =		NetworkActivityMonitor.cc \
			NetworkConfigurationManager.cc \
			Networking.cc \
			workrave.pb.cc

SUFFIXES = .proto .pb.cc

%.pb.cc %.pb.h: %.proto
			$(AM_V_GEN)$(PROTOC) $(PROTOCFLAGS) --cpp_out=. -I$(srcdir) $^

BUILT_SOURCES = workrave.pb.cc workrave.pb.h

endif

libworkrave_backend_networking_la_SOURCES = ${sources}

libworkrave_backend_networking_la_CXXFLAGS = \
			@X_CFLAGS@ @GLIB_CFLAGS@ \
			-I${top_srcdir}/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I$(top_srcdir)/libs/config/include \
			-I${top_srcdir}/libs/fog/include \
			-I${top_srcdir}/backend/src \
			-I${top_srcdir}/backend/include

endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

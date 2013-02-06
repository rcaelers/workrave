# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if !PLATFORM_OS_OSX
if HAVE_DISTRIBUTION

noinst_LTLIBRARIES = 	libworkrave-fog.la

sources	 =		Router.cc \
			Marshaller.cc \
			Link.cc \
			Client.cc \
			DirectLinkManager.cc \
			DirectLink.cc \
			Announce.cc \
			fog.pb.cc

SUFFIXES = .proto .pb.cc .pb.h

%.pb.cc %.pb.h: %.proto
			$(AM_V_GEN)$(PROTOC) $(PROTOCFLAGS) --cpp_out=. -I$(srcdir) $^

BUILT_SOURCES = fog.pb.cc fog.pb.h


libworkrave_fog_la_SOURCES = ${sources}

libworkrave_fog_la_CXXFLAGS = \
			@X_CFLAGS@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/fog/include/fog \
			-I${top_srcdir}/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I$(top_srcdir)/libs/fog/include

libworkrave_fog_la_LIBADD = \
			@GLIB_LIBS@ @PROTOBUF_LIBS@

endif
endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

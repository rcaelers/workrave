# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-cloud.la

if HAVE_DISTRIBUTION
sources	 =		Router.cc \
			Marshaller.cc \
			Link.cc \
			DirectLinkManager.cc \
			DirectLink.cc \
			Announce.cc \
			cloud.pb.cc

SUFFIXES = .proto .pb.cc

.proto.pb.cc:
			$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

BUILT_SOURCES = cloud.pb.cc cloud.pb.h

endif

libworkrave_cloud_la_SOURCES = ${sources}

libworkrave_cloud_la_CXXFLAGS = \
			@X_CFLAGS@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I$(top_srcdir)/libs/cloud/include

libworkrave_cloud_la_LIBADD = \
			@GLIB_LIBS@ @PROTOBUF_LIBS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

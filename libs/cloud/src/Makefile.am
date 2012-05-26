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
			-W \
			-I${top_srcdir}/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I$(top_srcdir)/libs/cloud/include \
			@X_CFLAGS@ \
			-D_XOPEN_SOURCE=600 @WR_COMMON_INCLUDES@ @GLIB_CFLAGS@

libworkrave_cloud_la_LIBADD = \
			@GLIB_LIBS@ @PROTOBUF_LIBS@

#			$(top_builddir)/common/src/libworkrave-common.la \
#			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
#			$(top_builddir)/libs/network/src/libworkrave-network.la

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES = 	*.pyc

bin_PROGRAMS = 		cloud

cloud_SOURCES = 	cloud.cc test.pb.cc

SUFFIXES = .proto .pb.cc

.proto.pb.cc:
			$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

BUILT_SOURCES = 	test.pb.cc test.pb.h

cloud_CXXFLAGS = 	@WR_COMMON_INCLUDES@ @GLIB_CFLAGS@ -D_XOPEN_SOURCE=600 \
			-I${top_srcdir}/libs/config/include \
			-I${top_srcdir}/libs/network/include \
			-I${top_srcdir}/libs/cloud/include \
			-I${top_srcdir}/libs/cloud/src

cloud_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags} ${WIN32LDFLAGS}

cloud_LDADD    =        $(top_builddir)/libs/cloud/src/libworkrave-cloud.la \
			$(top_builddir)/libs/network/src/libworkrave-network.la \
			$(top_builddir)/common/src/libworkrave-common.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/config/src/libworkrave-config.la
			@GLIB_LIBS@ @PROTOBUF_LIBS@

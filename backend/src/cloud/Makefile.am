# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009, 2010, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-backend-cloud.la

if HAVE_DISTRIBUTION
sources	 = 		Network.cc \
			NetworkAnnounce.cc \
			networking.pb.cc

SUFFIXES = .proto .pb.cc

.proto.pb.cc:
			$(AM_V_GEN)$(PROTOC) --cpp_out=. -I$(srcdir) $<

BUILT_SOURCES = networking.pb.cc networking.pb.h

endif

libworkrave_backend_cloud_la_SOURCES = ${sources}

libworkrave_backend_cloud_la_CXXFLAGS = \
			-W \
			-I${top_srcdir}/libs/network/include \
			-I${top_srcdir}/backend/src \
			-I${top_srcdir}/backend/include \
			@X_CFLAGS@ \
			-D_XOPEN_SOURCE=600 @WR_COMMON_INCLUDES@ @GLIB_CFLAGS@ @SIGC_CFLAGS@ 

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

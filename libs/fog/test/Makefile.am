# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012, 2013 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES = 	*.pyc

if !PLATFORM_OS_OSX
if HAVE_TESTS

bin_PROGRAMS = 		fog

fog_SOURCES = 	fog.cc test.pb.cc

SUFFIXES = .proto .pb.cc

%.pb.cc %.pb.h: %.proto
			$(AM_V_GEN)$(PROTOC) $(PROTOCFLAGS) --cpp_out=. -I$(srcdir) $^

BUILT_SOURCES = 	test.pb.cc test.pb.h

fog_CXXFLAGS = 	@GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I${top_srcdir}/libs/config/include \
			-I${top_srcdir}/libs/network/include \
			-I${top_srcdir}/libs/fog/include \
			-I${top_srcdir}/libs/fog/include/fog \
			-I${top_srcdir}/libs/fog/src

fog_LDFLAGS = 	-L$(top_builddir)/backend/src/ \
			-L$(top_builddir)/frontend/common/src \
			${ldflags} \
			@WIN32LDFLAGS@

fog_LDADD    =        $(top_builddir)/libs/fog/src/libworkrave-fog.la \
			$(top_builddir)/libs/network/src/libworkrave-network.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/config/src/libworkrave-config.la \
			@GLIB_LIBS@ @PROTOBUF_LIBS@ @BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ @WIN32LIBS@ 

endif
endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.proto) 


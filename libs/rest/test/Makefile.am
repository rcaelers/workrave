# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2010 - 2012 Rob Caelers <robc@krandor.nl>
#


MAINTAINERCLEANFILES = 	*.pyc

if HAVE_TESTS

bin_PROGRAMS = 		rest

rest_SOURCES = 		WorkraveAuth.cc rest_test.cc

rest_CXXFLAGS = 	@GLIB_CFLAGS@  @SECRET_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I${top_srcdir}/libs/rest/include

rest_LDFLAGS =		${ldflags} \
			@WIN32LDFLAGS@ 

rest_LDADD    =        $(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/rest/src/libworkrave-rest.la \
			@GLIB_LIBS@ @PROTOBUF_LIBS@ @BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ @WIN32LIBS@  @SECRET_LIBS@

endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.proto) 


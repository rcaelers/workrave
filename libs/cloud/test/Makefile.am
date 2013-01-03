# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012, 2013 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES = 	*.pyc

if HAVE_TESTS

bin_PROGRAMS = 		cloud

cloud_SOURCES = 	cloud.cc

cloud_CXXFLAGS = 	@GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I${top_srcdir}/libs/config/include \
			-I${top_srcdir}/libs/rest/include \
			-I${top_srcdir}/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/cloud/src

cloud_LDFLAGS = 	-L$(top_builddir)/backend/src/ \
			-L$(top_builddir)/frontend/common/src \
			${ldflags} \
			@WIN32LDFLAGS@

cloud_LDADD    =        $(top_builddir)/libs/cloud/src/libworkrave-cloud.la \
			$(top_builddir)/libs/rest/src/libworkrave-rest.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/config/src/libworkrave-config.la \
			@GLIB_LIBS@ @BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ @WIN32LIBS@ 

endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) 


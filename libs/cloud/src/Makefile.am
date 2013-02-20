# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-cloud.la

sources	 =		WorkraveAuth.cc CloudControl.cc

libworkrave_cloud_la_SOURCES = ${sources}

libworkrave_cloud_la_CXXFLAGS = \
			@X_CFLAGS@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/utils/include \
			-I$(top_srcdir)/libs/rest/include

libworkrave_cloud_la_LIBADD = \
			${ldadd_platform} \
		        $(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/rest/src/libworkrave-rest.la \
			@GLIB_LIBS@ @BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ @WIN32LIBS@ 


EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

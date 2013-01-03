# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-cloud.la

if HAVE_DISTRIBUTION
sources	 =		

endif

libworkrave_cloud_la_SOURCES = ${sources}

libworkrave_cloud_la_CXXFLAGS = \
			@X_CFLAGS@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I$(top_srcdir)/libs/rest/include

libworkrave_cloud_la_LIBADD = \
			@GLIB_LIBS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

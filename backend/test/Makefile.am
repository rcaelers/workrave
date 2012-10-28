# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES = 	*.pyc

if HAVE_TESTS

bin_PROGRAMS = 		cloud cloud_test

cloud_SOURCES = 	cloud.cc Workrave.cc

cloud_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-DWORKRAVE_DATADIR="\"${datadir}\""  \
			-I$(top_srcdir)/backend/include \
			@X_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @SIGC_CFLAGS@ @GCONF_CFLAGS@ \
			@DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@ @IGE_CFLAGS@ \
			-I$(top_srcdir)/backend/src \
			-I$(top_srcdir)/backend/src/networking \
			-I$(top_srcdir)/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I${top_srcdir}/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/cloud/include \
			-I${top_srcdir}/libs/config/include

cloud_LDFLAGS = 	${WIN32LDFLAGS}

cloud_LDADD    =        $(top_builddir)/backend/src/libworkrave-backend.la \
			@BOOST_THREAD_LIB@ @BOOST_SYSTEM_LIB@ \
			@X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@PULSE_LIBS@ @DBUS_LIBS@ @DBUSGLIB_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
			@GTK_LIBS@ @PROTOBUF_LIBS@ @DIRECTSOUNDLIBS@ \
			${X11LIBS} @WIN32LIBS@ ${WIN32CONSOLE}

cloud_test_SOURCES = 	cloud_test.cc Workrave.cc
cloud_test_CXXFLAGS = 	${cloud_CXXFLAGS}
cloud_test_LDFLAGS = 	${cloud_LDFLAGS}
cloud_test_LDADD =	${cloud_LDADD}


$(bin_PROGRAMS):	$(top_builddir)/backend/src/libworkrave-backend.la

endif

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh)

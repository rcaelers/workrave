# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES = 	*.pyc

bin_PROGRAMS = 		cloud

cloud_SOURCES = 	cloud.cc

cloud_CXXFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -W \
			-DWORKRAVE_DATADIR="\"${datadir}\""  \
			@WR_BACKEND_INCLUDES@ \
			@X_CFLAGS@ @GLIB_CFLAGS@ @SIGC_CFLAGS@ \
			@GCONF_CFLAGS@ -D_XOPEN_SOURCE=600 \
			@DBUS_CFLAGS@ @DBUSGLIB_CFLAGS@ @IGE_CFLAGS@ \
			$(includeswin32) $(win32cflags) $(includesinput) $(includesosx) \
			$(includesx) $(eggcflags) -I$(top_srcdir)/backend/src \
			-I$(top_srcdir)/backend/src/networking \
			-I$(top_srcdir)/libs/utils/include \
			-I${top_srcdir}/libs/network/include \
			-I${top_srcdir}/libs/cloud/include/cloud \
			-I${top_srcdir}/libs/cloud/include \
			-I${top_srcdir}/libs/config/include

cloud_LDFLAGS = 	@WR_LDFLAGS@ ${ldflags} ${WIN32LDFLAGS}

cloud_LDADD    =        -lworkrave-backend @X_LIBS@ @X11SM_LIBS@ @GLIB_LIBS@ \
			@X_LIBS@ @GCONF_LIBS@ @GDOME_LIBS@ \
			@PULSE_LIBS@ @DBUS_LIBS@ @DBUSGLIB_LIBS@ @IGE_LIBS@ @GSTREAMER_LIBS@ \
			${X11LIBS} ${WIN32LIBS} ${OSXLIBS} ${WIN32CONSOLE} \
			${DIRECTSOUNDLIBS} @GTK_LIBS@ @PROTOBUF_LIBS@

$(bin_PROGRAMS):	$(top_builddir)/backend/src/libworkrave-backend.la
# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in $(gsettings_SCHEMAS:.xml=.valid) $(wildcard $(srcdir)/*.pb.cc) $(wildcard $(srcdir)/*.pb.h)

SUBDIRS = 		unix win32 osx cloud

noinst_LTLIBRARIES = 	libworkrave-backend.la

sources = 		ActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Core.cc \
			CoreConfig.cc \
			CoreFactory.cc \
			InputMonitor.cc \
			InputMonitorFactory.cc \
			Statistics.cc \
			TimePredFactory.cc \
			Timer.cc \
			DayTimePred.cc \
			Test.cc \
			TimePredFactory.cc


DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I $(top_srcdir)/backend/include


if PLATFORM_OS_WIN32
platform_ldadd = 	win32/libworkrave-backend-win32.la
platform_cflags = 	-I$(top_srcdir)/backend/src/win32
endif
if PLATFORM_OS_OSX
platform_ldadd = 	osx/libworkrave-backend-osx.la
platform_cflags = 	-I$(top_srcdir)/backend/src/osx
endif
if PLATFORM_OS_UNIX
platform_ldadd = 	unix/libworkrave-backend-unix.la
platform_cflags = 	-I$(top_srcdir)/backend/src/unix
endif

# DBus service file
if HAVE_DBUS
dbussources =		DBusWorkrave.cc \
			DBusWorkrave.hh

BUILT_SOURCES = $(dbussources)

if HAVE_PYTHON_CHEETAH

if HAVE_DBUS_GIO
DBusWorkrave.cc:	workrave-service.xml \
			$(top_srcdir)/common/bin/dbusgen.py \
			$(top_srcdir)/common/bin/DBus-template-gio.cc \
			$(top_srcdir)/common/bin/DBus-template-gio.hh
			$(PYTHON) $(top_srcdir)/common/bin/dbusgen.py -s --gio -l C++ $(srcdir)/workrave-service.xml DBusWorkrave
else
DBusWorkrave.cc:	workrave-service.xml \
			$(top_srcdir)/common/bin/dbusgen.py \
			$(top_srcdir)/common/bin/DBus-template-freedesktop.cc \
			$(top_srcdir)/common/bin/DBus-template-freedesktop.hh
			$(PYTHON) $(top_srcdir)/common/bin/dbusgen.py -s -l C++ $(srcdir)/workrave-service.xml DBusWorkrave
endif

DBusWorkrave.hh: 	DBusWorkrave.cc ; test -f $@
endif # HAVE_PYTHON_CHEETAH

else # HAVE_DBUS

dbussources =

endif # HAVE_DBUS

if HAVE_GSETTINGS

gsettings_ENUM_NAMESPACE = org.workrave
gsettings_ENUM_FILES = $(srcdir)/../include/enum.h

gsettings_SCHEMAS = org.workrave.gschema.xml

@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@

endif

network_cflags = 	

libworkrave_backend_la_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesdistribution} \
			${dbussources} 

libworkrave_backend_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${platform_cflags} \
			-I$(top_srcdir)/libs/network/include \
			-I$(top_srcdir)/libs/config/include \
			 @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@@DBUS_CFLAGS@ \
			@PROTOBUF_CFLAGS@ -I$(top_srcdir)/backend/src/cloud

libworkrave_backend_la_CXXFLAGS = ${libworkrave_backend_la_CFLAGS} @SIGC_CFLAGS@

libworkrave_backend_la_LIBADD= \
			${platform_ldadd} \
			$(top_builddir)/libs/config/src/libworkrave-config.la \
			$(top_builddir)/libs/network/src/libworkrave-network.la \
			cloud/libworkrave-backend-cloud.la

DISTCLEANFILES = org.workrave.gschema.xml

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
			org.workrave.Workrave.service.in $(gsettings_SCHEMAS:.xml=.xml.in.in) \
			$(wildcard $(srcdir)/*.proto)

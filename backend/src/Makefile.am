# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in $(gsettings_SCHEMAS:.xml=.valid) $(wildcard $(srcdir)/*.pb.cc) $(wildcard $(srcdir)/*.pb.h)

SUBDIRS =

noinst_LTLIBRARIES = 	libworkrave-backend.la

sources =  		LocalActivityMonitor.cc \
			Break.cc \
			BreakConfig.cc \
			BreakDBus.cc \
			BreakStateModel.cc \
			BreakStatistics.cc \
			BreaksControl.cc \
			Core.cc \
			CoreModes.cc \
			CoreHooks.cc \
			CoreDBus.cc \
			CoreConfig.cc \
			DayTimePred.cc \
			Statistics.cc \
			Timer.cc \
			TimerActivityMonitor.cc \
			ReadingActivityMonitor.cc


DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I $(top_srcdir)/backend/include


# DBus
if HAVE_DBUS
dbussources =		DBusWorkrave.cc \
			DBusWorkrave.hh

BUILT_SOURCES = $(dbussources)

if HAVE_PYTHON_CHEETAH

DBusWorkrave.cc:	workrave-service.xml \
			$(top_srcdir)/libs/dbus/bin/dbusgen.py \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.cc \
			$(top_srcdir)/libs/dbus/data/DBus-template-gio.hh
			$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py -s --backend=${DBUS_BACKEND} -l C++ $(srcdir)/workrave-service.xml DBusWorkrave

DBusWorkrave.hh: 	DBusWorkrave.cc ; test -f $@
endif # HAVE_PYTHON_CHEETAH

else # HAVE_DBUS

dbussources =

endif # HAVE_DBUS

if HAVE_GSETTINGS

gsettings_ENUM_NAMESPACE = org.workrave
gsettings_ENUM_FILES = $(srcdir)/../include/CoreTypes.hh

gsettings_SCHEMAS = org.workrave.gschema.xml

@INTLTOOL_XML_NOMERGE_RULE@

@GSETTINGS_RULES@

endif

if HAVE_DBUS
dbuslib =		$(top_builddir)/libs/dbus/src/libworkrave-dbus.la
endif

libworkrave_backend_la_SOURCES = ${sources} ${dbussources} 

libworkrave_backend_la_CFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			@X_CFLAGS@ @GLIB_CFLAGS@ @DBUS_CFLAGS@ \
			-I$(top_srcdir)/libs/dbus/include \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/config/include \
			-I$(top_srcdir)/libs/input-monitor/include

libworkrave_backend_la_CXXFLAGS = ${libworkrave_backend_la_CFLAGS}

libworkrave_backend_la_LIBADD= \
			$(top_builddir)/libs/input-monitor/src/libworkrave-input-monitor.la \
			$(top_builddir)/libs/utils/src/libworkrave-utils.la \
			$(top_builddir)/libs/config/src/libworkrave-config.la \
			$(dbuslib)

DISTCLEANFILES = 	org.workrave.gschema.xml

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
			$(gsettings_SCHEMAS:.xml=.xml.in.in)

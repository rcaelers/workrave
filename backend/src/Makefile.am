# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in \
			workrave-client-bindings.h \
			workrave-server-bindings.h \
			DBusWorkrave.cc \
			DBusWorkrave.hh

SUBDIRS = 		unix win32 osx

noinst_LTLIBRARIES = 	libworkrave-backend.la

sources = 		ActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Configurator.cc \
			ConfiguratorFactory.cc \
			Core.cc \
			CoreConfig.cc \
			CoreFactory.cc \
			GlibIniConfigurator.cc \
			IdleLogManager.cc \
			InputMonitor.cc \
			InputMonitorFactory.cc \
			Statistics.cc \
			TimePredFactory.cc \
			Timer.cc \
			DayTimePred.cc \
			WeekTimePred.cc \
			TimePredFactory.cc


DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I ../include

if HAVE_GDOME
sourcesgdome = 		XMLConfigurator.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = 	DistributionManager.cc
if HAVE_GNET
sourcesgnet = 		DistributionSocketLink.cc PacketBuffer.cc \
			GNetSocketDriver.cc
endif
endif


if PLATFORM_OS_WIN32
platform_ldadd = 	win32/libworkrave-backend-win32.la
platform_cflags = 	-Iwin32
endif
if PLATFORM_OS_OSX
platform_ldadd = 	osx/libworkrave-backend-osx.la
platform_cflags = 	-Iosx
endif
if PLATFORM_OS_UNIX
platform_ldadd = 	unix/libworkrave-backend-unix.la
platform_cflags = 	-Iunix
endif

# DBus service file
if HAVE_DBUS
dbussources =           DBusWorkrave.cc

DBusWorkrave.cc:	workrave-service.xml
			python ../../common/bin/dbusgen.py -l C++ workrave-service.xml DBusWorkrave

DBusWorkrave.cc: 	../../common/bin/dbusgen.py \
			../../common/bin/DBus-template.cc \
			../../common/bin/DBus-template.hh

DBusWorkrave.hh: 	DBusWorkrave.cc
BreakControl.cc: 	DBusWorkrave.hh

endif

libworkrave_backend_la_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesdistribution} \
			${sourcesgnet} ${dbussources} 

libworkrave_backend_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${platform_cflags} @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GDOME_CFLAGS@ @GNET_CFLAGS@ @DBUS_CFLAGS@ \
			@GCONF_CFLAGS@ @GNOME_CFLAGS@

libworkrave_backend_la_CXXFLAGS = ${libworkrave_backend_la_CFLAGS}

libworkrave_backend_la_LIBADD=${platform_ldadd}


EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) \
			$(wildcard *.h) $(wildcard *.icc) workrave-service.xml \
			org.workrave.Workrave.service.in

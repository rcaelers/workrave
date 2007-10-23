# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in \
			workrave-client-bindings.h \
			workrave-server-bindings.h \
			org.workrave.Workrave.service

SUBDIRS = 		unix win32 osx

noinst_LTLIBRARIES = 	libworkrave-backend.la

sources = 		ActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Configurator.cc \
			ConfiguratorFactory.cc \
			Core.cc \
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
servicedir = $(DBUS_SERVICES_DIR)
service_in_files = org.workrave.Workrave.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|@bindir@|$(bindir)|" $<> $@

BUILT_SOURCES=workrave-server-bindings.h workrave-client-bindings.h

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)

#Rule to generate the binding headers
workrave-server-bindings.h:  workrave-service.xml
	dbus-binding-tool --prefix=workrave --mode=glib-server $<> $@

workrave-client-bindings.h:  workrave-service.xml
	dbus-binding-tool --prefix=workrave --mode=glib-client $<> $@

dbussources=DBus.cc
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

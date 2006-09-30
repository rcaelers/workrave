# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		unix win32 

noinst_LIBRARIES = 	libworkrave-backend.a

sources = 		ActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Configurator.cc \
			Core.cc \
			CoreFactory.cc \
			IdleLogManager.cc \
			GlibIniConfigurator.cc \
			Statistics.cc \
			Timer.cc

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

# DBus service file
if HAVE_DBUS
servicedir = $(DBUS_SERVICES_DIR)
service_in_files = org.workrave.Workrave.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Rule to make the service file with bindir expanded
$(service_DATA): $(service_in_files) Makefile
	@sed -e "s|@bindir@|$(bindir)|" $<> $@

BUILT_SOURCES=workrave-server-bindings.h workrave-client-bindings.h

# We don't want to install this header
#noinst_HEADERS = $(DBUS_HEADER_SOURCES)

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)

#Rule to generate the binding headers
workrave-server-bindings.h:  workrave-service.xml
	dbus-binding-tool --prefix=workrave --mode=glib-server $<> $@

workrave-client-bindings.h:  workrave-service.xml
	dbus-binding-tool --prefix=workrave --mode=glib-client $<> $@

#dbussources=DBus.cc

# HACK
libworkrave_backend_dbus_a_SOURCES=DBus.cc
noinst_LIBRARIES+=libworkrave-backend-dbus.a
libworkrave_backend_a_LIBADD=${libworkrave_backend_dbus_a_OBJECTS}
endif

libworkrave_backend_a_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesdistribution} \
			${sourcesgnet} ${dbussources}

libworkrave_backend_a_CFLAGS = \
			-W -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			@WR_COMMON_CFLAGS@ @WR_BACKEND_CFLAGS@ @GLIB_CFLAGS@ \
			@GDOME_CFLAGS@ @GNET_CFLAGS@ @DBUS_CFLAGS@ \
			@GCONF_CFLAGS@ @GNOMEUI_CFLAGS@

libworkrave_backend_a_CXXFLAGS = \
			-W -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			@WR_COMMON_CFLAGS@ @WR_BACKEND_CFLAGS@ @GLIB_CFLAGS@ \
			@GDOME_CFLAGS@ @GNET_CFLAGS@ @DBUS_CFLAGS@ \
			@GCONF_CFLAGS@ @GNOMEUI_CFLAGS@ 

if HAVE_DBUS
## HACK
libworkrave_backend_dbus_a_CXXFLAGS=-fpermissive ${libworkrave_backend_a_CXXFLAGS}
endif


EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) \
			$(wildcard *.h) $(wildcard *.icc) workrave-service.xml

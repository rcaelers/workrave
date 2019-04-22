# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in $(gsettings_SCHEMAS:.xml=.valid)

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
			GSettingsConfigurator.cc \
			IdleLogManager.cc \
			InputMonitor.cc \
			InputMonitorFactory.cc \
			Statistics.cc \
			TimePredFactory.cc \
			Timer.cc \
			DayTimePred.cc \
			Test.cc \
			TimePredFactory.cc


DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I $(top_srcdir)/backend/include

if HAVE_GDOME
sourcesgdome = 		XMLConfigurator.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = 	DistributionManager.cc \
			DistributionSocketLink.cc \
			PacketBuffer.cc \
			SocketDriver.cc \
			GIOSocketDriver.cc
if HAVE_GNET
sourcesgnet = 		GNetSocketDriver.cc
endif
endif


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

DBusWorkrave.cc:	workrave-service.xml \
									$(top_srcdir)/libs/dbus/bin/dbusgen.py \
							    $(top_srcdir)/libs/dbus/data/gio-cc.jinja
								  $(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-service.xml $(top_srcdir)/libs/dbus/data/gio-cc.jinja DBusWorkrave.cc

DBusWorkrave.hh:	workrave-service.xml \
									$(top_srcdir)/libs/dbus/bin/dbusgen.py \
									$(top_srcdir)/libs/dbus/data/gio-hh.jinja
									$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-service.xml $(top_srcdir)/libs/dbus/data/gio-hh.jinja DBusWorkrave.hh

#DBusWorkrave.hh: 	DBusWorkrave.cc ; test -f $@
#BreakControl.cc: 	DBusWorkrave.hh ; test -f $@

#workrave-client-bindings.h: DBusWorkrave.cc ; test -f $@
#workrave-server-bindings.h: DBusWorkrave.cc ; test -f $@

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

libworkrave_backend_la_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesgnet} ${sourcesdistribution} \
			${dbussources} 

libworkrave_backend_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${platform_cflags} @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GDOME_CFLAGS@ @GNET_CFLAGS@ @DBUS_CFLAGS@ \
			@GCONF_CFLAGS@

libworkrave_backend_la_CXXFLAGS = ${libworkrave_backend_la_CFLAGS}

libworkrave_backend_la_LIBADD=${platform_ldadd}

DISTCLEANFILES = org.workrave.gschema.xml

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
			org.workrave.Workrave.service.in $(gsettings_SCHEMAS:.xml=.xml.in.in)

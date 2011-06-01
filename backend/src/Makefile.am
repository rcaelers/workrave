# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

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

if HAVE_PYTHON_CHEETAH

DBusWorkrave.cc:	workrave-service.xml \
			$(top_srcdir)/common/bin/dbusgen.py \
			$(top_srcdir)/common/bin/DBus-template.cc \
			$(top_srcdir)/common/bin/DBus-template.hh
			python $(top_srcdir)/common/bin/dbusgen.py -l C++ -s $(srcdir)/workrave-service.xml DBusWorkrave

DBusWorkrave.hh: 	DBusWorkrave.cc ; test -f $@
#BreakControl.cc: 	DBusWorkrave.hh ; test -f $@

#workrave-client-bindings.h: DBusWorkrave.cc ; test -f $@
#workrave-server-bindings.h: DBusWorkrave.cc ; test -f $@

endif # HAVE_PYTHON_CHEETAH

else # HAVE_DBUS

dbussources =

endif # HAVE_DBUS

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


EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
			org.workrave.Workrave.service.in SConscript

MAINTAINERCLEANFILES = 	Makefile.in $(gsettings_SCHEMAS:.xml=.valid)

SUBDIRS = win32

noinst_LTLIBRARIES = libworkrave-core.la

sources = 	LocalActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Core.cc \
			CoreConfig.cc \
			CoreFactory.cc \
			Statistics.cc \
			TimePredFactory.cc \
			Timer.cc \
			DayTimePred.cc \
			TimePredFactory.cc

if HAVE_DISTRIBUTION
sources += 	DistributionManager.cc \
			DistributionSocketLink.cc \
			PacketBuffer.cc \
			SocketDriver.cc \
			IdleLogManager.cc \
			GIOSocketDriver.cc
if HAVE_GNET
sources +=	GNetSocketDriver.cc
endif
endif

if PLATFORM_OS_WINDOWS
platform_cflags = 	-I$(top_srcdir)/core/src/win32
endif

if PLATFORM_OS_MACOS
platform_cflags = 	-I$(top_srcdir)/core/src/macos
endif

if PLATFORM_OS_UNIX
platform_cflags = 	-I$(top_srcdir)/core/src/unix
endif

# DBus service file
if HAVE_DBUS
dbussources  =	DBusWorkrave.cc \
				DBusWorkrave.hh

BUILT_SOURCES = $(dbussources)
sources 	+= ${dbussources}

DBusWorkrave.cc:	workrave-service.xml \
						$(top_srcdir)/libs/dbus/bin/dbusgen.py \
					    $(top_srcdir)/libs/dbus/data/gio-cc.jinja
				  $(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-service.xml $(top_srcdir)/libs/dbus/data/gio-cc.jinja DBusWorkrave.cc

DBusWorkrave.hh:	workrave-service.xml \
						$(top_srcdir)/libs/dbus/bin/dbusgen.py \
						$(top_srcdir)/libs/dbus/data/gio-hh.jinja
					$(PYTHON) $(top_srcdir)/libs/dbus/bin/dbusgen.py $(srcdir)/workrave-service.xml $(top_srcdir)/libs/dbus/data/gio-hh.jinja DBusWorkrave.hh

endif # HAVE_DBUS

if HAVE_GSETTINGS

gsettings_SCHEMAS = org.workrave.gschema.xml

org.workrave.gschema.xml: org.workrave.gschema.xml.in
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

@GSETTINGS_RULES@

endif

DEFS = 	@DEFS@ -I. -I $(top_srcdir)/core/include

libworkrave_core_la_SOURCES = ${sources}

libworkrave_core_la_CFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${platform_cflags} @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GDOME_CFLAGS@ @GNET_CFLAGS@

libworkrave_core_la_CXXFLAGS = ${libworkrave_core_la_CFLAGS}

DISTCLEANFILES = org.workrave.gschema.xml

EXTRA_DIST =	$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
				$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) workrave-service.xml \
				org.workrave.Workrave.service.in $(gsettings_SCHEMAS:.xml=.xml.in) \
				dbus-example.py facedetect.py

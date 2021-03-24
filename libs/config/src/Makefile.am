MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = unix win32 macos

noinst_LTLIBRARIES = libworkrave-config.la

cflags =
sources =   Configurator.cc \
			ConfiguratorFactory.cc \
			GlibIniConfigurator.cc \
			GSettingsConfigurator.cc

if HAVE_GDOME
sources += 	XMLConfigurator.cc
endif

if PLATFORM_OS_WINDOWS
sources +=	win32/W32Configurator.cc
cflags += 	-I$(top_srcdir)/libs/config/src/win32
endif

if PLATFORM_OS_MACOS
sources += 	macos/MacOSConfigurator.cc
cflags += 	-I$(top_srcdir)/libs/config/src/macos
endif

if HAVE_GCONF
sources += 	unix/GConfConfigurator.cc
cflags += 	-I$(top_srcdir)/libs/config/src/unix
endif

DEFS = @DEFS@ -I. -I$(top_srcdir)/libs/config/include

libworkrave_config_la_SOURCES = ${sources}

libworkrave_config_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${cflags} @WR_COMMON_INCLUDES@ \
			@GLIB_CFLAGS@ @GDOME_CFLAGS@ \
			@GCONF_CFLAGS@

libworkrave_config_la_CXXFLAGS = ${libworkrave_config_la_CFLAGS}

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			 $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

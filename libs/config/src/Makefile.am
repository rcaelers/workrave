MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = win32 macos

noinst_LTLIBRARIES = libworkrave-config.la

cflags =
sources =   Configurator.cc \
			ConfiguratorFactory.cc \
			IniConfigurator.cc \
			XmlConfigurator.cc \
			SettingCache.cc

if HAVE_GSETTINGS
sources +=	GSettingsConfigurator.cc
endif

if PLATFORM_OS_WINDOWS
sources +=	win32/W32Configurator.cc
cflags += 	-I$(top_srcdir)/libs/config/src/win32
endif

if PLATFORM_OS_MACOS
sources += 	macos/MacOSConfigurator.cc
cflags += 	-I$(top_srcdir)/libs/config/src/macos
endif

DEFS = @DEFS@

libworkrave_config_la_SOURCES = ${sources}

libworkrave_config_la_CFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${cflags} @WR_COMMON_INCLUDES@ \
			-I. -I$(top_srcdir)/libs/config/include -I$(top_srcdir)/libs/config/include/config \
			@GLIB_CFLAGS@

libworkrave_config_la_CXXFLAGS = ${libworkrave_config_la_CFLAGS}

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			 $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

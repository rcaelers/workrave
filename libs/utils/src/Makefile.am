MAINTAINERCLEANFILES =  Makefile.in

SUBDIRS = windows

noinst_LTLIBRARIES = libworkrave-utils.la

cflags=
sources = 	DBusProxy-gio.cc \
			debug.cc \
			AssetPath.cc \
			Diagnostics.cc \
			Platform.cc \
			TimeSource.cc \
			WRID.cc

if PLATFORM_OS_WINDOWS
sources +=	windows/W32ActiveSetup.cc \
	   	  	windows/W32CriticalSection.cc \
    			Platform-windows.cc
cflags += 	-I$(top_srcdir)/libs/utils/src/windows
endif

if PLATFORM_OS_UNIX
sources +=	Platform-unix.cc
cflags += 	@GTK_CFLAGS@
endif

libworkrave_utils_la_SOURCES = ${sources}

libworkrave_utils_la_CXXFLAGS = \
				-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
				-D_XOPEN_SOURCE=600 ${cflags} \
				-I$(top_srcdir)/libs/utils/include/utils \
				@GLIB_CFLAGS@ @WR_COMMON_INCLUDES@

libworkrave_utils_la_CFLAGS = ${libworkrave_utils_la_CXXFLAGS}

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) #$(builddir)/locale.inc

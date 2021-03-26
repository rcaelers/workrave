MAINTAINERCLEANFILES =  Makefile.in

SUBDIRS = win32

noinst_LTLIBRARIES = libworkrave-utils.la

cflags=
sources = 	DBusProxy-gio.cc \
			debug.cc \
			Diagnostics.cc \
			GlibThread.cc \
			StringUtil.cc \
			Util.cc \
			WRID.cc

if PLATFORM_OS_WINDOWS
sources +=	win32/W32ActiveSetup.cc \
	   	  	win32/W32CriticalSection.cc \
		  	win32/crashlog.c
cflags += 	-I$(top_srcdir)/libs/session/src/win32
endif

libworkrave_utils_la_SOURCES = ${sources}

AM_CPPFLAGS = 	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
				-D_XOPEN_SOURCE=600 ${cflags} \
				@GLIB_CFLAGS@ @WR_COMMON_INCLUDES@

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc) #$(builddir)/locale.inc

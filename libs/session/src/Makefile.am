MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = libworkrave-session.la

SUBDIRS =	windows

cflags =
sources = 	System.cc

if PLATFORM_OS_UNIX
sources += 	ScreenLockCommandline.cc

if HAVE_DBUS
sources +=	ScreenLockDBus.cc	\
			SystemStateChangeConsolekit.cc \
			SystemStateChangeUPower.cc \
			SystemStateChangeLogind.cc
endif
endif

if PLATFORM_OS_WINDOWS
sources += 	windows/W32LockScreen.cc \
			windows/W32Shutdown.cc
cflags += 	-I$(top_srcdir)/libs/session/src/windows
endif

libworkrave_session_la_SOURCES = ${sources}

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

DEFS = @DEFS@ -I. -I$(top_srcdir)/libs/session/include

libworkrave_session_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ ${cflags} \
			@WR_COMMON_INCLUDES@ @GTK_CFLAGS@ @GLIB_CFLAGS@ \
			-I

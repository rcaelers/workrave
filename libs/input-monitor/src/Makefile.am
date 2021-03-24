MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = unix win32 macos

noinst_LTLIBRARIES = libworkrave-input-monitor.la

cflags =
sources = 	InputMonitor.cc \
			InputMonitorFactory.cc

if PLATFORM_OS_WINDOWS
sources += 	win32/Harpoon.cc \
			win32/W32AlternateMonitor.cc \
			win32/W32InputMonitor.cc \
			win32/W32InputMonitorFactory.cc \
			win32/W32LowLevelMonitor.cc
cflags += 	-I$(top_srcdir)/libs/input-monitor/src/win32 -I$(top_srcdir)/libs/hooks/harpoon/include -I$(top_srcdir)/libs/hooks/harpoonHelper/include
endif

if PLATFORM_OS_MACOS
sources += 	macos/MacOSInputMonitor.cc \
			macos/MacOSInputMonitorFactory.cc
cflags += 	-I$(top_srcdir)/libs/input-monitor/src/macos
endif

if PLATFORM_OS_UNIX
sources += 	unix/MutterInputMonitor.cc \
			unix/RecordInputMonitor.cc \
			unix/UnixInputMonitorFactory.cc \
			unix/X11InputMonitor.cc \
			unix/XScreenSaverMonitor.cc
cflags += 	-I$(top_srcdir)/libs/input-monitor/src/unix @GTK_CFLAGS@
endif

libworkrave_input_monitor_la_SOURCES = ${sources}

libworkrave_input_monitor_la_CFLAGS = \
			-W -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			${cflags} @WR_COMMON_INCLUDES@ -I$(top_srcdir)/libs/input-monitor/src \
			@GLIB_CFLAGS@

libworkrave_input_monitor_la_CXXFLAGS = ${libworkrave_input_monitor_la_CFLAGS}

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.rc) $(wildcard $(srcdir)/*.hh) \
			 $(wildcard $(srcdir)/*.h)

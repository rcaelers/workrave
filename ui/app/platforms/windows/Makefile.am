# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

if HAVE_APP_GTK
if PLATFORM_OS_WINDOWS
noinst_LTLIBRARIES = 	libworkrave-ui-gtkmm-windows.la
libworkrave_ui_gtkmm_windows_la_SOURCES = \
			DesktopWindow.cc \
			WindowsAppletWindow.cc \
			WindowsCompat.cc \
			WindowsStatusIcon.cc \
			WindowsForceFocus.cc \
			WindowsHarpoonLocker.cc \
			WindowsLocker.cc

libworkrave_ui_gtkmm_windows_la_CFLAGS = \
			-I$(top_srcdir)/ui/common/include \
			-I$(top_srcdir)/ui/applets/windows/include \
			-I$(top_srcdir)/ui/app/toolkits/gtkmm/src \
			@GTK_CFLAGS@ @WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@

libworkrave_ui_gtkmm_windows_la_CXXFLAGS = \
			$(libworkrave_ui_gtkmm_windows_la_CFLAGS)

endif
endif

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

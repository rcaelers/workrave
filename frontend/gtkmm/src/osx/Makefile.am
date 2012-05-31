# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2011, 2012 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if PLATFORM_OS_OSX

##AM_CPPFLAGS=-xobjective-c++

noinst_LTLIBRARIES = 	libworkrave-frontend-gtkmm-osx.la
libworkrave_frontend_gtkmm_osx_la_SOURCES = \
			OSXUtil.cc \
			AppController.m \
			OSXGtkMenu.cc \
			OSXAppletWindow.cc \
			OSXStatusBarView.m \
			OSXTimeBar.m

libworkrave_frontend_gtkmm_osx_la_OBJCFLAGS = \
			@X_CFLAGS@ -xobjective-c++ \
			-W  -I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/backend/include \
			-I$(top_srcdir)/frontend/gtkmm/src @GTK_CFLAGS@

libworkrave_frontend_gtkmm_osx_la_CFLAGS = \
			@X_CFLAGS@ \
			-W  -I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/backend/include \
			-I $(top_srcdir)/frontend/gtkmm/src @GTK_CFLAGS@

libworkrave_frontend_gtkmm_osx_la_CXXFLAGS = \
			@X_CFLAGS@ -xobjective-c++ \
			-W  -I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/backend/include \
			-I $(top_srcdir)/frontend/gtkmm/src @GTK_CFLAGS@ @IGE_CFLAGS@


endif

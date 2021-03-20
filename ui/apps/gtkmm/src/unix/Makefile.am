# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = 		$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

if HAVE_APP_GTKMM
if PLATFORM_OS_UNIX

noinst_LIBRARIES = 	libworkrave-frontend-gtkmm-unix.a
libworkrave_frontend_gtkmm_unix_a_SOURCES = \
			desktop-window.c


libworkrave_frontend_gtkmm_unix_a_CFLAGS = \
			@X_CFLAGS@ \
			-W  -I$(top_srcdir)/frontend/common/include \
			-I$(top_srcdir)/backend/include \
			-I $(top_srcdir)/frontend/gtkmm/src @GTK_CFLAGS@ \
                        -I $(top_srcdir)/common/include/unix \
                        -I $(top_srcdir)/common/include


endif
endif

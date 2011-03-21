# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
#

MAINTAINERCLEANFILES = 	Makefile.in

if PLATFORM_OS_UNIX

noinst_LTLIBRARIES = 	libworkrave-common-unix.la

libworkrave_common_unix_la_SOURCES =

libworkrave_common_unix_la_LIBADD =

libworkrave_common_unix_la_CXXFLAGS = @X_CFLAGS@

endif

INCLUDES = 		-W -D_XOPEN_SOURCE=600 -I. -I../../include \
			-I../../include/unix 

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) \
			$(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

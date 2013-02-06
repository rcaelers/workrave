# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2010 - 2013 Rob Caelers <robc@krandor.nl>
#

SUBDIRS = 		unix win32 osx

MAINTAINERCLEANFILES = 	Makefile.in

lib_LTLIBRARIES = 	libworkrave-rest.la

libworkrave_rest_la_SOURCES = \
			HttpSession.cc \
			HttpReply.cc \
			HttpRequest.cc \
			HttpServer.cc \
			HttpUtils.cc \
			HttpOperation.cc \
			HttpStreamOperation.cc \
			OAuth1.cc \
			OAuth1Filter.cc \
			OAuth2.cc \
			OAuth2Filter.cc \
			Uri.cc \
			jsoncpp.cpp

if PLATFORM_OS_UNIX
ldadd_platform = 	unix/libworkrave-rest-unix.la
cflags_platform =	-I$(top_srcdir)/libs/rest/src/unix
endif

if PLATFORM_OS_WIN32
ldadd_platform = 	win32/libworkrave-rest-win32.la
cflags_platform =	-I$(top_srcdir)/libs/rest/src/win32
endif

if PLATFORM_OS_OSX
ldadd_platform = 	osx/libworkrave-rest-osx.la
cflags_platform =	-I$(top_srcdir)/libs/rest/src/osx -xobjective-c++
endif

libworkrave_rest_la_CXXFLAGS = \
			${cflags_platform} \
			@GLIB_CFLAGS@ @SOUP_CFLAGS@ @LIBGCRYPT_CFLAGS@ @SECRET_CFLAGS@

libworkrave_rest_la_LIBADD =  \
			${ldadd_platform} \
			@GLIB_LIBS@ @SOUP_LIBS@ @LIBGCRYPT_LIBS@ @SECRET_LIBS@ 

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @GIO_CFLAGS@ \
			${cflags_platform} \
			-I$(top_srcdir)/libs/rest/include
			-I$(top_srcdir)/libs/utils/include

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h)

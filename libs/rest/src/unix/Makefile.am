# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2012, 2013 Rob Caelers & Raymond Penners
#

if PLATFORM_OS_UNIX

noinst_LTLIBRARIES = 	libworkrave-rest-unix.la

libworkrave_rest_unix_la_SOURCES = UnixKeyChain.cc

libworkrave_rest_unix_la_CXXFLAGS = \
			${cflags_platform} \
			@GLIB_CFLAGS@ @SOUP_CFLAGS@ @LIBGCRYPT_CFLAGS@ @SECRET_CFLAGS@

libworkrave_rest_unix_la_LIBADD =  \
			${ldadd_platform} \
			@GLIB_LIBS@ @SOUP_LIBS@ @LIBGCRYPT_LIBS@ @SECRET_LIBS@ 

AM_CPPFLAGS = 		@GLIB_CFLAGS@ @SIGC_CFLAGS@ \
			-I$(top_srcdir)/libs/utils/include \
			-I$(top_srcdir)/libs/rest/include
endif

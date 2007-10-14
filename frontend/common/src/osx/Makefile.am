# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id: Makefile.am 1092 2006-10-01 22:03:56Z dotsphinx $
#

MAINTAINERCLEANFILES =	Makefile.in

EXTRA_DIST = 		$(wildcard *.hh) $(wildcard *.cc) \
			$(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_OS_OSX

noinst_LIBRARIES = 	libworkrave-frontend-common-osx.a

libworkrave_frontend_common_osx_a_SOURCES = \
			Sound.cc \
			OSXSoundPlayer.cc

libworkrave_frontend_common_osx_a_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			-W -I.. -I../../include @WR_COMMON_CFLAGS@ \
			@WR_BACKEND_INCLUDES@ \
			@GTK_CFLAGS@ @GLIB_CFLAGS@ 

endif
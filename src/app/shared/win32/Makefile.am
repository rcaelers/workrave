# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST 		= $(wildcard *.hh) $(wildcard *.cc) $(wildcard *.c) $(wildcard *.h)  $(wildcard *.icc)

if PLATFORM_WIN32

noinst_LIBRARIES 	= libappwin32.a

libappwin32_a_SOURCES 	= Win32SoundPlayer.cc


libappwin32_a_CXXFLAGS 	= -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" -D_XOPEN_SOURCE=600 \
			  -W -I.. -I../intf @SHARED_CFLAGS@ @CORE_INTF@  

endif
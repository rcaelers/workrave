# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		unix win32 

noinst_LIBRARIES = 	libworkrave-backend.a

sources = 		ActivityMonitor.cc \
			Break.cc \
			BreakControl.cc \
			Configurator.cc \
			Core.cc \
			CoreFactory.cc \
			IdleLogManager.cc \
			Statistics.cc \
			Timer.cc

DEFS = 			@DEFS@ -I$(top_srcdir)/intl -I. -I ../include

if HAVE_GDOME
sourcesgdome = 		XMLConfigurator.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = 	DistributionManager.cc
if HAVE_GNET
sourcesgnet = 		DistributionSocketLink.cc PacketBuffer.cc \
			GNetSocketDriver.cc
endif
endif

libworkrave_backend_a_SOURCES = \
			${sources} ${sourcesgdome} ${sourcesdistribution} \
			${sourcesgnet}

libworkrave_backend_a_CXXFLAGS = \
			-W -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 \
			@WR_COMMON_CFLAGS@ @WR_BACKEND_CFLAGS@ @GLIB_CFLAGS@ \
			@GDOME_CFLAGS@ @GNET_CFLAGS@ \
			@GCONF_CFLAGS@ @GNOMEUI_CFLAGS@

EXTRA_DIST = 		$(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) \
			$(wildcard *.h) $(wildcard *.icc)

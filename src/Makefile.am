# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = workrave

sources = \
      ActivityMonitor.cc \
      Configurator.cc \
      Control.cc \
      ControlInterface.cc \
      DayTimePred.cc \
      GUIFactory.cc \
      TimePredFactory.cc \
      Timer.cc \
      WeekTimePred.cc \
      main.cc

if HAVE_X
sourcesxinput = X11InputMonitor.cc 
X11LIBS    = -lXmu
endif

DEFS = @DEFS@ -DGNOMELOCALEDIR=\""$(datadir)/locale"\" -I$(top_srcdir)/intl


if PLATFORM_WIN32
WINDRES ?= "windres"
.rc.o:
	${WINDRES} -o $@ $<

includesinput = -I$(top_srcdir)/arch/win32/harpoon/
sourceswin32input = Win32InputMonitor.cc resource.rc
WIN32LIBS = $(top_srcdir)/arch/win32/harpoon/Release/harpoon.lib \
	-lshell32 -lole32 -lwinmm
endif

if HAVE_GCONF
sourcesgconf = GConfConfigurator.cc 
endif

if HAVE_GDOME
sourcesgdome = XMLConfigurator.cc
endif

if HAVE_REGISTRY
sourcesregistry = Win32Configurator.cc
endif

if HAVE_DISTRIBUTION
sourcesdistribution = DistributionManager.cc
if HAVE_GNET
sourcesgnet = DistributionSocketLink.cc PacketBuffer.cc GNetSocketDriver.cc
endif
endif

workrave_SOURCES 	= ${sources} ${sourcesxinput} ${sourceswin32input} \
		   	  ${sourcesgdome} ${sourcesgconf} ${sourcesregistry} \
			  ${sourcesdistribution} ${sourcesgnet}

workrave_CXXFLAGS 	= -W -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/gui $(includesinput) \
	 	   	  -D_XOPEN_SOURCE=600 @GNOMEUI_CFLAGS@ @GTK_CFLAGS@ @GDOME_CFLAGS@ @GCONF_CFLAGS@ @GNET_CFLAGS@

workrave_LDADD 		=  @GUI_LIBS@ @ARCH_LIBS@ @GNOMEUI_LIBS@ @GNOMEUIMM_LIBS@ @GTK_LIBS@ \
			   @GTKMM_LIBS@ @GNET_LIBS@ \
		            @PTHREAD_LIBS@ @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ \
			   ${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}

#CXXLD=ccmalloc g++

$(bin_PROGRAMS): @ARCH_LIBS@ @GUI_LIBS@

EXTRA_DIST 		= $(wildcard *.cc) $(wildcard *.rc) $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

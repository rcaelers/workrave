# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = workrave

sources = \
      ActivityMonitor.cc \
      ActivityStateMonitor.cc \
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

if PLATFORM_WIN32
includesinput = -I$(top_srcdir)/arch/win32/harpoon/
sourceswin32input = Win32InputMonitor.cc 
WIN32LIBS = $(top_srcdir)/arch/win32/harpoon/Release/harpoon.lib \
	-lshell32 -lole32
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

workrave_SOURCES 	= ${sources} ${sourcesxinput} ${sourceswin32input} \
		   	  ${sourcesgdome} ${sourcesgconf} ${sourcesregistry}

EXTRA_DIST 		=  $(wildcard *.hh) $(wildcard *.h) $(wildcard *.icc)

workrave_CXXFLAGS 	= -W -DDATADIR="\"${pkgdatadir}\"" @ARCH_CFLAGS@ -I$(top_srcdir)/gui $(includesinput) \
	 	   	  -D_XOPEN_SOURCE=600 @GTK_CFLAGS@ @GDOME_CFLAGS@ @GCONF_CFLAGS@

workrave_LDADD 		=  @GUI_LIBS@ @ARCH_LIBS@ @gnomeui_LIBS@ @GTK_LIBS@ @GTKMM_LIBS@ @PTHREAD_LIBS@ \
			   @X_LIBS@ @GCONF_LIBS@ @GDOME_LIB@ ${X11LIBS} ${WIN32LIBS} ${WIN32CONSOLE}

#CXXLD=ccmalloc g++

$(bin_PROGRAMS): @ARCH_LIBS@ @GUI_LIBS@

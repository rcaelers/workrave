INCLUDES= $(all_includes)
METASOURCES = AUTO

if HAVE_KDE

lib_LTLIBRARIES = libkworkraveapplet.la

libkworkraveapplet_la_SOURCES = kworkraveapplet.cpp kworkraveapplet.skel kworkraveapplet.stub
libkworkraveapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkworkraveapplet_la_LIBADD = $(LIB_KDEUI) 

kworkraveapplet_DATA = kworkraveapplet.desktop
kworkraveappletdir = $(kde_datadir)/kicker/applets

endif

EXTRA_DIST 	= $(wildcard *.desktop)				\
		  $(wildcard *.h) $(wildcard *.cpp)		\
		  $(wildcard *.c) $(wildcard *.h)

INCLUDES= $(all_includes)
METASOURCES = AUTO

if HAVE_KDE

lib_LTLIBRARIES = libkworkraveapplet.la

libkworkraveapplet_la_SOURCES = kworkraveapplet.moc.cpp kworkraveapplet_skel.cpp kworkraveapplet_stub.cpp
libkworkraveapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkworkraveapplet_la_LIBADD = $(LIB_KDEUI)
libkworkraveapplet_la_CXXFLAGS = $(KDE_CXXFLAGS)

kworkraveapplet_DATA = kworkraveapplet.desktop
kworkraveappletdir = $(kde_datadir)/kicker/applets

kworkraveapplet.kidl: $(srcdir)/kworkraveapplet.h $(DCOP_DEPENDENCIES)
	$(DCOPIDL) $(srcdir)/kworkraveapplet.h > kworkraveapplet.kidl || ( rm -f kworkraveapplet.kidl ; false )
kworkraveapplet_skel.cpp: kworkraveapplet.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-stub kworkraveapplet.kidl
kworkraveapplet_stub.h: kworkraveapplet.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-skel kworkraveapplet.kidl
kworkraveapplet.moc.cpp: $(srcdir)/kworkraveapplet.h
	$(MOC) $(srcdir)/kworkraveapplet.h -o kworkraveapplet.moc.cpp

endif

EXTRA_DIST 	= $(wildcard *.desktop)				\
		  $(wildcard *.h) $(wildcard *.cpp)		\
		  $(wildcard *.c) $(wildcard *.h)

MAINTAINERCLEANFILES = Makefile Makefile.in

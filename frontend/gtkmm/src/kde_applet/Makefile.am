if HAVE_KDE

lib_LTLIBRARIES = libkworkraveapplet.la

libkworkraveapplet_la_SOURCES = kworkraveapplet.cpp
libkworkraveapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkworkraveapplet_la_LIBADD = $(LIB_KDEUI) @all_libraries@
libkworkraveapplet_la_CXXFLAGS = $(KDE_CXXFLAGS) @all_includes@
nodist_libkworkraveapplet_la_SOURCES = kworkraveapplet.moc.cpp kworkraveapplet_skel.cpp kworkraveapplet_stub.cpp

kworkraveapplet_DATA = kworkraveapplet.desktop
kworkraveappletdir = $(kde_datadir)/kicker/applets

kworkraveapplet.kidl: $(srcdir)/kworkraveapplet.h $(DCOP_DEPENDENCIES)
	$(DCOPIDL) $(srcdir)/kworkraveapplet.h > kworkraveapplet.kidl || ( rm -f kworkraveapplet.kidl ; false )
kworkraveapplet_skel.cpp: kworkraveapplet.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-stub kworkraveapplet.kidl
kworkraveapplet_stub.cpp: kworkraveapplet_stub.h 
kworkraveapplet_stub.h: kworkraveapplet.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-skel kworkraveapplet.kidl
kworkraveapplet.moc.cpp: $(srcdir)/kworkraveapplet.h
	$(MOC) $(srcdir)/kworkraveapplet.h -o kworkraveapplet.moc.cpp

endif

EXTRA_DIST 	= $(wildcard *.desktop)

MAINTAINERCLEANFILES = Makefile Makefile.in

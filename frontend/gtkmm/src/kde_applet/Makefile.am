if HAVE_KDE

lib_LTLIBRARIES = libkworkraveapplet.la

libkworkraveapplet_la_SOURCES = kworkraveapplet.cpp 
libkworkraveapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkworkraveapplet_la_LIBADD = $(LIB_KDEUI) @all_libraries@
libkworkraveapplet_la_CXXFLAGS = $(KDE_CXXFLAGS) @all_includes@
nodist_libkworkraveapplet_la_SOURCES = kworkraveapplet.moc.cpp kworkraveapplet_skel.cpp kworkraveapplet_stub.cpp kworkravecontroliface_skel.cpp kworkravecontroliface_stub.cpp 

kworkraveapplet.cpp: kworkravecontroliface_stub.h

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

kworkravecontroliface.kidl: $(srcdir)/kworkravecontroliface.h $(DCOP_DEPENDENCIES)
	$(DCOPIDL) $(srcdir)/kworkravecontroliface.h > kworkravecontroliface.kidl || ( rm -f kworkravecontroliface.kidl ; false )
kworkravecontroliface_skel.cpp: kworkravecontroliface.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-stub kworkravecontroliface.kidl
kworkravecontroliface_stub.cpp: kworkravecontroliface_stub.h 
kworkravecontroliface_stub.h: kworkravecontroliface.kidl
	$(DCOPIDL2CPP) --c++-suffix cpp --no-signals --no-skel kworkravecontroliface.kidl
kworkravecontroliface.moc.cpp: $(srcdir)/kworkravecontroliface.h
	$(MOC) $(srcdir)/kworkravecontroliface.h -o kworkravecontroliface.moc.cpp

##extrasources 	 +=	kdeworkravecontrol.moc.cpp kdeworkravecontrol_skel.cpp kdeworkravecontrol_stub.cpp

endif

EXTRA_DIST 	= $(wildcard $(srcdir)/*.desktop) kworkraveapplet.h kworkravecontroliface.h

MAINTAINERCLEANFILES = Makefile Makefile.in kworkravecontroliface.kidl kworkravecontroliface_skel.cpp kworkravecontroliface_stub.cpp kworkravecontroliface_stub.h

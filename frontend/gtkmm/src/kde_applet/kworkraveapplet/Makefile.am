INCLUDES= $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkworkraveapplet.la

libkworkraveapplet_la_SOURCES = kworkraveapplet.cpp kworkraveapplet.skel kworkraveapplet.stub
libkworkraveapplet_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkworkraveapplet_la_LIBADD = $(LIB_KDEUI) 

# kworkraveapplet.moc.cc: $(srcdir)/kworkraveapplet.hh $(srcdir)/kworkraveapplet.cc
# 	$(MOC) $(srcdir)/kworkraveapplet.hh -o kworkraveapplet.moc.cc

kworkraveapplet_DATA = kworkraveapplet.desktop
kworkraveappletdir = $(kde_datadir)/kicker/applets

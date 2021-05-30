MAINTAINERCLEANFILES =  Makefile.in


noinst_LTLIBRARIES = libworkrave-dbus.la

libworkrave_dbus_la_SOURCES =  DBusFactory.cc \
                               DBusException.cc \
                               DBusDummy.cc \
                               DBusGio.cc \
                               DBusGeneric.cc

if HAVE_DBUS_GIO
libworkrave_dbus_la_SOURCES += DBusBindingGio.cc
endif

AM_CPPFLAGS = -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" -DWORKRAVE_DATADIR="\"${datadir}\"" \
              -D_XOPEN_SOURCE=600 @WR_COMMON_INCLUDES@ @GLIB_CFLAGS@

EXTRA_DIST = $(wildcard $(srcdir)/*.cc) $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h) $(wildcard $(srcdir)/*.icc)

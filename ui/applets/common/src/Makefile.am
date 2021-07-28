-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --add-include-path=$(srcdir)/../include --warn-all \
		--include GObject-2.0 \
                --include Gtk-3.0 \
		--include cairo-1.0
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(srcdir)/../include

if HAVE_INTROSPECTION

workravelibdir = $(libdir)
workravelib_LTLIBRARIES = libworkrave-private-1.0.la
libworkrave_private_1_0_la_SOURCES = \
	timebar.c \
	timerbox.c \
	control.c
libworkrave_private_1_0_la_CFLAGS = \
	-std=c99 $(GTK_CFLAGS) -I$(srcdir)/../include \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
libworkrave_private_1_0_la_LIBADD = \
	$(GTK_LIBS)

introspection_sources = $(libworkrave_private_1_0_la_SOURCES)

Workrave-1.0.gir: libworkrave-private-1.0.la
Workrave_1_0_gir_INCLUDES = GObject-2.0
Workrave_1_0_gir_CFLAGS = $(INCLUDES) $(GTK_CFLAGS) -I$(srcdir)/../include
Workrave_1_0_gir_LIBS = libworkrave-private-1.0.la
Workrave_1_0_gir_FILES = $(introspection_sources) $(srcdir)/../include/timerbox.h $(srcdir)/../include/timebar.h
INTROSPECTION_GIRS += Workrave-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)
endif

if HAVE_GTK2

workravegtk2libdir = $(libdir)
workravegtk2lib_LTLIBRARIES = libworkrave-gtk2-private-1.0.la
libworkrave_gtk2_private_1_0_la_SOURCES = \
	timebar.c \
	timerbox.c \
	control.c \
	compat.c
libworkrave_gtk2_private_1_0_la_CFLAGS = \
	-DUSE_GTK2 -std=c99 $(GTK2_CFLAGS) -I$(srcdir)/../include \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
libworkrave_gtk2_private_1_0_la_LIBADD = \
	$(GTK2_LIBS)

introspection_sources = $(libworkrave_private_1_0_la_SOURCES)

endif
EXTRA_DIST = compat.h



EXTRA_DIST 	= $(ui_DATA)					\
		  $(wildcard $(srcdir)/*.xml.in)				\
		  Workrave-Applet.server.in.in			\
	  	  WorkraveApplet.c				\
		  WorkraveApplet.h				\
		  DBusGUI.xml					\
		  DBusGnomeApplet.xml				\
		  gnome-applet.xml

MAINTAINERCLEANFILES =  Makefile.in GNOME_WorkraveApplet.xml \
			Workrave-Applet.server.in Workrave-Applet.server \
			DBusGnomeApplet.xml DBusGUI.xml

SUBDIRS = 

if !HAVE_PANELAPPLET3
if HAVE_GNOMEAPPLET

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WorkraveApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = Workrave-Applet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

INCLUDES 			= -I$(srcdir)

FLAGS				= @GNOME_CFLAGS@ @DBUSGLIB_CFLAGS@ \
				  @WR_FRONTEND_COMMON_INCLUDES@ @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
	  	  		  -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
	  	  		  -DWORKRAVE_UIDATADIR="\"${uidir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\"" 

libexec_PROGRAMS 		= workrave-applet

workrave_applet_SOURCES 	= WorkraveApplet.c
workrave_applet_LDADD 		= @GNOME_LIBS@ @DBUSGLIB_LIBS@
workrave_applet_CXXFLAGS 	= ${FLAGS}
workrave_applet_CFLAGS 		= ${FLAGS}

GNOME_WorkraveApplet.xml: GNOME_WorkraveApplet.xml.in
	sed -e "s|\@PIXDIR\@|$(pkgdatadir)/images|" $< > $@

Workrave-Applet.server.in: Workrave-Applet.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" -e "s|\@ICONSDIR\@|$(datadir)/pixmaps/workrave|" $< > $@

@INTLTOOL_SERVER_RULE@

BUILT_SOURCES=applet-server-bindings.h applet-client-bindings.h gui-client-bindings.h  Workrave-Applet.server Workrave-Applet.server.in

# Correctly clean the generated headers, but keep the xml description
CLEANFILES = $(BUILT_SOURCES)

# We don't want to install this header
#noinst_HEADERS = $(DBUS_HEADER_SOURCES)

#Rule to generate the binding headers
applet-server-bindings.h:  DBusGnomeApplet.xml
	dbus-binding-tool --prefix=workrave --mode=glib-server $<> $@

applet-client-bindings.h:  DBusGnomeApplet.xml
	dbus-binding-tool --prefix=workrave --mode=glib-client $<> $@

gui-client-bindings.h:  DBusGUI.xml
	dbus-binding-tool --prefix=workrave --mode=glib-client $<> $@

endif
endif

if HAVE_PYTHON_CHEETAH

DBusGUI.xml: $(top_srcdir)/frontend/gtkmm/src/workrave-gui.xml
	python $(top_srcdir)/common/bin/dbusgen.py --language=dbus-glib $(srcdir)/../workrave-gui.xml DBusGUI

DBusGnomeApplet.xml: gnome-applet.xml
	python $(top_srcdir)/common/bin/dbusgen.py --language=dbus-glib $(srcdir)/gnome-applet.xml DBusGnomeApplet

endif

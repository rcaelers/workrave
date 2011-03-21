EXTRA_DIST 	= org.workrave.WorkraveApplet.panel-applet.in.in 	\
		  org.workrave.WorkraveAppletFactory.service.in		\
	  	  WorkraveApplet.c				 	\
		  WorkraveApplet.h				 	\
		  DBusGUI.xml					 	\
		  DBusGnomeApplet.xml				 	\
		  gnome-applet.xml					\
		  workrave-applet-menu.xml

CLEANFILES 	= $(applet_DATA) $(applet_DATA).in $(service_DATA) $(schemas_DATA)

MAINTAINERCLEANFILES =  Makefile.in \
			DBusGnomeApplet.xml DBusGUI.xml

SUBDIRS = 

if HAVE_GNOMEAPPLET
if HAVE_PANELAPPLET3

APPLET_IN_PROCESS 		= false
APPLET_LOCATION   		= $(libexecdir)/workrave-applet

appletdir       		= $(datadir)/gnome-panel/applets
applet_in_files 		= org.workrave.WorkraveApplet.panel-applet.in
applet_DATA     		= $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
		-e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
		-e "s|\@IN_PROCESS\@|$(APPLET_IN_PROCESS)|" \
		-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
		$< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

servicedir       		= $(datadir)/dbus-1/services
service_in_files 		= org.workrave.WorkraveAppletFactory.service.in
service_DATA     		= $(service_in_files:.service.in=.service)

org.workrave.WorkraveAppletFactory.service: $(service_in_files)
	$(AM_V_GEN)sed \
	    -e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
	    $< > $@

xmluidir    			= $(datadir)/gnome-panel/ui
xmlui_DATA  			= workrave-applet-menu.xml

INCLUDES 			= -I$(srcdir)

FLAGS				= @GNOME_CFLAGS@ @DBUSGLIB_CFLAGS@ \
				  @WR_FRONTEND_COMMON_INCLUDES@ @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
	  	  		  -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
	  	  		  -DWORKRAVE_UIDATADIR="\"${xmluidir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\"" 

libexec_PROGRAMS 		= workrave-applet

workrave_applet_SOURCES 	= WorkraveApplet.c
workrave_applet_LDADD 		= @GNOME_LIBS@ @DBUSGLIB_LIBS@
workrave_applet_CXXFLAGS 	= ${FLAGS}
workrave_applet_CFLAGS 		= ${FLAGS}

BUILT_SOURCES=applet-server-bindings.h applet-client-bindings.h gui-client-bindings.h 

# Correctly clean the generated headers, but keep the xml description
CLEANFILES += $(BUILT_SOURCES)

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

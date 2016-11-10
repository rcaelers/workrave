EXTRA_DIST 	= org.workrave.WorkraveApplet.panel-applet.in.in 		\
	  	  v4/WorkraveApplet.c				 		\
	  	  v5/WorkraveApplet.c				 		\
		  WorkraveApplet.h				 		\
		  workrave-gnome-applet.xml					\
		  v4/workrave-gnome-applet-menu.xml				\
		  v5/workrave-gnome-applet-menu.xml

BUILT_SOURCES   =
CLEANFILES 	= $(applet_DATA) $(applet_DATA).in $(schemas_DATA)

MAINTAINERCLEANFILES =  Makefile.in DBusGnomeApplet.xml DBusGUI.xml

SUBDIRS = 

if HAVE_PANELAPPLET45

xmluidir    			= $(datadir)/gnome-panel/ui
applet_in_files 		= org.workrave.WorkraveApplet.panel-applet.in
applet_DATA     		= $(applet_in_files:.panel-applet.in=.panel-applet)

if HAVE_PANELAPPLET4
LIBPANEL_APPLET_DIR		= `$(PKG_CONFIG) --variable=libpanel_applet_dir libpanelapplet-4.0`
appletdir       		= $(datadir)/gnome-panel/4.0/applets
xmlui_DATA  			= v4/workrave-gnome-applet-menu.xml
applet_source 			= v4/WorkraveApplet.c
else
LIBPANEL_APPLET_DIR		= `$(PKG_CONFIG) --variable=libpanel_applet_dir libpanel-applet`
appletdir			= $(LIBPANEL_APPLET_DIR)
xmlui_DATA  			= v5/workrave-gnome-applet-menu.xml
applet_source 			= v5/WorkraveApplet.c
endif

APPLET_IN_PROCESS 		= true
APPLET_LOCATION   		= ${pkglibdir}/libworkrave-applet.so

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
		-e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
		-e "s|\@IN_PROCESS\@|$(APPLET_IN_PROCESS)|" \
		-e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
		$< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

AM_CPPFLAGS 			= -I$(srcdir)

FLAGS				= @GNOME3_CFLAGS@ @WR_FRONTEND_COMMON_INCLUDES@ @WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ \
				  -std=c99 -I$(srcdir)/../include -I$(srcdir)/../../common/include \
	  	  		  -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
	  	  		  -DWORKRAVE_UIDATADIR="\"${xmluidir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\"" 

libapplet_libdir		= ${pkglibdir}
libapplet_lib_LTLIBRARIES	= libworkrave-applet.la

libworkrave_applet_la_SOURCES	= ${applet_source} workrave-gnome-applet-generated.c
libworkrave_applet_la_LIBADD	= @GNOME3_LIBS@  -L$(builddir)/../../common/src -lworkrave-private-1.0
libworkrave_applet_la_CXXFLAGS	= ${FLAGS}
libworkrave_applet_la_CFLAGS	= ${FLAGS}
libworkrave_applet_la_LDFLAGS	= -module -avoid-version

# ------------------------------------------------------------------------

GDBUS_SERVER_GENERATED = workrave-gnome-applet-generated.h workrave-gnome-applet-generated.c

$(GDBUS_SERVER_GENERATED) : workrave-gnome-applet.xml
	$(AM_V_GEN) gdbus-codegen					 	\
		--interface-prefix org.workrave.				\
		--c-namespace Workrave	 					\
		--c-generate-object-manager					\
		--generate-c-code workrave-gnome-applet-generated		\
		$<								

BUILT_SOURCES += $(GDBUS_SERVER_GENERATED)

CLEANFILES += $(GDBUS_SERVER_GENERATED)

endif

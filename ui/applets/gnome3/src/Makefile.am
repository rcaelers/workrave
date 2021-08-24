EXTRA_DIST 	= org.workrave.WorkraveApplet.panel-applet.in.in 		\
		  libgnome-panel/WorkraveApplet.c 				\
		  libgnome-panel/WorkraveApplet.h 				\
		  libgnome-panel/WorkraveModule.c 				\
		  libgnome-panel/workrave-gnome-applet-menu.xml 		\
		  libpanel-applet-v5/WorkraveApplet.c				 		\
		  libpanel-applet-v5/WorkraveApplet.h				 		\
		  libpanel-applet-v5/workrave-gnome-applet-menu.xml

BUILT_SOURCES   =
CLEANFILES 	= $(applet_DATA) $(applet_DATA).in $(schemas_DATA)

MAINTAINERCLEANFILES =  Makefile.in DBusGnomeApplet.xml DBusGUI.xml

SUBDIRS =

if HAVE_LIBGNOME_PANEL

appletdir = $(LIBGNOME_PANEL_MODULES_DIR)

applet_LTLIBRARIES = libworkrave-applet.la

xmluidir = $(pkgdatadir)/ui
xmlui_DATA = libgnome-panel/workrave-gnome-applet-menu.xml

applet_source = \
	libgnome-panel/WorkraveApplet.c \
	libgnome-panel/WorkraveApplet.h \
	libgnome-panel/WorkraveModule.c

AM_CPPFLAGS = -I$(srcdir)

FLAGS = \
	@GNOME3_CFLAGS@ \
	@WR_UI_COMMON_INCLUDES@ \
	@WR_COMMON_INCLUDES@ \
	-std=c99 \
	-I$(srcdir)/../include \
	-I$(srcdir)/../../common/include \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
	-DWORKRAVE_UIDATADIR="\"${xmluidir}\"" \
	-DGNOMELOCALEDIR="\"$(datadir)/locale\""

libworkrave_applet_la_SOURCES	= ${applet_source}
libworkrave_applet_la_LIBADD = @GNOME3_LIBS@ -L$(builddir)/../../common/src -lworkrave-private-1.0
libworkrave_applet_la_CXXFLAGS = ${FLAGS}
libworkrave_applet_la_CFLAGS = ${FLAGS}
libworkrave_applet_la_LDFLAGS = -module -avoid-version

endif # HAVE_LIBGNOME_PANEL

if HAVE_LIBPANEL_APPLET

# Common

xmluidir    			  = $(datadir)/gnome-panel/ui
applet_in_files 		= org.workrave.WorkraveApplet.panel-applet.in
applet_DATA     		= $(applet_in_files:.panel-applet.in=.panel-applet)

if LOCALINSTALL
appletdir			= $(datadir)/gnome-panel/applets
else
appletdir			= `$(PKG_CONFIG) --variable=libpanel_applet_dir libpanel-applet`
endif

xmlui_DATA  			= libpanel-applet-v5/workrave-gnome-applet-menu.xml
applet_source 			= libpanel-applet-v5/WorkraveApplet.c

APPLET_IN_PROCESS 		= true
APPLET_LOCATION   		= ${pkglibdir}/libworkrave-applet.so

$(applet_in_files): $(applet_in_files).in Makefile
	$(AM_V_GEN)sed \
		-e "s|\@LOCATION\@|$(APPLET_LOCATION)|" \
		-e "s|\@IN_PROCESS\@|$(APPLET_IN_PROCESS)|" \
		$< > $@

%.panel-applet: %.panel-applet.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

AM_CPPFLAGS 			= -I$(srcdir)

FLAGS				= @GNOME3_CFLAGS@ @WR_UI_COMMON_INCLUDES@ @WR_COMMON_INCLUDES@ \
				  -std=c99 -I$(srcdir)/../include -I$(srcdir)/../../common/include \
	  	  		  -DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
	  	  		  -DWORKRAVE_UIDATADIR="\"${xmluidir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\"" 

libapplet_libdir		= ${pkglibdir}
libapplet_lib_LTLIBRARIES	= libworkrave-applet.la

libworkrave_applet_la_SOURCES	= ${applet_source}
libworkrave_applet_la_LIBADD	= @GNOME3_LIBS@  -L$(builddir)/../../common/src -lworkrave-private-1.0
libworkrave_applet_la_CXXFLAGS	= ${FLAGS}
libworkrave_applet_la_CFLAGS	= ${FLAGS}
libworkrave_applet_la_LDFLAGS	= -module -avoid-version

endif

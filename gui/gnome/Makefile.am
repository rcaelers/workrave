INCLUDES =							\
	-I$(srcdir)						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps\""		\
	$(GNOME_INCLUDEDIR)					\
	$(WARN_CFLAGS)						\
	@APPLET_CFLAGS@						\
	@ARCH_CFLAGS@

SUBDIRS = 

libexec_PROGRAMS = workrave-applet

workrave_applet_SOURCES = workrave-applet.c

workrave_applet_LDADD = \
	@APPLET_LIBS@

EXTRA_DIST =					\
	GNOME_WorkraveApplet_Factory.server.in  \
	workrave.schemas.in				\
	$(ui_DATA)

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WorkraveApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_WorkraveApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

GNOME_WorkraveApplet_Factory.server: GNOME_WorkraveApplet_Factory.server.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

#@INTLTOOL_SERVER_RULE@

#schemasdir       = $(sysconfdir)/gconf/schemas
#schemas_in_files = workrave.schemas.in
#schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

#@INTLTOOL_SCHEMAS_RULE@

# don't do this if we are building in eg. rpm
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/applets/workrave/$$p ; \
	done \
	fi


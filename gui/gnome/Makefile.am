EXTRA_DIST =					\
	GNOME_WorkraveApplet_Factory.server.in.in  \
	workrave.schemas.in				\
	$(ui_DATA)

MAINTAINERCLEANFILES = GNOME_WorkraveApplet_Factory.server.in GNOME_WorkraveApplet_Factory.server

SUBDIRS = 

if HAVE_GNOME

INCLUDES =							\
	-I$(srcdir)						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"		\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps\""		\
	-DWORKRAVE_DATADIR="\"${pkgdatadir}\""			\
	$(GNOME_INCLUDEDIR)					\
	$(WARN_CFLAGS)						\
	@APPLET_CFLAGS@						\
	@ARCH_CFLAGS@						\
	`orbit2-config --cflags server`

libexec_PROGRAMS = workrave-applet

CORBA_GENERATED = \
	Bonobo_Sample_Moody-common.c \
	Bonobo_Sample_Moody-skels.c \
	Bonobo_Sample_Moody.h

workrave_applet_SOURCES = WorkraveApplet.cc ${CORBA_GENERATED}
workrave_applet_LDADD = @APPLET_LIBS@

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WorkraveApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_WorkraveApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

GNOME_WorkraveApplet_Factory.server.in: GNOME_WorkraveApplet_Factory.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

# don't do this if we are building in eg. rpm
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/applets/workrave/$$p ; \
	done \
	fi



#bonobo-sample-moody-component: ${CORBA_GENERATED} ${OBJECTS}
#	gcc -o $@ ${OBJECTS} `orbit2-config --libs server` `pkg-config --libs libbonobo-2.0 gobject-2.0`#
#
#.c.o:	
#	gcc -c $<  `pkg-config --cflags libbonobo-2.0 gobject-2.0` 

$(CORBA_GENERATED):
	orbit-idl-2 --nostubs x_.idl -I/usr/share/idl

endif
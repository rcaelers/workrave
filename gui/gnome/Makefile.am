uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WorkraveApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = Workrave-Applet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

EXTRA_DIST 	= Workrave-Applet.server.in.in  		\
	     	  workrave.schemas.in				\
	     	  $(ui_DATA)					\
		  $(wildcard *.hh) $(wildcard *.cc)		\
		  $(wildcard *.c) $(wildcard *.h)		\
		  $(wildcard *.icc) $(wildcard *.idl)

IDLS 		= Workrave-Applet.idl

IDL_GENERATED   = Workrave-Applet.h			\
		  Workrave-Applet-common.c		\
	  	  Workrave-Applet-skels.c		\
		  Workrave-Applet-stubs.c


MAINTAINERCLEANFILES =  Workrave-Applet.server.in Workrave-Applet.server $(IDL_GENERATED)


SUBDIRS = 

if HAVE_GNOME

## CORBA stuff

$(IDL_GENERATED): $(IDLS) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) Workrave-Applet.idl

## End CORBA stuff

INCLUDES 			= -I$(srcdir)

FLAGS				= @APPLET_CFLAGS@ @ARCH_CFLAGS@ \
	  	  		  -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\""

libexec_PROGRAMS 		= workrave-applet

workrave_applet_SOURCES 	= $(IDL_GENERATED) WorkraveApplet.c WorkraveAppletControl.c 
workrave_applet_LDADD 		= @APPLET_LIBS@
workrave_applet_CXXFLAGS 	= ${FLAGS}
workrave_applet_CFLAGS 		= ${FLAGS}

Workrave-Applet.server.in: Workrave-Applet.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

@INTLTOOL_SERVER_RULE@

endif
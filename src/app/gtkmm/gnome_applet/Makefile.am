EXTRA_DIST 	= Workrave-Applet.server.in.in  		\
		  Workrave-Control.server.in.in  		\
	     	  $(ui_DATA)					\
		  $(wildcard *.xml.in)				\
		  $(wildcard *.hh) $(wildcard *.cc)		\
		  $(wildcard *.c) $(wildcard *.h)		\
		  $(wildcard *.icc) $(wildcard *.idl)

IDL_A 		= Workrave-Applet.idl

IDL_A_GENERATED = Workrave-Applet.h			\
		  Workrave-Applet-common.c		\
	  	  Workrave-Applet-skels.c		\
		  Workrave-Applet-stubs.c

IDL_C 		= Workrave-Control.idl

IDL_C_GENERATED = Workrave-Control.h			\
		  Workrave-Control-common.c		\
	  	  Workrave-Control-skels.c		\
		  Workrave-Control-stubs.c

MAINTAINERCLEANFILES =  Workrave-Applet.server.in Workrave-Applet.server \
			Workrave-Control.server.in Workrave-Control.server \
			$(IDL_A_GENERATED) $(IDL_C_GENERATED) \
			Makefile.in GNOME_WorkraveApplet.xml

SUBDIRS = 

if HAVE_GNOME

uidir		= $(datadir)/gnome-2.0/ui
ui_DATA		= GNOME_WorkraveApplet.xml

serverdir       = $(libdir)/bonobo/servers
server_in_files = Workrave-Applet.server.in Workrave-Control.server.in
server_DATA     = $(server_in_files:.server.in=.server)

## CORBA stuff

$(IDL_A_GENERATED): $(IDL_A) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) Workrave-Applet.idl

$(IDL_C_GENERATED): $(IDL_C) 
	$(ORBIT_IDL) -I $(srcdir) -I $(datadir)/idl $(IDL_INCLUDES) Workrave-Control.idl

## End CORBA stuff

INCLUDES 			= -I$(srcdir)

FLAGS				= @APPLET_CFLAGS@ @SHARED_CFLAGS@ \
	  	  		  -DWORKRAVE_DATADIR="\"${pkgdatadir}\"" \
	 		 	  -DGNOMELOCALEDIR="\"$(datadir)/locale\""

libexec_PROGRAMS 		= workrave-applet

workrave_applet_SOURCES 	= $(IDL_C_GENERATED) $(IDL_A_GENERATED) WorkraveApplet.c
workrave_applet_LDADD 		= @APPLET_LIBS@
workrave_applet_CXXFLAGS 	= ${FLAGS}
workrave_applet_CFLAGS 		= ${FLAGS}

Workrave-Applet.server.in: Workrave-Applet.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

Workrave-Control.server.in: Workrave-Control.server.in.in
	sed -e "s|\@BINDIR\@|$(bindir)|" $< > $@

GNOME_WorkraveApplet.xml: GNOME_WorkraveApplet.xml.in
	sed -e "s|\@PIXDIR\@|$(pkgdatadir)/images|" $< > $@

@INTLTOOL_SERVER_RULE@

endif

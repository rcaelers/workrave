# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2007, 2008, 2011 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in workrave.soundlist
DISTCLEANFILES = 	workrave.soundlist

SUBDIRS	= subtle default

soundsdir = 		$(datadir)/sounds/workrave/
defaultsoundsdir = 	$(soundsdir)/default

EXTRA_DIST = 		${defaultsounds_DATA} $(utils_DATA) workrave.soundlist.in eventsrc.in

if HAVE_KDE

kdesoundsdir = 		$(kde_sounddir)/workrave
kdesounds_DATA = 	$(wildcard default/*.wav)

eventsrc: eventsrc.in
	LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

##	$(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)

rcdir = $(kde_datadir)/kworkrave
rc_DATA = eventsrc

endif

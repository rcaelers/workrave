# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2007, 2008 Rob Caelers & Raymond Penners
#
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in workrave.soundlist
DISTCLEANFILES = 	workrave.soundlist

SUBDIRS	= subtle

soundsdir = 		$(datadir)/sounds/workrave/
defaultsoundsdir = 	$(soundsdir)/default
defaultsounds_DATA = 	$(wildcard *.wav) soundtheme

if HAVE_GNOME

utilsdir = 		$(sysconfdir)/sound/events
utils_in_files = 	workrave.soundlist.in
utils_DATA = 		$(utils_in_files:.soundlist.in=.soundlist)

@INTLTOOL_SOUNDLIST_RULE@

endif

EXTRA_DIST = 		${defaultsounds_DATA} $(utils_DATA) workrave.soundlist.in eventsrc.in


if HAVE_KDE

kdesoundsdir = 		$(kde_sounddir)/workrave
kdesounds_DATA = 	$(wildcard *.wav)

eventsrc: eventsrc.in
	LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

##	$(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)

rcdir = $(kde_datadir)/kworkrave
rc_DATA = eventsrc

endif

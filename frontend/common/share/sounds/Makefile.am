# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
#
#
# $Id$
#

MAINTAINERCLEANFILES = 	Makefile.in workrave.soundlist

soundsdir = 		$(datadir)/sounds/workrave
sounds_DATA = 		$(wildcard *.wav)

utilsdir = 		$(sysconfdir)/sound/events
utils_in_files = 	workrave.soundlist.in
utils_DATA = 		$(utils_in_files:.soundlist.in=.soundlist)

@INTLTOOL_SOUNDLIST_RULE@

EXTRA_DIST = 		${sounds_DATA} $(utils_DATA) workrave.soundlist.in eventsrc.in


if HAVE_KDE

kdesoundsdir = 		$(kde_sounddir)/workrave
kdesounds_DATA = 	$(wildcard *.wav)

eventsrc: eventsrc.in
	LC_ALL=C $(INTLTOOL_MERGE) -d -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

##	$(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po)

rcdir = $(kde_datadir)/kworkrave
rc_DATA = eventsrc

endif

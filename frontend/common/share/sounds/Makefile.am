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

EXTRA_DIST = 		${sounds_DATA} $(utils_DATA) workrave.soundlist.in


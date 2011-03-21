# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2003, 2008 Raymond Penners <raymond@dotsphinx.com>
#

MAINTAINERCLEANFILES = 	Makefile.in
DISTCLEANFILES =	exercises.xml

exercisesdir = 		$(pkgdatadir)/exercises
exercises_DATA = 	$(wildcard $(srcdir)/*.png) exercises.xml

xmldir = 		$(exercisesdir)

xml_in_files = 		exercises.xml.in
xml_DATA = 		$(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = 		${exercises_DATA} $(xml_in_files) $(xml_DATA)

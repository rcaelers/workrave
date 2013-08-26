# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2004, 2005, 2008, 2011 Rob Caelers & Raymond Penners
#
#

MAINTAINERCLEANFILES = 	Makefile.in

SUBDIRS = 		win32 osx 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable

imagesdir = 		$(pkgdatadir)/images
images_DATA = 		$(wildcard $(srcdir)/*.png) $(wildcard $(srcdir)/*.ico) $(wildcard $(srcdir)/*.xpm)

svgdir = 		$(datadir)/icons/hicolor/scalable/$(category)
svg_DATA = 		workrave-sheep.svg

EXTRA_DIST = 		${images_DATA} $(icons_DATA) $(svg_DATA)

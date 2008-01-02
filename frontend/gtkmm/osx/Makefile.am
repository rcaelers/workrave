# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2008 Rob Caelers & Raymond Penners
#
# $Id: Makefile.am 687 2004-01-28 12:16:46Z dotsphinx $
#

MAINTAINERCLEANFILES = 	Makefile.in Info.plist install.log tmp.r Workrave.dmg

clean-local:
	-rm -rf Workrave.app
	-rm -rf workrave-image

install-exec-hook:
	./make-app.sh
	./make-image.sh

# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2002, 2003, 2008 Rob Caelers & Raymond Penners
#

MAINTAINERCLEANFILES = 	Makefile.in Info.plist install.log tmp.r Workrave.dmg

clean-local:
	-rm -rf Workrave.app
	-rm -rf workrave-image

dmg:
	./make-app.sh
	./make-image.sh

EXTRA_DIST = Info.plist.in Workrave.webloc dmg.applescript dmg_background.png fonts.conf import-frameworks.sh make-app.sh make-image.sh relocate-framework.sh workrave

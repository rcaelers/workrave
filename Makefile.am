# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
#
# $Id$
#

MAINTAINERCLEANFILES = 	\
		aclocal.m4 config.h.in configure stamp-h.in \
		Makefile.in \
		install-sh missing mkinstalldirs config.cache \
		config.log config.status \
		config.guess config.sub depcomp ltmain.sh \
		build-stamp intltool-update.in intltool-update \
		intltool-merge.in intltool-merge \
		intltool-extract.in intltool-extract workrave.spec \
		compile AUTHORS.txt COPYING.txt ChangeLog.txt \
		NEWS.txt README.txt

SUBDIRS = 	m4 intl po backend common frontend 
DIST_SUBDIRS =  $(SUBDIRS) build contrib

EXTRA_DIST = 	config.rpath  \
		config.xml \
		autogen.sh \
		autogenwin32.sh \
		autogencross.sh \
		depcomp \
		workrave.spec.in \
		workrave.spec \
		intltool-extract.in intltool-merge.in intltool-update.in

WIN32_NMAKE=wineconsole ~/.wine/fake_windows/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/bin/nmake.exe

noinst_PROGRAMS =

if PLATFORM_WIN32
dist_pkgdata_DATA = README.txt NEWS.txt AUTHORS.txt COPYING.txt
endif

ACLOCAL_AMFLAGS = -I m4

unix2dos = perl -e 'while (<>) { s/$$/\r/; print; }'

README.txt: README
	$(unix2dos) <$^ >$@

NEWS.txt: NEWS
	$(unix2dos) <$^ >$@

AUTHORS.txt: AUTHORS
	$(unix2dos) <$^ >$@

# Removed from build, because our ChangeLog is empty.
ChangeLog.txt: ChangeLog
	$(unix2dos) <$^ >$@

COPYING.txt: COPYING
	$(unix2dos) <$^ >$@

changelog:
	svn2cl --group-by-day -i --authors=./build/authors.xml

win32:
	( . build/vcvars32.sh; \
          cd common/win32/harpoon/src; \
	  ${WIN32_NMAKE} /f harpoon.mak CFG="harpoon - Win32 Release"; \
	  cd ../../../.. \
          cd frontend/win32/applet/src; \
          ${WIN32_NMAKE} /f Applet.mak CFG="Applet - Win32 Release"; \
	  cd ../../../.. \
	)

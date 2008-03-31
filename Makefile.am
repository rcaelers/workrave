# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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
		compile AUTHORS.txt COPYING.txt \
		NEWS.txt README.txt
DISTCLEANFILES = intltool-extract intltool-merge intltool-update

SUBDIRS =      intl m4 po common backend frontend 
DIST_SUBDIRS = $(SUBDIRS) build contrib

EXTRA_DIST = 	config.rpath  \
		config.xml \
		autogen.sh \
		autogenwin32.sh \
		autogencross.sh \
		depcomp \
		workrave.spec.in \
		workrave.spec \
		intltool-extract.in intltool-merge.in intltool-update.in \
		SConstruct

WIN32_NMAKE=wineconsole ~/.wine/fake_windows/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/bin/nmake.exe

noinst_PROGRAMS =

if PLATFORM_OS_WIN32
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

COPYING.txt: COPYING
	$(unix2dos) <$^ >$@

win32:
	( . build/vcvars32.sh; \
          cd common/win32/harpoon/src; \
	  ${WIN32_NMAKE} /f harpoon.mak CFG="harpoon - Win32 Release"; \
	  cd ../../../.. \
          cd frontend/win32/applet/src; \
          ${WIN32_NMAKE} /f Applet.mak CFG="Applet - Win32 Release"; \
	  cd ../../../.. \
	)

version.h: config.h
	PACKAGE_VERSION=`grep PACKAGE_VERSION config.h | cut -d' ' -f 3 | sed "s/\\"//g"`
	echo "#ifndef WORKRAVE_VERSION_H" > version.h
	echo "#define WORKRAVE_VERSION_H" >> version.h
	if [ -e ".svn" ] ; then \
            PACKAGE_SVN_VERION=`svn info . | grep Revision: | sed -e 's/Revision: \([0-9]*\)/\1/'`; \
	    echo "#define WORKRAVE_VERSION \"$PACKAGE_VERSION" > version.h \
        fi
	echo "#define WORKRAVE_SVN_VERSION \"$PACKAGE_SVN_VERSION" > version.h
	echo "#endif // WORKRAVE_VERSION" > version.h

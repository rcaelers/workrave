# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012 Rob Caelers & Raymond Penners
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

SUBDIRS =      m4 po libs backend frontend 
DIST_SUBDIRS = $(SUBDIRS) build contrib

EXTRA_DIST = 	config.xml

DISTCHECK_CONFIGURE_FLAGS = --enable-localinstall

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

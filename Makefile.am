# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
#
# $Id$
#

MAINTAINERCLEANFILES 	= aclocal.m4 config.h.in configure stamp-h.in Makefile.in \
        install-sh missing mkinstalldirs config.cache config.log config.status \
	config.guess config.sub depcomp ltmain.sh build-stamp

SUBDIRS 	= intl m4 po arch gui src images debian sounds 

EXTRA_DIST	= \
	config.xml \
	autogen.sh \
	autogenwin32.sh \
	depcomp \
	workrave.spec.in

INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/arch $(GTKMM_CFLAGS) $(GTK_CFLAGS) $(SIGC_CFLAGS)

LDADD = $(GTKMM_LIBS) $(GTK_LIBS) $(SIGC_LIBS)

noinst_PROGRAMS =

if PLATFORM_WIN32
dist_pkgdata_DATA = README.txt NEWS.txt AUTHORS.txt COPYING.txt ChangeLog.txt
endif

ACLOCAL_AMFLAGS = -I m4


unix2dos = perl -e 'while (<>) { s/$$/\r/; print; }'

README.txt: README
	$(unix2dos) <$^ >$@

NEWS.txt: NEWS
	$(unix2dos) <$^ >$@

AUTHORS.txt: AUTHORS
	$(unix2dos) <$^ >$@

ChangeLog.txt: ChangeLog
	$(unix2dos) <$^ >$@

COPYING.txt: COPYING
	$(unix2dos) <$^ >$@

if HAVE_INDICATOR

libexec_PROGRAMS = indicator-workrave-test

workravelibdir = $(INDICATORDIR)
workravelib_LTLIBRARIES = libworkrave.la
libworkrave_la_SOURCES = \
	indicator-workrave.c
libworkrave_la_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -std=c99 -I../include -I../../common/include \
	-DG_LOG_DOMAIN=\"Indicator-Workrave\" \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
libworkrave_la_LIBADD = \
	$(INDICATOR_LIBS) -L../../common/src -lworkrave-1.0
libworkrave_la_LDFLAGS = \
	-module \
	-avoid-version

#	timebar.c \
#	timebar.h \
#	timerbox.c \
#	timerbox.h	
#

indicator_workrave_test_SOURCES = \
	test.c 
indicator_workrave_test_CFLAGS = \
	-Wall -std=c99 $(GTK_CFLAGS)  -I../include -I../../common/include \
	-DG_LOG_DOMAIN=\"Indicator-Workrave\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
indicator_workrave_test_LDADD = \
	$(GTK_LIBS) ../../common/src/libworkrave-1.0.la

BUILT_SOURCES =

endif

CLEANFILES = \
	$(BUILT_SOURCES)

EXTRA_DIST = 



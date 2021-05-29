MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = 	libworkrave-ui-common.la

libworkrave_ui_common_la_SOURCES = \
			Backend.cc \
			Exercise.cc \
			GUIConfig.cc \
			Session.cc \
			SoundTheme.cc \
			TimerBoxControl.cc

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

libworkrave_ui_common_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			@WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@ @GLIB_CFLAGS@ \
			-I$(top_srcdir)/ui/common/include/commonui

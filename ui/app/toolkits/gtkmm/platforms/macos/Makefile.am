MAINTAINERCLEANFILES = 	Makefile.in

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)
			 $(wildcard $(srcdir)/*.mm) $(wildcard $(srcdir)/*.m)

if PLATFORM_OS_MACOS

##AM_CPPFLAGS=-xobjective-c++

noinst_LTLIBRARIES = 	libworkrave-ui-gtkmm-macos.la
libworkrave_ui_gtkmm_macos_la_SOURCES = \
			MacOSUtil.cc \
			AppController.m \
			MacOSGtkMenu.cc \
			MacOSAppletWindow.cc \
			MacOSStatusBarView.mm \
			MacOSTimeBar.mm

libworkrave_ui_gtkmm_macos_la_OBJCFLAGS = \
			@X_CFLAGS@ -xobjective-c++ \
			-I$(top_srcdir)/ui/common/include \
			-I$(top_srcdir)/ui/app/toolkits/gtkmm/src @GTK_CFLAGS@ \
            -I$(top_srcdir)/common/include/macos \
            -I$(top_srcdir)/common/include

libworkrave_ui_gtkmm_macos_la_CFLAGS = \
			@X_CFLAGS@ \
			-I$(top_srcdir)/ui/common/include \
			-I$(top_srcdir)/ui/app/toolkits/gtkmm/src @GTK_CFLAGS@ \
            -I$(top_srcdir)/common/include/macos \
            -I$(top_srcdir)/common/include

libworkrave_ui_gtkmm_macos_la_CXXFLAGS = \
			@X_CFLAGS@ -xobjective-c++ \
			-I$(top_srcdir)/ui/common/include \
			-I$(top_srcdir)/ui/app/toolkits/gtkmm/src @GTK_CFLAGS@ @IGE_CFLAGS@ \
            -I$(top_srcdir)/common/include/macos \
            -I$(top_srcdir)/common/include

endif

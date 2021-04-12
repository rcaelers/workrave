if HAVE_XFCE

if HAVE_XFCE_GTK2

plugin_PROGRAMS = xfce4-workrave-plugin

plugindir = $(libdir)/xfce4/panel-plugins

xfce4_workrave_plugin_SOURCES = main.c

xfce4_workrave_plugin_CFLAGS =	-std=c99 \
				@WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@ \
				-I$(srcdir)/../include -I$(srcdir)/../../common/include \
				$(LIBXFCE4UTIL_CFLAGS) 	$(LIBXFCEGUI4_CFLAGS) $(LIBXFCE4PANEL_CFLAGS) \
				-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""

xfce4_workrave_plugin_LDADD = 	$(LIBXFCE4UTIL_LIBS) $(LIBXFCEGUI4_LIBS) $(LIBXFCE4PANEL_LIBS) \
				@GTK2_LIBS@ -L$(builddir)/../../common/src -lworkrave-gtk2-private-1.0

workrave-xfce-applet.desktop.in: workrave-xfce-applet-1.0.desktop.in.in
				sed -e "s,\@libexecdir\@,$(libdir),g" < $< > $@

endif

if HAVE_XFCE_GTK3

plugin_LTLIBRARIES = libworkrave-plugin.la

plugindir = $(libdir)/xfce4/panel/plugins

libworkrave_plugin_la_SOURCES = main.c

libworkrave_plugin_la_CFLAGS =	-std=c99 \
				@WR_COMMON_INCLUDES@ @WR_UI_COMMON_INCLUDES@ \
				-I$(srcdir)/../include -I$(srcdir)/../../common/include \
				$(LIBXFCE4UTIL_CFLAGS) 	$(LIBXFCEGUI4_CFLAGS) $(LIBXFCE4PANEL_CFLAGS) \
				-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""

libworkrave_plugin_la_LIBADD = $(LIBXFCE4UTIL_LIBS) $(LIBXFCEGUI4_LIBS) $(LIBXFCE4PANEL_LIBS) \
				-L$(builddir)/../../common/src -lworkrave-private-1.0

libworkrave_plugin_la_LDFLAGS = \
				-avoid-version \
				-module \
				-no-undefined \
				-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)'

workrave-xfce-applet.desktop.in: workrave-xfce-applet-2.0.desktop.in.in
				sed -e "s,\@libexecdir\@,$(libdir),g" < $< > $@

endif

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = workrave-xfce-applet.desktop

workrave-xfce-applet.desktop: workrave-xfce-applet.desktop.in
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

endif

desktop_in_in_files = workrave-xfce-applet-1.0.desktop.in.in workrave-xfce-applet-2.0.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
  workrave-xfce-applet.desktop.in \
	$(desktop_in_files)				\
	$(desktop_DATA)

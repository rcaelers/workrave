if HAVE_XFCE

plugin_PROGRAMS = xfce4-workrave-plugin

plugindir = $(libdir)/xfce4/panel-plugins

xfce4_workrave_plugin_SOURCES = main.c

#s-menu.c s-menu.h image.c image.h

xfce4_workrave_plugin_CFLAGS =	-Wall -std=c99 \
				@WR_COMMON_INCLUDES@ @WR_BACKEND_INCLUDES@ @WR_FRONTEND_COMMON_INCLUDES@ \
				-I$(srcdir)/../include -I$(srcdir)/../../common/include \
				$(LIBXFCE4UTIL_CFLAGS) 	$(LIBXFCEGUI4_CFLAGS) $(LIBXFCE4PANEL_CFLAGS) \
	  	  		-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""

xfce4_workrave_plugin_LDADD = 	$(LIBXFCE4UTIL_LIBS) $(LIBXFCEGUI4_LIBS) $(LIBXFCE4PANEL_LIBS) \
			        @GTK2_LIBS@ -L$(builddir)/../../common/src -lworkrave-gtk2-private-1.0

desktopdir = $(datadir)/xfce4/panel-plugins
desktop_in_in_files = workrave-xfce-applet.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libdir),g" < $< > $@
desktop_DATA =								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

endif

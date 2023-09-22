if HAVE_GNOME_45

topextensiondir = $(datadir)/gnome-shell/extensions
extensionurl = http://www.workrave.org
uuid = workrave@workrave.org

extensiondir = $(topextensiondir)/$(uuid)

dist_extension_DATA = extension.js stylesheet.css
nodist_extension_DATA = metadata.json $(EXTRA_EXTENSION)

metadata.json: metadata.json.in $(top_builddir)/config.status
	$(AM_V_GEN) sed -e "s|[@]LOCALEDIR@|$(datadir)/locale|" \
	    -e "s|[@]uuid@|$(uuid)|" \
	    -e "s|[@]shell_current@|$(GNOME_SHELL_VERSION)|" \
	    -e "s|[@]url@|$(extensionurl)|" $< > $@

endif

EXTRA_DIST = metadata.json.in extension.js stylesheet.css
CLEANFILES = metadata.json

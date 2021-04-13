topextensiondir = $(datadir)/cinnamon/applets
extensionurl = http://www.workrave.org
uuid = workrave@workrave.org

extensiondir = $(topextensiondir)/$(uuid)

dist_extension_DATA = applet.js
nodist_extension_DATA = metadata.json $(EXTRA_EXTENSION)

EXTRA_DIST = metadata.json.in

metadata.json: metadata.json.in $(top_builddir)/config.status
	$(AM_V_GEN) sed -e "s|[@]LOCALEDIR@|$(datadir)/locale|" \
	    -e "s|[@]uuid@|$(uuid)|" \
	    -e "s|[@]url@|$(extensionurl)|" $< > $@

CLEANFILES = metadata.json

DOCDIR = $(DESTDIR)/$(datadir)/doc/@PACKAGE@-@VERSION@
HTML_FILES =  html/*.html
IMAGE_FILES = images/*.png
EXTRA_DIST =  manual.xsl style.css $(srcdir)/$(IMAGE_FILES)
CLEANFILES = pdf-build.stamp txt-build.stamp htmldoc-build.stamp

pdf-build.stamp: manual.xml
	SP_ENCODING=XML SP_CHARSET_FIXED=YES \
	$(JW) -b pdf $(srcdir)/manual.xml && \
	mv $(srcdir)/manual.pdf manual-@VERSION@.pdf && \
	touch pdf-build.stamp

txt-build.stamp: manual.xml
	$(XMLTO) txt $(srcdir)/manual.xml && \
	mv $(srcdir)/manual.txt manual-@VERSION@.txt && \
	touch txt-build.stamp

htmldoc-build.stamp: manual.xml
	$(mkinstalldirs) html && \
	$(XMLTO) -m manual.xsl -o html/ xhtml $(srcdir)/manual.xml && \
	cp $(srcdir)/style.css html/ && \
	touch htmldoc-build.stamp

doc: txt-build.stamp htmldoc-build.stamp pdf-build.stamp

if ENABLE_MANUAL
all-local: doc
else
all-local:
endif

clean-local:
	rm -rf html/*.html
	rm -f html/style.css
	rm -f manual*.txt
	rm -f *.pdf

uninstall-local:
	rm -rf $(DOCDIR);

install-data-local:
	installfiles=`echo $(srcdir)/html/*`; \
	if test "$$installfiles" = '$(srcdir)/html/*'; then \
	echo '-- Nothing to install' ; \
	else \
	  $(mkinstalldirs) $(DOCDIR)/html/images ; \
	  for file in `ls $(srcdir)/$(HTML_FILES)`; do \
	    if [ -f $$file ]; then \
	      basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	      $(INSTALL_DATA) $$file $(DOCDIR)/html/$$basefile; \
	    fi \
	  done; \
	  for file in `ls $(srcdir)/$(IMAGE_FILES)`; do \
	    if [ -f $$file ]; then \
	      basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	      $(INSTALL_DATA) $$file $(DOCDIR)/html/images/$$basefile; \
	    fi \
	  done; \
	  $(INSTALL_DATA) $(srcdir)/style.css $(DOCDIR)/html/style.css; \
	  $(INSTALL_DATA) $(srcdir)/manual-@VERSION@.txt $(DOCDIR)/manual-@VERSION@.txt; \
	  $(INSTALL_DATA) $(srcdir)/manual-@VERSION@.pdf $(DOCDIR)/manual-@VERSION@.pdf; \
	fi

.PHONY : doc

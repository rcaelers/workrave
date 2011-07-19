if HAVE_INDICATOR

libexec_PROGRAMS = indicator-workrave-service indicator-workrave-test

dbus_servicesdir = $(DBUS_SERVICES_DIR)
dbus_services_DATA = indicator-workrave.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

indicator_workrave_service_SOURCES = \
	workrave-interface.c \
	workrave-interface.h \
	gen-workrave-service.xml.c \
	workrave-service.c \
	dbus-shared.h \
	settings-shared.h
indicator_workrave_service_CFLAGS = \
	-Wall \
	$(INDICATOR_CFLAGS) \
	-DG_LOG_DOMAIN=\"Indicator-Workrave\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"
indicator_workrave_service_LDADD = \
	$(INDICATOR_LIBS)

workravelibdir = $(INDICATORDIR)
workravelib_LTLIBRARIES = libworkrave.la
libworkrave_la_SOURCES = \
	gen-workrave-service.xml.h \
	dbus-shared.h \
	indicator-workrave.c \
	timebar.c \
	timebar.h \
	timerbox.c \
	timerbox.h
libworkrave_la_CFLAGS = \
	$(INDICATOR_CFLAGS) \
	-Wall -std=c99 \
	-DG_LOG_DOMAIN=\"Indicator-Workrave\" \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
libworkrave_la_LIBADD = \
	$(INDICATOR_LIBS)
libworkrave_la_LDFLAGS = \
	-module \
	-avoid-version

indicator_workrave_test_SOURCES = \
	timebar.c \
	timebar.h \
	timerbox.c \
	timerbox.h \
	test.c
indicator_workrave_test_CFLAGS = \
	-Wall -std=c99 $(GTK_CFLAGS) \
	-DG_LOG_DOMAIN=\"Indicator-Workrave\" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\""
indicator_workrave_test_LDADD = \
	$(GTK_LIBS)


gen-%.xml.c: %.xml
	@echo "Building $@ from $<"
	@echo "const char * _$(subst -,_,$(subst .,_,$(basename $<))) = " > $@
	@sed -e "s:\":\\\\\":g" -e s:^:\": -e s:\$$:\\\\n\": $< >> $@
	@echo ";" >> $@

gen-%.xml.h: %.xml
	@echo "Building $@ from $<"
	@echo "extern const char * _$(subst -,_,$(subst .,_,$(basename $<)));" > $@

BUILT_SOURCES = \
	gen-workrave-service.xml.c \
	gen-workrave-service.xml.h
endif

CLEANFILES = \
	$(BUILT_SOURCES) \
	indicator-workrave.service

EXTRA_DIST = \
	workrave-service.xml \
	indicator-workrave.service.in




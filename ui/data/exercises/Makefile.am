MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = exercises.xml

exercisesdir = $(pkgdatadir)/exercises
exercises_DATA = $(wildcard $(srcdir)/*.png)

xmldir = $(exercisesdir)
xml_DATA = exercises.xml

exercises.xml: exercises.xml.in
	$(AM_V_GEN)GETTEXTDATADIR=$(top_srcdir)/po $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = ${exercises_DATA} exercises.xml.in $(xml_DATA) $(wildcard $(srcdir)/*.xcf)

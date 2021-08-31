MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = libworkrave-audio.la

SUBDIRS = windows macos

cflags  =
sources = 	SoundPlayer.cc

if PLATFORM_OS_WINDOWS
sources +=  windows/W32Mixer.cc windows/W32SoundPlayer.cc
if HAVE_DSOUND
sources += 	windows/W32DirectSoundPlayer.cc
endif
cflags +=	-I$(top_srcdir)/libs/audio/src/windows
endif

if PLATFORM_OS_MACOS
sources += 	macos/MacOSSoundPlayer.cc
cflags += 	-I$(top_srcdir)/libs/audio/src/macos
endif

if PLATFORM_OS_UNIX
sources +=	GstSoundPlayer.cc PulseMixer.cc
cflags +=  	-I$(top_srcdir)/libs/audio/src/x11
endif

libworkrave_audio_la_SOURCES = ${sources}

libworkrave_audio_la_CXXFLAGS = \
			-DWORKRAVE_PKGDATADIR="\"${pkgdatadir}\"" \
			-D_XOPEN_SOURCE=600 @X_CFLAGS@ \
			@WR_COMMON_INCLUDES@ ${cflags} \
			@PULSE_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@ @GSTREAMER_CFLAGS@

EXTRA_DIST = $(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.cc) \
			 $(wildcard $(srcdir)/*.c) $(wildcard $(srcdir)/*.h)  $(wildcard $(srcdir)/*.icc)

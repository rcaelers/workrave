MAINTAINERCLEANFILES =	Makefile.in

noinst_LTLIBRARIES = libworkrave-audio.la

SUBDIRS = win32 x11 macos

cflags  =
sources = 	SoundPlayer.cc \
			GstSoundPlayer.cc \
			PulseMixer.cc

if PLATFORM_OS_WINDOWS
sources +=  win32/W32Mixer.cc win32/W32SoundPlayer.cc win32/Sound.cc
if HAVE_DSOUND
sources += 	win32/W32DirectSoundPlayer.cc
endif
cflags +=	-I$(top_srcdir)/libs/audio/src/win32
endif

if PLATFORM_OS_MACOS
sources += 	macos/MacOSSoundPlayer.cc macos/Sound.cc
cflags += 	-I$(top_srcdir)/libs/audio/src/macos
endif

if PLATFORM_OS_UNIX
sources += 	x11/Sound.cc
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

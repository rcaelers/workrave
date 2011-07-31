dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
dnl
dnl
dnl $Id$
dnl

m4_define([workrave_major_version], [1])
m4_define([workrave_minor_version], [9])
m4_define([workrave_micro_version], [4])
m4_define([workrave_version_suffix], [0])
m4_define([workrave_version],
          [workrave_major_version.workrave_minor_version.workrave_micro_version])
m4_define([workrave_resource_version],
          [workrave_major_version,workrave_minor_version,workrave_micro_version,workrave_version_suffix])

AC_INIT(workrave, [workrave_version],
        [http://issues.workrave.org/cgi-bin/bugzilla/enter_bug.cgi])

if test -d $srcdir/.git && test "$TINDERBOX_BUILD" = yes; then
    git_version=`git describe --tags --abbrev=10 --dirty 2>/dev/null`
    AC_DEFINE_UNQUOTED(GIT_VERSION, "$git_version", [GIT Version])
fi	

dnl Demand at least version 2.61 of autoconf.
AC_PREREQ(2.61)
AC_CONFIG_MACRO_DIR([m4])

dnl Automake
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(workrave, workrave_version)
AM_GNU_GETTEXT_VERSION()
AM_MAINTAINER_MODE

dnl Set of available languages.
AC_SUBST(ALL_LINGUAS)
ALL_LINGUAS="nl de eo pl da es zh_TW ru fr pt_BR nb lt cs he eu ca sv ja bg hu tr el sk zh_CN it sl ro et te ga ko fa hr uk"
AC_DEFINE_UNQUOTED(ALL_LINGUAS, "$ALL_LINGUAS" ,[All available languages])

dnl
dnl Command line options.                           
dnl

AC_ARG_ENABLE(app-gtk,
              [AS_HELP_STRING([--disable-app-gtk],
                              [compile without Gtk support (NOT recommended)])])

AC_ARG_ENABLE(app-text,
              [AS_HELP_STRING([--enable-app-text],
                              [compile with dummy text GUI support (NOT recommended)])])
    
AC_ARG_ENABLE(xml,
              [AS_HELP_STRING([--disable-xml],
                              [compile without XML support])])

AC_ARG_ENABLE(gconf,
              [AS_HELP_STRING([--disable-gconf],
                              [compile without Gnome gconf2 support])])

AC_ARG_ENABLE(gnome,
              [AS_HELP_STRING([--disable-gnome],
                              [compile without Gnome support])])

AC_ARG_ENABLE(gnome3,
              [AS_HELP_STRING([--enable-gnome3],
                              [compile with preliminary Gnome 3 support])],
              [test "x$enable_gnome3" = "xyes" && enable_gnome=yes])

AC_ARG_ENABLE(indicator,
              [AS_HELP_STRING([--enable-indicator],
                              [compile with preliminary Unity indicator applet support])])
	      
AC_ARG_ENABLE(dbus,
              [AS_HELP_STRING([--disable-dbus],
                              [compile without DBus support])])

AC_ARG_ENABLE(gstreamer,
              [AS_HELP_STRING([--disable-gstreamer],
                              [compile without GStreamer audio support])])

AC_ARG_ENABLE(pulse,
              [AS_HELP_STRING([--disable-pulse],
                              [compile without PulseAudio support])])

AC_ARG_ENABLE(x11-monitoring-fallback,
              [AS_HELP_STRING([--enable-x11-monitoring-fallback],
                              [compile with fallback monitoring if RECORD is not available (discouraged)])])

AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug],
                              [compile with debugging flags set])])

AC_ARG_ENABLE(distribution,
              [AS_HELP_STRING([--disable-distribution],
                              [compile without support for distributed operation])])

AC_ARG_ENABLE(experimental,
              [AS_HELP_STRING([--enable-experimental],
                              [compile with experimental features])])

AC_ARG_ENABLE(exercises,
              [AS_HELP_STRING([--disable-exercises],
                              [compile without exercises support])])

AC_ARG_ENABLE(tests,
              [AS_HELP_STRING([--enable-tests],
                              [compile with test code])])

AC_ARG_ENABLE(tracing,
              [AS_HELP_STRING([--enable-tracing],
                              [compile with method call tracing])],
              [test "x$enable_tracing" = "xyes" && enable_debug=yes])

      
case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac

	      
dnl
dnl Checks for programs
dnl

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_OBJC
AC_PROG_CXX
AC_PROG_CPP
AC_GNU_SOURCE
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_ISC_POSIX
AM_PROG_LIBTOOL
AC_CHECK_PROGS(M4, gm4 m4, m4)

AC_CHECK_PROGS(WINDRES, ${target_or_host}-windres,false)

dnl
dnl checks for compiler characteristics
dnl

AC_LANG_CPLUSPLUS
AC_REQUIRE_CPP
AC_C_INLINE
AC_C_CONST


dnl
dnl Headers/Types/Functions
dnl

AC_STDC_HEADERS
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdlib.h sys/time.h sys/select.h unistd.h])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCT.hwnd,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCT,,[struct MOUSEHOOKSTRUCT]),, [#include <windows.h>])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCTEX.mouseData,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCTEX,,[struct MOUSEHOOKSTRUCTEX]),, [#include <windows.h>])

have_ishelldispatch=no
AC_MSG_CHECKING([IShellDispatch])
AC_TRY_COMPILE([#include <shlobj.h>], 
        [ IShellDispatch* pShellDispatch = NULL; ],
        [have_ishelldispatch=yes
         AC_DEFINE(HAVE_ISHELLDISPATCH, 1, "IShellDispatch")
         AC_MSG_RESULT(yes)],
         AC_MSG_RESULT(no))
	 
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gettimeofday nanosleep select setlocale realpath])


have_extern_timezone_defined=no
AC_MSG_CHECKING([external timezone variable defined in time.h])
AC_TRY_COMPILE([#include <time.h>], 
        [ timezone += 1;
          return 0;],
        [have_extern_timezone_defined=yes
         AC_DEFINE(HAVE_EXTERN_TIMEZONE_DEFINED, 1, "External timezone")
         AC_MSG_RESULT(yes)],
         AC_MSG_RESULT(no))

have_extern_timezone=no
AC_MSG_CHECKING([external timezone variable])
AC_TRY_COMPILE([#include <time.h>], 
        [ extern long timezone;
	  timezone += 1;
          return 0;],
        [have_extern_timezone=yes
         AC_DEFINE(HAVE_EXTERN_TIMEZONE, 1, "External timezone")
         AC_MSG_RESULT(yes)],
         AC_MSG_RESULT(no))

	 
dnl Update version and date.                           
AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

RESOURCE_VERSION=workrave_resource_version
AC_SUBST(RESOURCE_VERSION)

AC_CANONICAL_HOST
AC_PREFIX_PROGRAM(workrave)

dnl
dnl Checking target platform.
dnl
AC_MSG_CHECKING([for platform])

platform_os_win32=no
platform_os_osx=no
platform_os_unix=no
platform_variant_x11=no
platform_variant_gnome=no

case "$target_or_host" in
  *-*mingw*|*-*-mingw*|*-*-cygwin*)
    platform_os_win32=yes
    platform_os_name="Windows"
   AC_DEFINE(PLATFORM_OS_WIN32,,[Define if compiling for the Win32 platform])
   AC_DEFINE(HAVE_LANGUAGE_SELECTION,,[Define if the UI language can be defined in the preferences])
    ;;
  *-*-darwin* | *-*-rhapsody*)
    platform_os_osx=yes
    platform_os_name="OS X"
   AC_DEFINE(PLATFORM_OS_OSX,,[Define if compiling for the OS X platform])
    ;;
  *)
    platform_os_unix=yes
    platform_os_name="Unix"
   AC_DEFINE(PLATFORM_OS_UNIX,,[Define if compiling for a Unix platform])
    ;;
esac

AC_MSG_RESULT([$platform_os_name])

AM_CONDITIONAL(PLATFORM_OS_WIN32, test "$platform_os_win32" = "yes")
AM_CONDITIONAL(PLATFORM_OS_OSX, test "$platform_os_osx" = "yes")
AM_CONDITIONAL(PLATFORM_OS_UNIX, test "$platform_os_unix" = "yes")

AM_CONDITIONAL(HAVE_LANGUAGE_SELECTION, test "$platform_os_win32" = "yes")

dnl
dnl Debug
dnl

config_debug=no

if test "x$enable_debug" = "xyes"
then
   config_debug=yes
   CFLAGS="${CFLAGS} -O0 -g -Wall"
   CXXFLAGS="${CXXFLAGS} -O0 -g -Wall"
   LDFLAGS="${LDFLAGS} -g"

   AC_DEFINE([GLIBCXX_FORCE_NEW], , [Define for debugging purposes])
   AC_DEFINE([GLIBCPP_FORCE_NEW], , [Define for debugging purposes])
else
   AC_DEFINE([NDEBUG], , [Define if debugging code should be disabled])
fi

if test "x$enable_tracing" = "xyes"
then
   AC_DEFINE([TRACING], , [Define to enable method call tracing])
fi


dnl
dnl Windows specific checks
dnl

have_dsound=no
LIBSTDCPP_LIBS=
WIN32CONSOLE=
DIRECTSOUNDLIBS=

if test "x$platform_os_win32" = "xyes"
then
    LIBSTDCPP_LIBS=-lstdc++

    AC_DEFINE([_WIN32_IE],    [0x0600], [Windows version])
    AC_DEFINE([_WIN32_WINNT], [0x0600], [Windows version])
    
    if test "x$enable_debug" != "xyes"
    then
        WIN32CONSOLE=-mwindows
    else
        WIN32CONSOLE=-mconsole
    fi

    config_gconf=no
    config_gnome=no
    config_xml=no
    config_indicator=no

   AC_CHECK_HEADER(dsound.h, [ AC_DEFINE(HAVE_DSOUND, 1, [Define if we have DirectSound])
                                        have_dsound=yes DIRECTSOUNDLIBS="-ldsound -luuid -ldxguid -ldxerr8"
                                     ])

   AC_TRY_COMPILE(
   [
#include <mmdeviceapi.h>
#include <endpointvolume.h>

static REFIID foo = __uuidof(IAudioEndpointVolume); 
   ], 
   [
      IMMDeviceEnumerator *device_enum = NULL;
   ],
   [ have_mmdeviceapi=yes
     AC_DEFINE(HAVE_MMDEVICEAPI_H, 1, [Define if we have mm device api])
     AC_MSG_RESULT(yes)
   ],
   AC_MSG_RESULT(no))

fi

AC_SUBST(LIBSTDCPP_LIBS)
AC_SUBST(WIN32CONSOLE)
AC_SUBST(DIRECTSOUNDLIBS)

AM_CONDITIONAL(HAVE_DSOUND, test "$have_dsound" = "yes")

dnl
dnl OSX specific checks
dnl

PKG_PROG_PKG_CONFIG

AC_SUBST(OSX_CFLAGS)

if test "$platform_os_osx" = "yes"
then
   PKG_CHECK_MODULES(IGE, ige-mac-integration >= 0.6)
   
   AC_CHECK_LIB(resolv, res_query)

   AC_CHECK_HEADER(CoreFoundation/CoreFoundation.h, [ AC_CHECK_HEADER(IOKit/IOKitLib.h,
                                     [  AC_DEFINE(HAVE_IOKIT, 1, [Define if we have IOKit])
                                        LIBS="$LIBS -framework IOKit -framework CoreFoundation"
                                     ])])

    config_gnome=no
    config_gconf=no

    LDFLAGS="$LDFLAGS -headerpad_max_install_names"
    LIBS="$LIBS -framework QuickTime -framework CoreServices -framework Carbon -framework Cocoa -framework Foundation"
    OSX_CFLAGS=""
fi

AC_SUBST(IGE_LIBS)
AC_SUBST(IGE_CFLAGS)

dnl
dnl Unix specific checks
dnl

if test "$platform_os_unix" = "yes"
then
    AC_PATH_X
    AC_PATH_XTRA

    if test x$have_x != xyes ; then
       AC_MSG_ERROR(X11 required on Unix platform)
    fi
    
    LIBS_save=$LIBS
    if test -n "$ac_x_libraries"
    then
       LIBS="$LIBS -L $ac_x_libraries"
    fi
    have_xrecord=no
    AC_CHECK_LIB(Xtst, XRecordEnableContext,
                       have_xrecord=yes X_LIBS="$X_LIBS -lXtst" AC_DEFINE(HAVE_XRECORD,,[Define if the RECORD extension is available]),
    )
    if test x$have_xrecord != xyes ; then
       AC_MSG_ERROR(X RECORD extension headers files required on Unix platform)
    fi

    PKG_CHECK_MODULES(X11SM, sm ice)
    LIBS=$LIBS_save
fi


dnl
dnl GLIB
dnl

PKG_CHECK_MODULES([GLIB],
                  [glib-2.0 >= 2.10 gmodule-2.0 gthread-2.0],
                  [AC_DEFINE(HAVE_GLIB, , [Define if GLib is available])])


dnl
dnl GTK+
dnl

config_gtk=no
config_gtk3=no

if test "x$enable_gtk" != "xno"
then
   if test "x$enable_experimental" = "xyes"; then
      PKG_CHECK_MODULES(GTK,
			glib-2.0 >= 2.28.0
			gio-2.0 >= 2.25.0
			gtk+-3.0 >= 3.0.0
			gdk-3.0 >= 3.0.0
                        sigc++-2.0 >= 2.2.4.2
                        glibmm-2.4 >= 2.28.0
                        gtkmm-3.0 >= 3.0.0,
                        [config_gtk3=yes
                         AC_DEFINE([HAVE_APP_GTK], , [Define if GTK+ is available])])
      if test x$config_gtk3 = xyes
      then
	  config_gtk=yes
	  AC_DEFINE(HAVE_GTK3, 1, Support for GTK3)
          CFLAGS="${CFLAGS} -DGTK_DISABLE_SINGLE_INCLUDES_XX  -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE"
          CXXFLAGS="${CXXFLAGS} -DGTK_DISABLE_SINGLE_INCLUDES_XX  -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE"
      fi
   fi

   if test x$config_gtk3 = xno
   then
      PKG_CHECK_MODULES(GTK,
                        gtk+-2.0 >= 2.6.0 \
                        gdk-2.0 >= 2.8 \
                        sigc++-2.0 \
                        glibmm-2.4 >= 2.10 \
                        gtkmm-2.4 >= 2.10,
                        [config_gtk=yes
                         AC_DEFINE([HAVE_APP_GTK], , [Define if GTK+ is available])])
fi
fi

AM_CONDITIONAL(HAVE_APP_GTKMM, test $config_gtk = yes)
AM_CONDITIONAL(HAVE_GTK3, test "x$config_gtk3" = "xyes")


dnl
dnl Gnome
dnl

config_gnome=no
config_gnome3=no
config_panelapplet4=no

if test "x$enable_gnome3" != "xno"
then
    PKG_CHECK_MODULES([GNOME],
                      [libpanelapplet-4.0],
                      [config_gnome=yes
		       config_gnome3=yes
                       config_panelapplet4=yes
                       AC_DEFINE(HAVE_PANELAPPLET4, 1, [Support for Gnome panel applet v4])],
                       [if test "x$enable_gnome3" = "xyes"; then
                           AC_MSG_ERROR([Gnome 3 development headers not found.])
                       fi])
fi
   
if test "x$enable_gnome" != "xno" -a x$config_panelapplet4 = xno -a x$config_gtk3 = xno
then
    PKG_CHECK_MODULES([GNOME],
                      [libpanelapplet-2.0 >= 2.0.10 \
                       libbonobo-2.0 >= 2.0.0],
                      [config_gnome=yes],
                      [if test "x$enable_gnome" = "xyes"; then
                          AC_MSG_ERROR([Gnome development headers not found.])
                       fi])
fi
   
if test $config_gnome = yes
then
    AC_DEFINE([HAVE_GNOME], , [Define if Gnome is available])
fi

AM_CONDITIONAL(HAVE_GNOME, test "x$config_gnome" = "xyes")
AM_CONDITIONAL(HAVE_PANELAPPLET4, test "x$config_panelapplet4" = "xyes")

dnl
dnl Indicator applet
dnl


INDICATOR_REQUIRED_VERSION=0.3.19
DBUSMENUGLIB_REQUIRED_VERSION=0.1.1
DBUSMENUGTK_REQUIRED_VERSION=0.3.95

if test "x$enable_indicator" != "xno" -a "x$config_gtk3" = "xyes"
then
          PKG_CHECK_MODULES(INDICATOR, indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                        dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                        dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION,
                              [ config_indicator=yes
                       	        AC_DEFINE(HAVE_INDICATOR, 1, [Support for indicator applet])])
fi

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

AS_IF([test "x$with_localinstall" = "xyes"],
		INDICATORDIR="${libdir}/indicators3/2/"
		INDICATORICONSDIR="${datadir}/libindicate/icons/"
		,
		INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
		INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`
		)

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)
	
AM_CONDITIONAL(HAVE_INDICATOR, test "x$config_indicator" = "xyes")

dnl
dnl GConf
dnl

config_gconf=no

if test "x$enable_gconf" != "xno"
then
  PKG_CHECK_MODULES([GCONF],
                    [gconf-2.0 >= 2.31.1],
                    [config_gconf=yes
                     AC_DEFINE([HAVE_GCONF], , [Define if GConf is available])],
                    [if test "x$enable_gconf" = "xyes"; then
                       AC_MSG_ERROR([GConf development headers not found.])
                     fi])
fi

AM_CONDITIONAL(HAVE_GCONF, test "x$config_gconf" = "xyes")


dnl
dnl GIO/GNet2 networking
dnl

config_distribution=no
have_gnet=no

if test "x$enable_distribution" != "xno"
then
  PKG_CHECK_MODULES([GIO],
                    [gio-2.0 >= 2.22.0],
                    [config_distribution=yes
                     AC_DEFINE(HAVE_GIO_NET, 1, [Have GIO networking support])],
                    [PKG_CHECK_MODULES([GNET],
                                       [gnet-2.0],
                                       [config_distribution=yes
                                        have_gnet=yes
                                        AC_DEFINE([HAVE_GNET], , [Define if GNet is available])
                                        AC_DEFINE([HAVE_GNET2], , [Define if GNet2 is available])],
                                       [if test "x$enable_distribution" = "xyes"; then
                                          AC_MSG_ERROR([GIO/GNet development headers not found.])
                                        fi])])
fi

if test $config_distribution = yes
then
  AC_DEFINE([HAVE_DISTRIBUTION], , [Define if network-distributed operation is available])
fi

AM_CONDITIONAL(HAVE_DISTRIBUTION, test "x$config_distribution" = "xyes")
AM_CONDITIONAL(HAVE_GNET, test $have_gnet = yes)


dnl
dnl XML
dnl

config_xml=no

if test "x$enable_xml" != "xno"
then
    PKG_CHECK_MODULES([GDOME],
                      [gdome2],
                      [config_xml=yes
                       AC_DEFINE([HAVE_GDOME], ,[Define if GDome is available])],
                      [if test "x$enable_xml" = "xyes"; then
                         AC_MSG_ERROR([GDome2 development headers not found.])
                       fi])
fi

AM_CONDITIONAL(HAVE_GDOME, test "x$config_xml" = "xyes")


dnl
dnl DBus
dnl
            
have_dbus=no
have_python_cheetah=no

if test "x$enable_dbus" != "xno"
then
    AS_AC_EXPAND(DATADIR, "$datadir")
    DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
    AC_SUBST(DBUS_SERVICES_DIR)

    if test "x$platform_os_win32" = "xyes"
    then
	PKG_CHECK_MODULES([DBUS],
                          [dbus-1 >= 1.2],
                          [have_dbus=yes],
                          [true])
    else
	PKG_CHECK_MODULES([DBUS],
                          [dbus-1 >= 1.2],
                          [PKG_CHECK_MODULES([DBUSGLIB], [dbus-glib-1], [have_dbus=yes], [true])
                           PKG_CHECK_EXISTS([dbus-glib-1 >= 0.84],
                                         [AC_DEFINE(HAVE_DBUSGLIB_GET_PRIVATE, , [DBus-glib 0.84+ support])])],
                          [true])

        if test "x$have_dbus" != "xyes" -a "x$enable_dbus" = "xyes"; then
            AC_MSG_ERROR([DBus and DBus-glib development headers not found.])
        fi
    fi
    
	 
    if test "x$have_dbus" = "xyes"
    then
        AC_DEFINE(HAVE_DBUS, , [DBus support])
	if test "x$platform_os_win32" != "xyes"
	then
            AC_DEFINE(HAVE_DBUSGLIB, , [DBus-glib support])
	fi
        AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for DBUS is])
    fi

    AC_CHECK_PROG(PYTHON, python, python)
      AC_MSG_CHECKING([for the Cheetah Python package])
        ac_cheetah_result=`$PYTHON -c "from Cheetah.Template import Template" 2>&1`
        if test -z "$ac_cheetah_result"; then
           AC_MSG_RESULT([yes])
        have_python_cheetah=yes
        else
        AC_MSG_RESULT([no, dbusgen rules will be disabled])
  fi
fi

AM_CONDITIONAL(HAVE_DBUS, test x$have_dbus = xyes)
AM_CONDITIONAL(HAVE_PYTHON_CHEETAH, test x$have_python_cheetah = xyes)


dnl 
dnl GStreamer
dnl

config_gstreamer=no

if test "x$enable_gstreamer" != "xno";
then
    PKG_CHECK_MODULES([GSTREAMER],
                      [gstreamer-0.10],
                      [config_gstreamer=yes
                       AC_DEFINE(HAVE_GSTREAMER, 1, [Use GStreamer for playing sounds])],
                      [if test "x$enable_gstreamer" = "xyes"; then
                           AC_MSG_ERROR([GStreamer development headers not found.])
                       fi])
fi


dnl
dnl PulseAudio
dnl

have_pulse=no

if test "$platform_os_unix" = "yes" -a "x$enable_pulse" != "xno"
then
           PA_REQUIRED_VERSION=0.9.15
    PKG_CHECK_MODULES([PULSE],
                      [glib-2.0 libpulse >= $PA_REQUIRED_VERSION libpulse-mainloop-glib >= $PA_REQUIRED_VERSION],
                      [have_pulse=yes
                  AC_DEFINE(HAVE_PULSE, 1, [Define if PULSE sound server should be used])],
                      [if test "x$enable_pulse" = "xyes"; then
                           AC_MSG_ERROR([Pulseaudio development headers not found.])
                       fi])
fi

AM_CONDITIONAL(HAVE_PULSE, test "x$have_pulse" = "xyes")


dnl
dnl Internationalization
dnl

AM_GNU_GETTEXT()

dnl Hack because windows gettext package is SEVERELY broken...
if test "x$platform_os_win32" = "xyes"
then
        USE_NLS=yes
        AC_DEFINE(ENABLE_NLS,1,[Define to 1 if translation of program messages to the user's native language is requested.])
        AC_DEFINE(USE_INCLUDED_LIBINTL, "no", [Dont])
        AC_DEFINE(BUILD_INCLUDED_LIBINTL, "no", [Dont])
        USE_INCLUDED_LIBINTL=no
        BUILD_INCLUDED_LIBINTL=no
fi

GETTEXT_PACKAGE=workrave
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE(GETTEXT_PACKAGE, "workrave", [Blaat])
AC_PROG_INTLTOOL


dnl
dnl  Manual
dnl
dnl (taken from clutter)
dnl

AC_ARG_ENABLE(manual,
              AS_HELP_STRING([--enable-manual],
                             [Build user manual. Requires jw and xmlto binaries.]))

if test "x$enable_manual" = "xyes"; then
  AC_PATH_PROG(JW, jw, no)
  if test "x$JW" = "xno"; then
    AC_MSG_ERROR(['jw' program needed by the manual not found in path])
  fi

  AC_PATH_PROG(XMLTO, xmlto, no)
  if test "x$XMLTO" = "xno"; then
    AC_MSG_ERROR(['xmlto' program needed by the manual not found in path])
  fi
fi

AM_CONDITIONAL(ENABLE_MANUAL, test "x$enable_manual" = "xyes")


dnl
dnl What variant to build
dnl
    
variant=""

if test "x$platform_os_win32" = "xyes"
then
   variant="$variant win32"
elif test "x$platform_os_osx" = "xyes"
then
   variant="$variant osx unix"
elif test "x$platform_os_unix" = "xyes"
then
   variant="$variant x11 unix"
fi

if test "x$config_gnome" = "xyes"
then
   variant="$variant gnome"
fi

WR_BACKEND_INCLUDES="-I\$(top_srcdir)/backend/include"
WR_COMMON_INCLUDES="-I\$(top_srcdir)/common/include"
WR_FRONTEND_COMMON_INCLUDES="-I\$(top_srcdir)/frontend/common/include"

for v in $variant; do
    if test -d common/src/$v
    then
       WR_COMMON_INCLUDES="$WR_COMMON_INCLUDES -I\$(top_srcdir)/common/include/$v"
    fi
done

WR_LDADD="-lworkrave-backend -lworkrave-frontend-common -lworkrave-common"
WR_LDFLAGS="-L\$(top_builddir)/common/src/ -L\$(top_builddir)/backend/src/ -L\$(top_builddir)/frontend/common/src"

AC_SUBST(WR_LDADD)
AC_SUBST(WR_LDFLAGS)

AC_SUBST(WR_FRONTEND_COMMON_INCLUDES)
AC_SUBST(WR_COMMON_INCLUDES)
AC_SUBST(WR_BACKEND_INCLUDES)


dnl
dnl Defines
dnl

config_tests=no
if test "x$enable_tests" = "xyes"; then
   config_tests=yes
   AC_DEFINE([HAVE_TESTS], , [Define if test code is enabled])
fi
AM_CONDITIONAL(HAVE_TESTS, test "x$config_tests" = "xyes")

config_gnomeapplet=no
if test "x$config_gnome" = "xyes" -a "x$have_dbus" = "xyes" ; then
   config_gnomeapplet=yes
   AC_DEFINE(HAVE_GNOMEAPPLET,,[Define if Gnome Applet is available])
fi
AM_CONDITIONAL(HAVE_GNOMEAPPLET, test "x$config_gnomeapplet" = "xyes")

config_exercises=no
if test "x$enable_exercises" != "xno"; then
   config_exercises=yes
   AC_DEFINE(HAVE_EXERCISES,,[Define if exercises are available])
fi
AM_CONDITIONAL(HAVE_EXERCISES, test "x$config_exercises" = "xyes")

config_experimental=no
if test "x$enable_experimental" = "xyes"; then
   config_experimental=yes
   AC_DEFINE(HAVE_EXPERIMENTAL,,[Define if experimental features are available])
fi
AM_CONDITIONAL(HAVE_EXPERIMENTAL, test "x$config_experimental" = "xyes")

config_text=no
if test "x$enable_app_text" = "xyes"; then
   config_text=yes
   AC_DEFINE(HAVE_APP_TEXT,,[Define if Text is available])
fi
AM_CONDITIONAL(HAVE_APP_TEXT, test "x$config_text" = "xyes")

if test "x$enable_x11_monitoring_fallback" = "xyes"; then
   AC_DEFINE(HAVE_XRECORD_FALLBACK,,[Define if RECORD fallback is enabled])
fi


dnl
dnl Generate language list for InoSetup installer
dnl

if test "x$platform_os_win32" = "xyes"
then
    LANGFILE="./frontend/gtkmm/win32/setup/lang.iss"

    echo -n "; language.iss --- Inno setup file, support languages" > $LANGFILE
    echo -en "\r\n" >> $LANGFILE
    for a in $ALL_LINGUAS; do
        X="Source: \"..\\..\\..\\..\\po\\$a.gmo\"; DestDir: \"{app}\\lib\\locale\\$a\\LC_MESSAGES\"; DestName: \"workrave.mo\"; Flags: ignoreversion;"
        echo -n $X     >> $LANGFILE
        echo -en "\r\n" >> $LANGFILE
    done
fi

dnl
dnl Makefile output
dnl

AC_OUTPUT(Makefile 
          backend/Makefile \
          backend/test/Makefile \
          backend/src/Makefile \
          backend/src/unix/Makefile \
          backend/src/osx/Makefile \
          backend/src/win32/Makefile \
          backend/include/Makefile \
          common/Makefile \
          common/bin/Makefile \
          common/src/Makefile \
          common/src/win32/Makefile \
          common/include/Makefile \
          common/include/win32/Makefile \
          common/win32/Makefile \
          common/win32/harpoon/Makefile \
          common/win32/harpoon/include/Makefile \
          common/win32/harpoon/src/Makefile \
          common/win32/harpoonHelper/Makefile \
          common/win32/harpoonHelper/include/Makefile \
          common/win32/harpoonHelper/src/Makefile \
          frontend/Makefile \
          frontend/common/Makefile \
          frontend/common/include/Makefile \
          frontend/common/share/Makefile \
          frontend/common/share/sounds/Makefile \
          frontend/common/share/sounds/subtle/Makefile \
          frontend/common/share/sounds/default/Makefile \
          frontend/common/share/images/Makefile \
          frontend/common/share/images/win32/Makefile \
          frontend/common/share/images/osx/Makefile \
          frontend/common/src/Makefile \
          frontend/common/src/win32/Makefile \
          frontend/common/src/x11/Makefile \
          frontend/common/src/osx/Makefile \
          frontend/win32/Makefile \
          frontend/win32/applet/Makefile \
          frontend/win32/applet/src/Makefile \
          frontend/win32/applet/include/Makefile \
          frontend/gtkmm/Makefile \
          frontend/gtkmm/osx/Makefile \
          frontend/gtkmm/win32/Makefile \
          frontend/gtkmm/win32/setup/Makefile \
          frontend/gtkmm/win32/setup/setup.iss \
          frontend/gtkmm/win32/setup/src/Makefile \
          frontend/gtkmm/src/Makefile \
          frontend/gtkmm/src/win32/Makefile \
          frontend/gtkmm/src/unix/Makefile \
          frontend/gtkmm/src/osx/Makefile \
          frontend/gtkmm/src/gnome_applet/Makefile \
          frontend/gtkmm/src/gnome3_applet/Makefile \
          frontend/gtkmm/src/indicator_applet/Makefile \
          frontend/text/Makefile \
          frontend/text/src/Makefile \
          frontend/plugin/Makefile \
          frontend/plugin/distribution/Makefile \
          frontend/plugin/distribution/gtkmm/Makefile \
          frontend/plugin/distribution/gtkmm/src/Makefile \
          frontend/plugin/statistics/Makefile \
          frontend/plugin/statistics/gtkmm/Makefile \
          frontend/plugin/statistics/gtkmm/src/Makefile \
          frontend/plugin/exercises/Makefile \
          frontend/plugin/exercises/common/Makefile \
          frontend/plugin/exercises/common/share/Makefile \
          frontend/plugin/exercises/common/src/Makefile \
          frontend/plugin/exercises/gtkmm/Makefile \
          frontend/plugin/exercises/gtkmm/src/Makefile \
          intl/Makefile \
          po/Makefile.in \
          contrib/Makefile \
          contrib/plot/Makefile \
          build/Makefile \
          m4/Makefile \
          workrave.spec)



dnl ========================================================================

echo ""
echo "                        Workrave $VERSION"
echo "                       ==================="
echo ""
echo "                        prefix:   ${prefix}"
echo ""
echo "                           OS :   ${platform_os_name}"
echo ""
echo "          GConf configuration :   ${config_gconf}"
echo "            XML configuration :   ${config_xml}"
echo ""
if test "${config_gtk3}" = "yes"; then
echo "                      Gtk GUI :   ${config_gtk} (Gtk3)"
else
echo "                      Gtk GUI :   ${config_gtk} (Gtk2)"
fi
if test "${config_gnome3}" = "yes"; then
echo "                Gnome support :   Gnome 3"
elif test "${config_gnome}" = "yes"; then
echo "                Gnome support :   Gnome 2"
else
echo "                Gnome support :   No"
fi
echo "            Indicator support :   ${config_indicator}"
echo "                 DBUS support :   ${have_dbus}"
echo "           Networking support :   ${config_distribution}"
echo "            GStreamer support :   ${config_gstreamer}"
if test "$platform_os_unix" = "yes"; then
echo "           Pulseaudio support :   ${have_pulse}"
fi
echo "                    Exercises :   ${config_exercises}"
echo ""
echo "        Experimental features :   ${config_experimental}"
echo "                    Debugging :   ${config_debug}"
echo "                    Test code :   ${config_tests}"
echo ""

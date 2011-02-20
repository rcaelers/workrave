dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
dnl
dnl
dnl $Id$
dnl

m4_define([workrave_major_version], [1])
m4_define([workrave_minor_version], [9])
m4_define([workrave_micro_version], [3])
m4_define([workrave_version_suffix], [99])
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
AM_INIT_AUTOMAKE(workrave, 1.9.4-rc1) # workrave_version)
AM_GNU_GETTEXT_VERSION()
AM_MAINTAINER_MODE

dnl Set of available languages.
AC_SUBST(ALL_LINGUAS)
ALL_LINGUAS="nl de eo pl da es zh_TW ru fr pt_BR nb lt cs he eu ca sv ja bg hu tr el sk zh_CN it sl ro et te ga ko"
AC_DEFINE_UNQUOTED(ALL_LINGUAS, "$ALL_LINGUAS" ,[All available languages])

dnl
dnl Command line options.                           
dnl

AC_ARG_ENABLE(app-gtk,
              [AC_HELP_STRING([--disable-app-gtk],
                              [compile without Gtk support (NOT recommended)])],
              [config_gtk=$enableval], [config_gtk=yes])

AC_ARG_ENABLE(app-text,
              [AC_HELP_STRING([--enable-app-text],
                              [compile with dummy text GUI support (NOT recommended)])],
              [config_text=$enableval], [config_text=no])
    
AC_ARG_ENABLE(xml,
              [AC_HELP_STRING([--disable-xml],
                              [compile without XML support])],
              [config_xml=$enableval], [config_xml=yes])

AC_ARG_ENABLE(gconf,
              [AC_HELP_STRING([--disable-gconf],
                              [compile without Gnome gconf2 support])],
              [config_gconf=$enableval], [config_gconf=yes])

AC_ARG_ENABLE( gnome,
              [AC_HELP_STRING([--disable-gnome],
                              [compile without Gnome support])],
              [config_gnome=$enableval], [config_gnome=yes])

AC_ARG_ENABLE( dbus,
              [AC_HELP_STRING([--disable-dbus],
                              [compile without DBus support])],
              [config_dbus=$enableval], [config_dbus=yes])
 
AC_ARG_ENABLE( gstreamer,
    	      [AC_HELP_STRING([--disable-gstreamer],
	                      [compile without GStreamer audio support])],
   	      [config_gst="$enableval"], [enable_gst="yes"])

AC_ARG_ENABLE( kde,
              [AC_HELP_STRING([--enable-kde],
                              [compile with KDE support (broken)])],
              [config_kde=$enableval], [config_kde=no])

AC_ARG_ENABLE( x11-monitoring-fallback,
              [AC_HELP_STRING([--enable-x11-monitoring-fallback],
                              [compile with fallback monitoring if RECORD is not available (discouraged)])],
              [config_fallback=$enableval], [config_fallback=no])
	      
AC_ARG_ENABLE( debug,
              [AC_HELP_STRING([--enable-debug],
                              [compile with debugging flags set])],
              [config_debug=$enableval], [config_debug=no])

AC_ARG_ENABLE( distribution,
              [AC_HELP_STRING([--disable-distribution],
                              [compile without support for distributed operation])],
              [config_distribution=$enableval], [config_distribution=yes])

AC_ARG_ENABLE( experimental,
              [AC_HELP_STRING([--enable-experimental],
                              [compile with experimental features])],
              [config_experimental=$enableval], [config_experimental=yes])
	      
AC_ARG_ENABLE( exercises,
              [AC_HELP_STRING([--disable-exercises],
                              [compile without exercises support])],
              [config_exercises=$enableval], [config_exercises=yes])

AC_ARG_ENABLE( tests,
              [AC_HELP_STRING([--enable-tests],
                              [compile with test code])],
              [config_tests=$enableval], [config_tests=no])

AC_ARG_ENABLE( tracing,
              [AC_HELP_STRING([--enable-tracing],
                              [compile with method call tracing])],
              [config_tracing=$enableval], [config_tracing=no])

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
platform_variant_kde=no

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

if test "x$config_debug" = "xyes"
then
   CFLAGS="${CFLAGS} -O0 -g -Wall -DGLIBCXX_FORCE_NEW -DGLIBCPP_FORCE_NEW"
   CXXFLAGS="${CXXFLAGS} -O0 -g -Wall -DGLIBCXX_FORCE_NEW -DGLIBCPP_FORCE_NEW"
   LDFLAGS="${LDFLAGS} -g"
   config_tracing=yes
else
   OPT=2

   CFLAGS="${CFLAGS} -O${OPT} -g -DNDEBUG"
   CXXFLAGS="${CXXFLAGS} -O${OPT} -g -DNDEBUG"
   LDFLAGS="${LDFLAGS} -g"
fi

if test "x$config_tracing" = "xyes"
then
   CFLAGS="${CFLAGS} -DTRACING"
   CXXFLAGS="${CXXFLAGS} -DTRACING"
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

   CFLAGS="${CFLAGS} -D_WIN32_IE=0x0600 -D_WIN32_WINNT=0x0600"
   CXXFLAGS="${CXXFLAGS} -D_WIN32_IE=0x0600 -D_WIN32_WINNT=0x0600"
    
    if test "x$config_debug" != "xyes"
    then
        WIN32CONSOLE=-mwindows
    else
        WIN32CONSOLE=-mconsole
    fi

    config_gconf=no
    config_gnome=no
    config_xml=no

   AC_CHECK_HEADER(dsound.h, [ AC_DEFINE(HAVE_DSOUND, 1, [Define if we have DirectSound])
                                        have_dsound=yes DIRECTSOUNDLIBS="-ldsound -luuid -ldxguid -ldxerr8"
                                     ])

   AC_CHECK_HEADERS(mmdeviceapi.h endpointvolume.h, [ AC_DEFINE(HAVE_MMDEVICEAPI_H, 1, [Define if we have mm device api])
                                        have_mmdeviceapi=yes
                                     ])
				     
#ifdef HAVE_MMDEVICEAPI_H
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#else
				     
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

PKG_CHECK_MODULES(GLIB,
                glib-2.0 >= 2.10 \
                gmodule-2.0 \
                gthread-2.0)

AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

dnl
dnl GTK
dnl

if test "x$config_gtk" = "xyes"
then
    PKG_CHECK_MODULES(GTK,
                      gtk+-2.0 >= 2.6.0 \
                      gdk-2.0 >= 2.8 \
                      sigc++-2.0 \
                      glibmm-2.4 >= 2.10 \
                      gtkmm-2.4 >= 2.10)
fi

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)


dnl
dnl GNome
dnl

if test "x$config_gnome" = "xyes";
then
   have_panelapplet3=no
   
   if test "x$config_experimental" = "xyes"; then
      PKG_CHECK_MODULES(GNOME, libpanelapplet-3.0, have_panelapplet3=yes, have_panelapplet3=no)
      if test x$have_panelapplet3 = xyes
      then
	  AC_DEFINE(HAVE_PANELAPPLET3, 1, Support for Gnome panel applet V3)
      fi
   fi
   
   if test x$have_panelapplet3 = xno
   then
      PKG_CHECK_MODULES(GNOME,
                      libpanelapplet-2.0 >= 2.0.10 \
                      libbonobo-2.0 >= 2.0.0)
   fi
fi

AM_CONDITIONAL(HAVE_PANELAPPLET3, test "x$have_panelapplet3" = "xyes")

AC_SUBST(GNOME_LIBS)
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(IDL_INCLUDES)
   

dnl
dnl KDE
dnl
  
KDE_MISC_TESTS
if test "x$config_kde" = "xyes";
then
    AC_BASE_PATH_KDE

    dnl for NLS support. Call them in this order!
    dnl WITH_NLS is for the po files
    dnl AM_KDE_WITH_NLS
    dnl KDE_PROG_LIBTOOL

    KDE_USE_QT(3.0.0)
    AC_PATH_KDE

    dnl PACKAGE set before
    AC_CHECK_KDEMAXPATHLEN

    KDE_CREATE_SUBDIRSLIST
else
    build_arts="no"
    include_ARTS_TRUE='#'
    include_ARTS_FALSE=
fi

  
dnl
dnl GCONF
dnl

if test "x$config_gconf" = "xyes"
then
  PKG_CHECK_MODULES(GCONF, gconf-2.0)
fi
                                
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

dnl
dnl GIO networking
dnl

have_gio_net=no
AC_MSG_CHECKING([GIO networking support])

CFLAGS_save=$LIBS
CFLAGS="$CFLAGS $GLIB_CFLAGS"
CXXFLAGS="$CXXFLAGS $GLIB_CFLAGS"

AC_TRY_COMPILE([#include <gio/gio.h>], 
        [ GSocketService *socket = g_socket_service_new();
	],
        [have_gio_net=yes
         AC_DEFINE(HAVE_GIO_NET, 1, "Have GIO networking support")
         AC_MSG_RESULT(yes)],
         AC_MSG_RESULT(no))

CLFAGS=$CFLAGS_save
CXXLFAGS=$CXXFLAGS_save

dnl
dnl GNet2
dnl

if test "x$config_distribution" = "xyes" -a $have_gio_net != yes
then
    PKG_CHECK_MODULES(GNET, gnet-2.0)
    GNET2=yes                          
fi
AC_SUBST(GNET_LIBS)
AC_SUBST(GNET_CFLAGS)



dnl
dnl XML
dnl

if test "x$config_xml" = "xyes"
then
    AC_PATH_PROG(GDOME_CONFIG, gdome-config, no)
    if test "$GDOME_CONFIG" = no
    then
    	if test x$2 = xfailure
        then
    		AC_MSG_ERROR(Could not find gdome-config)
    	fi
        AC_MSG_ERROR(Need GDome2)
    fi
    GDOME_CFLAGS=`$GDOME_CONFIG --cflags`
    GDOME_LIB=`$GDOME_CONFIG --libs`    
else
    GDOME_LIB="";
    GDOME_CFLAGS=""
fi
AC_SUBST(GDOME_CFLAGS)
AC_SUBST(GDOME_LIB)


dnl
dnl DBus
dnl
            
if test "x$config_dbus" != "xno"
then
   dbus_pkgconfig="dbus-1 >= 1.2"

   PKG_CHECK_MODULES(DBUS, $dbus_pkgconfig, have_dbus=yes, have_dbus=no)
   if test x$have_dbus = xyes
   then
      AC_DEFINE(HAVE_DBUS, 1, DBus support)

      PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1 >= 0.84, have_dbus_glib=yes, have_dbus_glib=no)
      if test x$have_dbus_glib = xyes; then
         AC_DEFINE(HAVE_DBUSGLIB, 1, DBus-glib support)
         AC_DEFINE(HAVE_DBUSGLIB_GET_PRIVATE, 1, DBus-glib support)
      else
         PKG_CHECK_MODULES(DBUSGLIB, dbus-glib-1, have_dbus_glib=yes, have_dbus_glib=no)
         if test x$have_dbus_glib = xyes; then
            AC_DEFINE(HAVE_DBUSGLIB, 1, DBus-glib support)
         fi
      fi
	 
      AS_AC_EXPAND(DATADIR, $datadir)

      DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
      AC_SUBST(DBUS_SERVICES_DIR)
      AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for DBUS is])

      AC_CHECK_PROG(PYTHON,python,python)
      
      AC_MSG_CHECKING([for the Cheetah Python package])
        ac_cheetah_result=`$PYTHON -c "from Cheetah.Template import Template" 2>&1`
        if test -z "$ac_cheetah_result"; then
           AC_MSG_RESULT([yes])
        else
           AC_MSG_RESULT([no])
           AC_MSG_ERROR([cannot import Python module "cheetah"])
	   have_dbus=no
	   have_dbus_glib=no
           dbus_pkgconfig=
        fi
   else
     AC_MSG_ERROR([
DBus development headers not found.
Use --disable-dbus if you do not need DBus support.
])
      have_dbus=no
      have_dbus_glib=no
      dbus_pkgconfig=
   fi
else
   have_dbus=no
   have_dbus_glib=no
   dbus_pkgconfig=
fi

if test "x$have_dbus" != "xyes"
then
  if test "x$enable_dbus" = "xyes"
  then
      AC_MSG_ERROR([Couldn't find d-bus.])
  else
   dbus_pkgconfig=
  fi
fi

AM_CONDITIONAL(HAVE_DBUS, test x$have_dbus = xyes)

dnl 
dnl GStreamer
dnl

if test "x$config_gst" != "xno";
then
    PKG_CHECK_MODULES(GSTREAMER, [gstreamer-0.10], [
		AC_DEFINE(HAVE_GSTREAMER, 1, [Use GStreamer for playing sounds])
		AC_SUBST(GSTREAMER_CFLAGS)
		AC_SUBST(GSTREAMER_LIBS)
	      ], [
		AC_MSG_RESULT(no)
		AC_MSG_ERROR([
GStreamer development headers not found.
Use --disable-gstreamer if you do not need GStreamer (sound) support.
])])
fi


dnl
dnl PulseAudio
dnl

have_pulse=false

if test "$platform_os_unix" = "yes"
then
    have_pulse=false
    AC_ARG_ENABLE(pulse,
      AC_HELP_STRING([--disable-pulse],
                     [turn off PulseAudio support]),
           [case "${enableval}" in
                   yes) WANT_PULSE=yes ;;
                   no)  WANT_PULSE=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --disable-pulse) ;;
           esac],
           [WANT_PULSE=yes]) dnl Default value

    if test x$WANT_PULSE = xyes ; then
           PA_REQUIRED_VERSION=0.9.15
           PKG_CHECK_MODULES(PULSE, glib-2.0 libpulse >= $PA_REQUIRED_VERSION libpulse-mainloop-glib >= $PA_REQUIRED_VERSION,
                 [have_pulse=true
                  AC_DEFINE(HAVE_PULSE, 1, [Define if PULSE sound server should be used])],
                 [AC_MSG_ERROR([
Pulseaudio development headers not found.
Use --disable-pulse if you do not need Pulseaudio (mute) support.
])])
    fi
    AC_SUBST(PULSE_CFLAGS)
    AC_SUBST(PULSE_LIBS)
fi

AM_CONDITIONAL(HAVE_PULSE, test "x$have_pulse" = "xtrue")



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
              AC_HELP_STRING([--enable-manual=@<:@no/yes@:>@],
                             [Build user manual. Requires jw and xmlto binaries.]),
              enable_manual=$enableval, enable_manual=no)

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

AM_CONDITIONAL(ENABLE_MANUAL, test "x$enable_manual" != "xno")


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
if test "x$config_kde" = "xyes"
then
   variant="$variant kde"
fi

WR_BACKEND_INCLUDES="-I \$(top_srcdir)/backend/include"
WR_COMMON_INCLUDES="-I \$(top_srcdir)/common/include"
WR_FRONTEND_COMMON_INCLUDES="-I \$(top_srcdir)/frontend/common/include"

for v in $variant; do
    if test -d common/src/$v
    then
       WR_COMMON_INCLUDES="$WR_COMMON_INCLUDES -I \$(top_srcdir)/common/include/$v"
    fi
done

WR_LDADD="-lworkrave-backend -lworkrave-frontend-common -lworkrave-common"
WR_LDFLAGS="-L\$(top_builddir)/common/src/ -L\$(top_builddir)/backend/src/ -L\$(top_builddir)/frontend/common/src"

AC_SUBST(WR_LDADD)
AC_SUBST(WR_LDFLAGS)

AC_SUBST(WR_FRONTEND_COMMON_INCLUDES)
AC_SUBST(WR_COMMON_INCLUDES)
AC_SUBST(WR_BACKEND_INCLUDES)

AC_SUBST(include_ARTS)

dnl
dnl Defines
dnl

if test "x$config_tests" = "xyes"; then
   config_tests=yes
fi

HAVE_GCONF=
if test "x$GCONF_LIBS" != "x" -a "x$config_gconf" = "xyes"; then
   AC_DEFINE(HAVE_GCONF,,[Define if GConf is available])
fi

HAVE_GDOME=
if test "x$GDOME_LIB" != "x" -a "x$config_xml" = "xyes"; then
   AC_DEFINE(HAVE_GDOME,,[Define if GDome is available])
fi

HAVE_GLIB=
if test "x$GLIB_LIBS" != "x"; then
   AC_DEFINE(HAVE_GLIB,,[Define if glib is available])
fi

HAVE_GNOME=
if test "x$config_gnome" = "xyes"; then
   AC_DEFINE(HAVE_GNOME,,[Define if Gnome is available])
fi

HAVE_GNOMEAPPLET=
if test "x$config_gnome" = "xyes" -a "x$have_dbus" = "xyes" -a "x$have_dbus_glib" = "xyes"; then
   AC_DEFINE(HAVE_GNOMEAPPLET,,[Define if Gnome Applet is available])
fi

HAVE_KDE=
if test "x$config_kde" = "xyes"; then
   AC_DEFINE(HAVE_KDE,,[Define if KDE is available])
fi

HAVE_DISTRIBUTION=
if test "x$config_distribution" = "xyes"; then
   AC_DEFINE(HAVE_DISTRIBUTION,,[Define if distributed operation is available])
fi

HAVE_EXERCISES=
if test "x$config_exercises" = "xyes"; then
   AC_DEFINE(HAVE_EXERCISES,,[Define if exercises are available])
fi

HAVE_EXPERIMENTAL=
if test "x$config_experimental" = "xyes"; then
   AC_DEFINE(HAVE_EXPERIMENTAL,,[Define if experimental features are available])
fi

HAVE_TESTS=
if test "x$config_tests" = "xyes"; then
   AC_DEFINE(HAVE_TESTS,,[Define if test code is enabled])
fi

HAVE_GNET=
if test "x$GNET_LIBS" != "x"; then
   AC_DEFINE(HAVE_GNET,,[Define if GNet is available])
fi

HAVE_GNET2=
if test "x$GNET2" != "x"; then
   AC_DEFINE(HAVE_GNET2,,[Define if GNet2 is available])
fi
        
HAVE_APP_GTK=
if test "x$GTK_LIBS" != "x"; then
   AC_DEFINE(HAVE_APP_GTK,,[Define if Gtk is available])
fi

HAVE_APP_TEXT=
if test "x$config_text" = "xyes"; then
   AC_DEFINE(HAVE_APP_TEXT,,[Define if Text is available])
fi

HAVE_XRECORD_FALLBACK=
if test "x$config_fallback" = "xyes"; then
   AC_DEFINE(HAVE_XRECORD_FALLBACK,,[Define if RECORD fallback is enabled])
fi


dnl
dnl Generate Makefile conditionals
dnl

AM_CONDITIONAL(HAVE_APP_GTKMM, test "x$GTK_LIBS" != "x")
AM_CONDITIONAL(HAVE_APP_TEXT, test "x$config_text" = "xyes")
AM_CONDITIONAL(HAVE_GCONF, test "x$GCONF_LIBS" != "x" -a "x$config_gconf" = "xyes")
AM_CONDITIONAL(HAVE_GDOME, test "x$GDOME_LIB" != "x" -a "x$config_xml" = "xyes")
AM_CONDITIONAL(HAVE_GNOME, test "x$config_gnome" = "xyes")
AM_CONDITIONAL(HAVE_KDE, test "x$config_kde" = "xyes")
AM_CONDITIONAL(HAVE_GNET, test "x$GNET_LIBS" != "x")
AM_CONDITIONAL(NDEBUG, test "x$config_debug" != "xyes")
AM_CONDITIONAL(HAVE_DISTRIBUTION, test "x$config_distribution" = "xyes")
AM_CONDITIONAL(HAVE_EXERCISES, test "x$config_exercises" = "xyes")
AM_CONDITIONAL(HAVE_EXPERIMENTAL, test "x$config_experimental" = "xyes")
AM_CONDITIONAL(HAVE_TESTS, test "x$config_tests" = "xyes")
AM_CONDITIONAL(HAVE_GNOMEAPPLET, test "x$config_gnome" = "xyes" -a "x$have_dbus" = "xyes")


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
          common/src/unix/Makefile \
          common/src/win32/Makefile \
          common/include/Makefile \
          common/include/win32/Makefile \
          common/include/unix/Makefile \
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
          frontend/common/src/kde/Makefile \
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
          frontend/gtkmm/src/gnome_applet3/Makefile \
          frontend/gtkmm/src/kde_applet/Makefile \
          frontend/qt/Makefile \
          frontend/qt/src/Makefile \
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
echo "                      Gtk GUI :   ${config_gtk}"
echo "                Gnome support :   ${config_gnome}"
echo "                  Kde support :   ${config_kde}"
echo "                 DBUS support :   ${have_dbus}"
echo "           Networking support :   ${config_distribution}"
echo "                    Exercises :   ${config_exercises}"
echo ""
echo "                    Debugging :   ${config_debug}"
echo "                    Test code :   ${config_tests}"
echo ""

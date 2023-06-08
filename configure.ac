dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 2001 - 2014 Rob Caelers & Raymond Penners
dnl

m4_define([workrave_major_version], [1])
m4_define([workrave_minor_version], [10])
m4_define([workrave_micro_version], [50])
m4_define([workrave_version_suffix], [])
m4_define([workrave_version],
          workrave_major_version.workrave_minor_version.workrave_micro_version[]m4_ifset([workrave_version_suffix],-[workrave_version_suffix]))
m4_define([workrave_resource_version],
          [workrave_major_version,workrave_minor_version,workrave_micro_version,0])

AC_PREREQ([2.67])
AC_INIT([workrave],
        [workrave_version],
        [https://github.com/rcaelers/workrave/issues],
        [workrave],
        [http://www.workrave.org])
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_CONFIG_SRCDIR([backend/include/ICore.hh])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign silent-rules tar-ustar subdir-objects])
dnl AM_SILENT_RULES([yes])

GNOME_SHELL_VERSION=3.26.1
AC_SUBST(GNOME_SHELL_VERSION)

if test -n "$WORKRAVE_GIT_VERSION"; then
    AC_DEFINE_UNQUOTED(GIT_VERSION, "$WORKRAVE_GIT_VERSION", [GIT Version])
fi

if test "X$prefix" != "XNONE" ; then
   pkg_config_prefix="--define-variable=prefix=$prefix"
fi

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
              [AS_HELP_STRING([--enable-gconf],
                              [compile with Gnome gconf2 support])])

AC_ARG_ENABLE(gsettings,
              [AS_HELP_STRING([--disable-gsettings],
                              [compile without GSettings support])])

AC_ARG_ENABLE(gnome3,
              [AS_HELP_STRING([--disable-gnome3],
                              [compile without Gnome 3 support])])

AC_ARG_ENABLE(indicator,
              [AS_HELP_STRING([--enable-indicator],
                              [compile with Unity/Ayatana indicator applet support])])

AC_ARG_ENABLE(xfce,
              [AS_HELP_STRING([--enable-xfce],
                              [compile with preliminary XFCE applet support])])

AC_ARG_ENABLE(mate,
              [AS_HELP_STRING([--enable-mate],
                              [compile with preliminary MATE applet support])])
AC_ARG_ENABLE(dbus,
              [AS_HELP_STRING([--disable-dbus],
                              [compile without DBus support])])

AC_ARG_ENABLE(gstreamer,
              [AS_HELP_STRING([--disable-gstreamer],
                              [compile without GStreamer audio support])])

AC_ARG_ENABLE(pulse,
              [AS_HELP_STRING([--disable-pulse],
                              [compile without PulseAudio support])])

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

AC_ARG_ENABLE(monitors,
             [AS_HELP_STRING([--enable-monitors=LIST],
                             [comma separated list of activity monitors to use, currently support: record, screensaver, x11events (Unix Only) @<:@default=yes@:>@])])


case x"$target" in
  xNONE | x)
    target_or_host="$host" ;;
  *)
    target_or_host="$target" ;;
esac


dnl
dnl Checks for programs
dnl

AC_USE_SYSTEM_EXTENSIONS

AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR
AC_PROG_OBJC
AC_PROG_OBJCXX
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_CHECK_PROGS(M4, gm4 m4, m4)

AC_CHECK_PROGS(WINDRES, ${target_or_host}-windres,false)

dnl
dnl checks for compiler characteristics
dnl

AC_LANG([C++])
AC_REQUIRE_CPP
AC_C_INLINE
AC_C_CONST

AC_DEFUN([ADD_COMPILER_FLAG],[
AX_CHECK_COMPILE_FLAG([$1],[
CPPFLAGS="$CPPFLAGS $1"
])])

ADD_COMPILER_FLAG([-Wno-cast-function-type])

AX_CXX_COMPILE_STDCXX_11(ext, mandatory)

dnl
dnl Headers/Types/Functions
dnl

AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdlib.h sys/time.h sys/select.h unistd.h])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCT.hwnd,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCT,,[struct MOUSEHOOKSTRUCT]),, [#include <windows.h>])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCTEX.mouseData,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCTEX,,[struct MOUSEHOOKSTRUCTEX]),, [#include <windows.h>])

have_ishelldispatch=no
AC_MSG_CHECKING([IShellDispatch])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <shlobj.h>]], [[ IShellDispatch* pShellDispatch = NULL; ]])],[have_ishelldispatch=yes
         AC_DEFINE(HAVE_ISHELLDISPATCH, 1, "IShellDispatch")
         AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

AC_CHECK_FUNCS([gettimeofday nanosleep select setlocale realpath])

have_extern_timezone_defined=no
AC_MSG_CHECKING([external timezone variable defined in time.h])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <time.h>]], [[ timezone += 1;
          return 0;]])],[have_extern_timezone_defined=yes
         AC_DEFINE(HAVE_EXTERN_TIMEZONE_DEFINED, 1, "External timezone")
         AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

have_extern_timezone=no
AC_MSG_CHECKING([external timezone variable])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <time.h>]], [[ extern long timezone;
          timezone += 1;
          return 0;]])],[have_extern_timezone=yes
         AC_DEFINE(HAVE_EXTERN_TIMEZONE, 1, "External timezone")
         AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

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

PLATFORM_OS_WINDOWS=no
PLATFORM_OS_MACOS=no
platform_os_unix=no
platform_variant_x11=no

case "$target_or_host" in
  *-*mingw*|*-*-mingw*|*-*-cygwin*)
    PLATFORM_OS_WINDOWS=yes
    platform_os_name="Windows"
   AC_DEFINE(PLATFORM_OS_WINDOWS,,[Define if compiling for the Win32 platform])
   AC_DEFINE(HAVE_LANGUAGE_SELECTION,,[Define if the UI language can be defined in the preferences])
    ;;
  *-*-darwin* | *-*-rhapsody*)
    PLATFORM_OS_MACOS=yes
    platform_os_name="OS X"
   AC_DEFINE(PLATFORM_OS_MACOS,,[Define if compiling for the OS X platform])
    ;;
  *)
    platform_os_unix=yes
    platform_os_name="Unix"
   AC_DEFINE(PLATFORM_OS_UNIX,,[Define if compiling for a Unix platform])
    ;;
esac

AC_MSG_RESULT([$platform_os_name])

AM_CONDITIONAL(PLATFORM_OS_WINDOWS, test "x$PLATFORM_OS_WINDOWS" = "xyes")
AM_CONDITIONAL(PLATFORM_OS_MACOS, test "x$PLATFORM_OS_MACOS" = "xyes")
AM_CONDITIONAL(PLATFORM_OS_UNIX, test "x$platform_os_unix" = "xyes")
AM_CONDITIONAL(HAVE_LANGUAGE_SELECTION, test "x$PLATFORM_OS_WINDOWS" = "xyes")

dnl
dnl Debug
dnl

config_debug=no
config_tracing=no

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
   config_tracing=yes
   AC_DEFINE([TRACING], , [Define to enable method call tracing])
fi


dnl
dnl Windows specific checks
dnl

have_dsound=no
LIBSTDCPP_LIBS=
WIN32CONSOLE=
DIRECTSOUNDLIBS=

if test "x$PLATFORM_OS_WINDOWS" = "xyes"
then
    LIBSTDCPP_LIBS=-lstdc++

    AC_DEFINE([_WIN32_IE],    [0x0600], [Windows version])
    AC_DEFINE([_WIN32_WINNT], [0x0600], [Windows version])

    #if test "x$enable_debug" != "xyes"
    #then
    WIN32CONSOLE=-mwindows
    #else
    #    WIN32CONSOLE=-mconsole
    #fi

    enable_gconf=no
    enable_gnome3=no
    enable_xml=no
    enable_indicator=no
    enable_xfce=no
    enable_mate=no

   AC_CHECK_HEADER(dsound.h, [ AC_DEFINE(HAVE_DSOUND, 1, [Define if we have DirectSound])
                                        have_dsound=yes DIRECTSOUNDLIBS="-ldsound -luuid -ldxguid -ldxerr8"
                                     ])

   AC_CHECK_HEADERS([dxerr.h dxerr8.h], [], [],
[[#include <windows.h>
]])

   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <mmdeviceapi.h>
#include <endpointvolume.h>

static REFIID foo = __uuidof(IAudioEndpointVolume);
   ]], [[
      IMMDeviceEnumerator *device_enum = NULL;
   ]])],[ have_mmdeviceapi=yes
     AC_DEFINE(HAVE_MMDEVICEAPI_H, 1, [Define if we have mm device api])
     AC_MSG_RESULT(yes)
   ],[AC_MSG_RESULT(no)])

fi

AC_SUBST(LIBSTDCPP_LIBS)
AC_SUBST(WIN32CONSOLE)
AC_SUBST(DIRECTSOUNDLIBS)

AM_CONDITIONAL(HAVE_DSOUND, test "x$have_dsound" = "xyes")

dnl
dnl MacOS specific checks
dnl

PKG_PROG_PKG_CONFIG

AC_SUBST(MacOS_CFLAGS)

if test "x$PLATFORM_OS_MACOS" = "xyes"
then

   PKG_CHECK_MODULES(IGE, gtk-mac-integration >= 2.0,
        [AC_DEFINE(HAVE_GTK_MAC_INTEGRATION, 1, [Define if we have gtk-mac-integration 2.0+])],
        [PKG_CHECK_MODULES(IGE, ige-mac-integration >= 0.6, AC_DEFINE(HAVE_IGE_MAC_INTEGRATION, 1, [Define if we have ige-mac-integration 0.6+]))])

   AC_CHECK_LIB(resolv, res_query)

   AC_CHECK_HEADER(CoreFoundation/CoreFoundation.h, [ AC_CHECK_HEADER(IOKit/IOKitLib.h,
                                     [  AC_DEFINE(HAVE_IOKIT, 1, [Define if we have IOKit])
                                        LIBS="$LIBS -framework IOKit -framework CoreFoundation"
                                     ])])

    config_gnome3=no
    config_gconf=no

    LDFLAGS="$LDFLAGS -headerpad_max_install_names"
    LIBS="$LIBS -framework CoreServices -framework Carbon -framework Cocoa -framework Foundation"
    MacOS_CFLAGS=""
fi

AC_SUBST(IGE_LIBS)
AC_SUBST(IGE_CFLAGS)

dnl
dnl Unix specific checks
dnl

if test "x$platform_os_unix" = "xyes"
then
    AC_PATH_X
    AC_PATH_XTRA

    if test x$have_x != xyes ; then
       AC_MSG_ERROR(X11 required on Unix platform)
    fi

    LIBS_save=$LIBS
    if test -n "$ac_x_libraries"
    then
       LIBS="$LIBS -L$x_libraries"
    fi

    CPPFLAGS_save="$CPPFLAGS"
    if test -n "$x_includes"
    then
       CPPFLAGS="$CPPFLAGS -I$x_includes"
    fi

    have_xrecord=no
    AC_CHECK_LIB(Xtst, XRecordEnableContext,
                       have_xrecord=yes X_LIBS="$X_LIBS -lXtst" AC_DEFINE(HAVE_XRECORD,,[Define if the RECORD extension is available]),
    )
    if test x$have_xrecord != xyes ; then
       AC_MSG_ERROR(X RECORD extension headers files required on Unix platform)
    fi

    AC_CHECK_LIB(Xext, XScreenSaverRegister,
                       have_xscreensaver=yes X_LIBS="$X_LIBS -lX11 -lXext",
                       [],
                       [-lX11 -lXext -lm])
    AC_CHECK_LIB(Xss, XScreenSaverRegister,
                      have_xscreensaver=yes X_LIBS="$X_LIBS -lX11 -lXss",
                      [],
                      [-lX11 -lXext -lm])

    if test "x$have_xscreensaver" = "xyes"; then
       AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            #include <X11/Xlib.h>
            #include <X11/extensions/scrnsaver.h>
          ]], [[]])], [], [have_xscreensaver=no])
    fi

    if test "x$have_xscreensaver" = "xyes" ; then
       AC_DEFINE(HAVE_SCREENSAVER, 1, [Define if XScreenSaver is available.])
    fi

    PKG_CHECK_MODULES(X11SM, sm ice)
    LIBS=$LIBS_save
    CPPFLAGS=$CPPFLAGS_save
fi

dnl
dnl Monitors
dnl

if test "x$platform_os_unix" = "xyes"
then

    if test "x$enable_monitors" = "x"; then
        enable_monitors="mutter"

        if test "x$have_xrecord" = "xyes" ; then
            if test "x$enable_monitors" != "x"; then
               enable_monitors="$enable_monitors,"
            fi
            enable_monitors="${enable_monitors}record"
        fi
        if test "x$have_xscreensaver" = "xyes" ; then
            if test "x$enable_monitors" != "x"; then
               enable_monitors="$enable_monitors,"
            fi
            enable_monitors="${enable_monitors}screensaver"
        fi

        if test "x$enable_monitors" != "x"; then
            enable_monitors="$enable_monitors,"
        fi
        enable_monitors="${enable_monitors}x11events"
    fi

    loop=${enable_monitors},

    while echo $loop | grep \, &> /dev/null
    do
        monitor=${loop%%\,*}
        loop=${loop#*\,}

        case "$monitor" in
           record)
               if test "x$have_xrecord" != "xyes" ; then
                   AC_MSG_ERROR([record activity monitor not supported.])
               fi
               ;;

           x11events)
               ;;

           mutter)
               ;;

           screensaver)
               if test "x$have_xscreensaver" != "xyes" ; then
                   AC_MSG_ERROR([screensaver activity monitor not supported.])
               fi
               ;;

           *)
               AC_MSG_ERROR([unknown activity monitor: $monitor])
               ;;

        esac
    done

    AC_DEFINE_UNQUOTED(HAVE_MONITORS, "$enable_monitors", "Enabled activity monitors")

fi

dnl
dnl DBus
dnl

config_dbus=no
have_python_jinja2=no

if test "x$enable_dbus" != "xno" -a "x$PLATFORM_OS_WINDOWS" != "xyes" -a "x$PLATFORM_OS_MACOS" != "xyes"
then
PKG_CHECK_MODULES([GIO],
                  [gio-2.0 >= 2.26.0],
                  [ config_dbus=yes ], [ config_dbus=no])
fi

if test "x$config_dbus" = "xyes"
then
    # For now, only needed for DBus.
    AX_BOOST_BASE([1.52],, AC_MSG_ERROR([requires boost library >= 1.52.]))

    AC_DEFINE(HAVE_DBUS, , [DBus support])

    AS_AC_EXPAND(DATADIR, "$datadir")
    DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
    AC_SUBST(DBUS_SERVICES_DIR)
    AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for DBUS is])

    AM_PATH_PYTHON([3.0])
    AX_PYTHON_MODULE([jinja2], [needed], python3)
fi

AM_CONDITIONAL(HAVE_DBUS, test x$config_dbus = xyes)
AM_CONDITIONAL(HAVE_PYTHON_JINJA2, test x$HAVE_PYMOD_jinja2 = xyes)


dnl
dnl Gnome
dnl

config_gnome3=no
config_panelapplet4=no
config_panelapplet5=no
config_libgnome_panel=no

if test "x$enable_gnome3" != "xno" -a "x$config_dbus" = "xyes"
then
    PKG_CHECK_MODULES([GNOME3],
                      [libgnome-panel >= 3.36.0 gio-2.0 >= 2.26.0],
                      [config_gnome3=yes
                       config_libgnome_panel=yes
                       LIBGNOME_PANEL_MODULES_DIR=`$PKG_CONFIG ${pkg_config_prefix} --variable=modulesdir libgnome-panel`
                       AC_SUBST([LIBGNOME_PANEL_MODULES_DIR], [$LIBGNOME_PANEL_MODULES_DIR])
                       AC_DEFINE([HAVE_LIBGNOME_PANEL], 1, [Support for GNOME Panel applet])],
                      [ ])

    if test "x$config_libgnome_panel" != "xyes"
    then
        PKG_CHECK_MODULES(GNOME3,
                          [libpanel-applet >= 3.16.1 gio-2.0 >= 2.26.0],
                          [config_gnome3=yes
                           config_panelapplet5=yes
                           AC_DEFINE([HAVE_PANELAPPLET], [1], [Support for Gnome panel applet v4/v5])
                           AC_DEFINE([HAVE_PANELAPPLET5], [1], [Support for Gnome panel applet v5])],
                           [ ])

        if test "x$config_panelapplet5" != "xyes"
        then
            PKG_CHECK_MODULES([GNOME3],
                              [libpanelapplet-4.0 gio-2.0 >= 2.26.0],
                              [config_gnome3=yes
                               config_panelapplet4=yes
                               AC_DEFINE(HAVE_PANELAPPLET,  1, [Support for Gnome panel applet v4/v5]),
                               AC_DEFINE(HAVE_PANELAPPLET4, 1, [Support for Gnome panel applet v4])],
                               [if test "x$enable_gnome3" = "xyes"; then
                                  AC_MSG_ERROR([Gnome 3 development headers not found.])
                               fi])
        fi
    fi
fi

AM_CONDITIONAL(HAVE_PANELAPPLET,   test "x$config_panelapplet4" = "xyes" -o "x$config_panelapplet5" = "xyes")
AM_CONDITIONAL(HAVE_PANELAPPLET4, test "x$config_panelapplet4" = "xyes")
AM_CONDITIONAL(HAVE_PANELAPPLET5, test "x$config_panelapplet5" = "xyes")
AM_CONDITIONAL(HAVE_LIBGNOME_PANEL, test "x$config_libgnome_panel" = "xyes")

dnl
dnl GLIB
dnl

PKG_CHECK_MODULES([GLIB],
                  [glib-2.0 >= 2.10 gmodule-2.0 gthread-2.0],
                  [AC_DEFINE(HAVE_GLIB, , [Define if GLib is available])])


config_gsettings=no
if test "x$enable_gsettings" != "xno"
then
   if test "x$platform_os_unix" = "xyes"; then
      GLIB_GSETTINGS
      config_gsettings=yes
      AC_DEFINE(HAVE_GSETTINGS, 1, [Have GSettings])
   fi
fi

AM_CONDITIONAL(HAVE_GSETTINGS, test "x$config_gsettings" = "xyes")

dnl
dnl GTK+
dnl

config_gtk=no
config_gtk_version=none

if test "x$PLATFORM_OS_WINDOWS" != "xyes"
then

PKG_CHECK_MODULES(GTK,
                     glib-2.0 >= 2.28.0
                     gio-2.0 >= 2.26.0
                     gtk+-3.0 >= 3.0.0
                     gdk-3.0 >= 3.0.0
                     sigc++-2.0 >= 2.2.4.2
                     glibmm-2.4 >= 2.28.0
                     gtkmm-3.0 >= 3.0.0,
                     [config_gtk=yes
                      config_gtk_version=3
                      AC_DEFINE(HAVE_GTK3, 1, Support for GTK3)
                      AC_DEFINE([HAVE_APP_GTK], , [Define if GTK+ is available])], [ ] )

PKG_CHECK_MODULES(GTK4,
                     gtk4 >= 4.10.0,
                     [have_gtk4=yes
                      AC_DEFINE(HAVE_GTK4, 1, Support for GTK4)], [ ] )


fi

if test x$config_gtk = xno
then
   PKG_CHECK_MODULES(GTK,
                     gtk+-2.0 >= 2.6.0 \
                     gdk-2.0 >= 2.8 \
                     sigc++-2.0 \
                     glibmm-2.4 >= 2.10 \
                     gtkmm-2.4 >= 2.10,
                     [config_gtk=yes
                      config_gtk_version=2
                      AC_DEFINE([HAVE_APP_GTK], , [Define if GTK+ is available])])
fi

AM_CONDITIONAL(HAVE_APP_GTKMM, test $config_gtk = yes)
AM_CONDITIONAL(HAVE_GTK3, test "x$config_gtk_version" = "x3")
AM_CONDITIONAL(HAVE_GTK4, test $have_gtk4 = yes)


dnl
dnl Introspection
dnl

if test "x$PLATFORM_OS_WINDOWS" != "xyes"
then
    GOBJECT_INTROSPECTION_CHECK([0.6.7])
else
    AM_CONDITIONAL(HAVE_INTROSPECTION, false)
fi

dnl
dnl Indicator applet
dnl

config_indicator=no
config_indicator_package=none

INDICATOR_REQUIRED_VERSION=0.3.19
DBUSMENUGLIB_REQUIRED_VERSION=0.1.1
DBUSMENUGTK_REQUIRED_VERSION=0.3.95

if test "x$enable_indicator" != "xno" -a "x$config_gtk_version" = "x3"
then
   if test "x$found_introspection" != "xyes"
   then
       AC_MSG_ERROR([Introspection development headers not found.])
   else
       PKG_CHECK_MODULES(INDICATOR, ayatana-indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                    dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                    dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION,
                                  [ config_indicator=yes
                                    config_indicator_package=ayatana-indicator3-0.4
                                    AC_DEFINE(HAVE_INDICATOR, 1, [Support for indicator applet])
                                    AC_DEFINE(HAVE_INDICATOR_AYATANA, 1, [Support for ayatana indicator applet])
                                  ],

                                  [  PKG_CHECK_MODULES(INDICATOR, indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                                                  dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                                                  dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION,
                                                      [ config_indicator=yes
                                                        config_indicator_package=indicator3-0.4
                                                         AC_DEFINE(HAVE_INDICATOR, 1, [Support for indicator applet])],
                                                      [if test "x$enable_indicator" = "xyes"; then
                                                         AC_MSG_ERROR([Indicator development headers not found.])
                                                      fi])
                                  ])

       if test "x$config_indicator" != "xno"
       then
          PKG_CHECK_EXISTS(dbusmenu-gtk3-0.4 >= 0.5.90,
                         [ AC_DEFINE(HAVE_DBUSMENU_NEW_INCLUDES, 1, [DBusmenu header in new location])],
                     [ true ]
                         )

       fi
   fi
fi

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

AS_IF([test "x$with_localinstall" = "xyes" -a "x$config_indicator_package" != "xnone"],
    INDICATORDIR="${libdir}/indicators3/7"
    INDICATORICONSDIR="${datadir}/libindicate/icons"
    ,
    INDICATORDIR=`$PKG_CONFIG --variable=indicatordir $config_indicator_package | sed -e 's%/$%%'`
    INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir $config_indicator_package`
    )

AM_CONDITIONAL(LOCALINSTALL, test "x$with_localinstall" = "xyes")

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

AM_CONDITIONAL(HAVE_INDICATOR, test "x$config_indicator" = "xyes")

dnl
dnl Xfce
dnl

config_xfce=no
config_xfce_gtk_version=none

if test "x$enable_xfce" != "xno"
then
   PKG_CHECK_MODULES([LIBXFCE4PANEL],
                    [libxfce4panel-2.0 >= 4.12],
                    [config_xfce=yes
                     config_xfce_gtk_version=3
                     AC_DEFINE(HAVE_XFCE, 1, [Support for xfce applet])
                     AC_DEFINE(HAVE_XFCE_GTK3, 1, [Support for xfce applet])
                     XFCE4LIBDIR=`$PKG_CONFIG ${pkg_config_prefix} --variable=libdir libxfce4panel-2.0`
                     ],
                     [ true ])

   if test x$config_xfce != xyes
   then
      PKG_CHECK_MODULES([LIBXFCE4PANEL],
                         [libxfce4panel-1.0 >= 4.4],
                         [config_xfce=yes
                          config_xfce_gtk_version=2
                          AC_DEFINE(HAVE_XFCE, 1, [Support for xfce applet])
                          AC_DEFINE(HAVE_XFCE_GTK2, 1, [Support for xfce applet])
                          ],
                         [if test "x$enable_xfce" = "xyes"; then
                          AC_MSG_ERROR([XFCE Panel applet development headers not found.])
                          fi])
   fi
fi

AC_SUBST(XFCE4LIBDIR)

AM_CONDITIONAL(HAVE_XFCE, test "x$config_xfce" = "xyes")

AM_CONDITIONAL(HAVE_XFCE_GTK2, test "x$config_xfce_gtk_version" = "x2")
AM_CONDITIONAL(HAVE_XFCE_GTK3, test "x$config_xfce_gtk_version" = "x3")


dnl
dnl Mate
dnl

config_mate=no

if test "x$enable_mate" != "xno"
then
   PKG_CHECK_MODULES([LIBMATEPANEL],
                      [libmatepanelapplet-4.0],
                      [config_mate=yes
                       AC_DEFINE(HAVE_MATE, 1, [Support for mate applet])],
                      [if test "x$enable_mate" = "xyes"; then
                          AC_MSG_ERROR([MATE Panel applet development headers not found.])
                       fi])

   config_mate_gtk_version=none
   if test "x$config_mate" = "xyes"
   then
      MATE_GTK=`$PKG_CONFIG --print-requires libmatepanelapplet-4.0 2>/dev/null | grep -m 1 '^gtk+-'`
      case "$MATE_GTK" in
         gtk+-2*)
           config_mate_gtk_version=2
           ;;
         gtk+-3*)
           config_mate_gtk_version=3
           ;;
      esac
   fi
fi

AM_CONDITIONAL(HAVE_MATE, test "x$config_mate" = "xyes")
AM_CONDITIONAL(HAVE_MATE_GTK2, test "x$config_mate_gtk_version" = "x2")
AM_CONDITIONAL(HAVE_MATE_GTK3, test "x$config_mate_gtk_version" = "x3")


if test "x$config_mate_gtk_version" = "x2" -o "x$config_xfce_gtk_version" = "x2"
then
  PKG_CHECK_MODULES([GTK2],
                    [gtk+-2.0 >= 2.6.0 gdk-2.0 >= 2.8 ],
                    [config_gtk2=yes
                     AC_DEFINE(HAVE_GTK2, 1, [Support for GTK2])],
                    [AC_MSG_ERROR([GTK2 development headers not found.])])
fi

AM_CONDITIONAL(HAVE_GTK2, test "x$config_gtk2" = "xyes")

dnl
dnl GConf
dnl

config_gconf=no

if test "x$enable_gconf" != "xno" -a "x$config_gnome3" != "xyes"
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
dnl GStreamer
dnl

config_gstreamer=no

if test "x$enable_gstreamer" != "xno";
then
    PKG_CHECK_MODULES([GSTREAMER],
                      [gstreamer-0.10],
                      [config_gstreamer=yes
                       AC_DEFINE(HAVE_GSTREAMER, 1, [Use GStreamer for playing sounds])], [ ])

    if test "x$config_gstreamer" != "xyes";
    then
        PKG_CHECK_MODULES([GSTREAMER],
                      [gstreamer-1.0],
                      [config_gstreamer=yes
                       AC_DEFINE(HAVE_GSTREAMER, 1, [Use GStreamer for playing sounds])],
                         [if test "x$enable_gstreamer" = "xyes"; then
                           AC_MSG_ERROR([GStreamer development headers not found.])
                       fi])
    fi
fi


dnl
dnl PulseAudio
dnl

config_pulse=no

if test "$platform_os_unix" = "yes" -a "x$enable_pulse" != "xno"
then
    PA_REQUIRED_VERSION=0.9.15
    PKG_CHECK_MODULES([PULSE],
                      [glib-2.0 libpulse >= $PA_REQUIRED_VERSION libpulse-mainloop-glib >= $PA_REQUIRED_VERSION],
                      [config_pulse=yes
                  AC_DEFINE(CONFIG_PULSE, 1, [Define if PULSE sound server should be used])],
                      [if test "x$enable_pulse" = "xyes"; then
                           AC_MSG_ERROR([Pulseaudio development headers not found.])
                       fi])
fi

AM_CONDITIONAL(CONFIG_PULSE, test "x$config_pulse" = "xyes")


dnl
dnl Internationalization
dnl

AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])

dnl Hack because windows gettext package is SEVERELY broken...
if test "x$PLATFORM_OS_WINDOWS" = "xyes"
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
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the gettext domain name.])
IT_PROG_INTLTOOL([0.40.0])


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

if test "x$PLATFORM_OS_WINDOWS" = "xyes"
then
   variant="$variant win32"
elif test "x$PLATFORM_OS_MACOS" = "xyes"
then
   variant="$variant macos unix"
elif test "x$platform_os_unix" = "xyes"
then
   variant="$variant x11 unix"
fi

WR_BACKEND_INCLUDES="-I\$(top_srcdir)/backend/include"
WR_COMMON_INCLUDES="-I\$(top_srcdir)/common/include -I\$(top_srcdir)/libs/dbus/include"
WR_FRONTEND_COMMON_INCLUDES="-I\$(top_srcdir)/frontend/common/include"

for v in $variant; do
    if test -d common/src/$v
    then
       WR_COMMON_INCLUDES="$WR_COMMON_INCLUDES -I\$(top_srcdir)/common/include/$v"
    fi
done

WR_LDADD="-lworkrave-backend -lworkrave-frontend-common -lworkrave-common"
if test "x$config_dbus" = "xyes"; then
  WR_LDADD="$WR_LDADD -lworkrave-dbus"
fi

WR_LDFLAGS="-L\$(top_builddir)/common/src/ -L\$(top_builddir)/backend/src/ -L\$(top_builddir)/frontend/common/src -L\$(top_builddir)/libs/dbus/src/"

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


dnl
dnl Generate language list.
dnl

if test "x$PLATFORM_OS_WINDOWS" = "xyes"
then
    LANGFILE="./frontend/gtkmm/win32/setup/lang.iss"

    ALL_LINGUAS="$(cat ./po/LINGUAS |grep -v '^#' | tr '\n' ' ' | awk '{$1=$1;print}')"
    AC_DEFINE_UNQUOTED(ALL_LINGUAS, "$ALL_LINGUAS" ,[All available languages])

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

AC_CONFIG_FILES([Makefile
             backend/Makefile
             backend/test/Makefile
             backend/src/Makefile
             backend/src/org.workrave.gschema.xml.in
             backend/src/unix/Makefile
             backend/src/macos/Makefile
             backend/src/win32/Makefile
             backend/include/Makefile
             common/Makefile
             common/bin/Makefile
             common/src/Makefile
             common/src/win32/Makefile
             common/include/Makefile
             common/include/win32/Makefile
             common/win32/Makefile
             common/win32/harpoon/Makefile
             common/win32/harpoon/include/Makefile
             common/win32/harpoon/src/Makefile
             common/win32/harpoonHelper/Makefile
             common/win32/harpoonHelper/include/Makefile
             common/win32/harpoonHelper/src/Makefile
             libs/Makefile
             libs/dbus/Makefile
             libs/dbus/bin/Makefile
             libs/dbus/data/Makefile
             libs/dbus/include/Makefile
             libs/dbus/include/dbus/Makefile
             libs/dbus/src/Makefile
             frontend/Makefile
             frontend/common/Makefile
             frontend/common/include/Makefile
             frontend/common/share/Makefile
             frontend/common/share/sounds/Makefile
             frontend/common/share/sounds/bells/Makefile
             frontend/common/share/sounds/subtle/Makefile
             frontend/common/share/sounds/default/Makefile
             frontend/common/share/images/Makefile
             frontend/common/share/images/16x16/Makefile
             frontend/common/share/images/24x24/Makefile
             frontend/common/share/images/32x32/Makefile
             frontend/common/share/images/48x48/Makefile
             frontend/common/share/images/64x64/Makefile
             frontend/common/share/images/96x96/Makefile
             frontend/common/share/images/128x128/Makefile
             frontend/common/share/images/scalable/Makefile
             frontend/common/share/images/win32/Makefile
             frontend/common/share/images/macos/Makefile
             frontend/common/src/Makefile
             frontend/common/src/win32/Makefile
             frontend/common/src/x11/Makefile
             frontend/common/src/macos/Makefile
             frontend/applets/Makefile
             frontend/applets/win32/Makefile
             frontend/applets/win32/src/Makefile
             frontend/applets/win32/include/Makefile
             frontend/applets/common/Makefile
             frontend/applets/common/src/Makefile
             frontend/applets/common/include/Makefile
             frontend/applets/gnome3/Makefile
             frontend/applets/gnome3/src/Makefile
             frontend/applets/indicator/Makefile
             frontend/applets/indicator/src/Makefile
             frontend/applets/indicator/include/Makefile
             frontend/applets/xfce/Makefile
             frontend/applets/xfce/src/Makefile
             frontend/applets/xfce/include/Makefile
             frontend/applets/mate/Makefile
             frontend/applets/mate/src/Makefile
             frontend/applets/mate/include/Makefile
             frontend/applets/gnome-shell/Makefile
             frontend/applets/gnome-shell/src/Makefile
             frontend/applets/cinnamon/Makefile
             frontend/applets/cinnamon/src/Makefile
             frontend/gtkmm/Makefile
             frontend/gtkmm/macos/Makefile
             frontend/gtkmm/win32/Makefile
             frontend/gtkmm/win32/setup/Makefile
             frontend/gtkmm/win32/setup/setup.iss
             frontend/gtkmm/win32/setup/src/Makefile
             frontend/gtkmm/src/Makefile
             frontend/gtkmm/src/org.workrave.gui.gschema.xml.in
             frontend/gtkmm/src/win32/Makefile
             frontend/gtkmm/src/unix/Makefile
             frontend/gtkmm/src/macos/Makefile
             frontend/text/Makefile
             frontend/text/src/Makefile
             frontend/plugin/Makefile
             frontend/plugin/distribution/Makefile
             frontend/plugin/distribution/gtkmm/Makefile
             frontend/plugin/distribution/gtkmm/src/Makefile
             frontend/plugin/statistics/Makefile
             frontend/plugin/statistics/gtkmm/Makefile
             frontend/plugin/statistics/gtkmm/src/Makefile
             frontend/plugin/exercises/Makefile
             frontend/plugin/exercises/common/Makefile
             frontend/plugin/exercises/common/share/Makefile
             frontend/plugin/exercises/common/src/Makefile
             frontend/plugin/exercises/gtkmm/Makefile
             frontend/plugin/exercises/gtkmm/src/Makefile
             po/Makefile.in
             contrib/Makefile
             contrib/plot/Makefile
             contrib/send_menu_command/Makefile
             contrib/send_menu_command/win32/Makefile
             contrib/send_dbus_command/Makefile
             contrib/send_dbus_command/kde/Makefile
             build/Makefile
             m4/Makefile
])

AC_OUTPUT

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
echo "      GSettings configuration :   ${config_gsettings}"
echo "            XML configuration :   ${config_xml}"
echo ""
if test "$platform_os_unix" = "yes"; then
echo "            Activity Monitors :   ${enable_monitors}"
echo ""
fi
echo "                      Gtk GUI :   ${config_gtk} (Gtk${config_gtk_version})"
echo "              Gnome 3 support :   ${config_gnome3}"
echo "GObject-Introspection support :   ${found_introspection}"
echo "            Indicator support :   ${config_indicator}"
echo "                 XFCE support :   ${config_xfce} (Gtk${config_xfce_gtk_version})"
echo "                 MATE support :   ${config_mate} (Gtk${config_mate_gtk_version})"
echo "                 DBUS support :   ${config_dbus}"
echo "           Networking support :   ${config_distribution}"
echo "            GStreamer support :   ${config_gstreamer}"
if test "$platform_os_unix" = "yes"; then
echo "           Pulseaudio support :   ${config_pulse}"
fi
echo "                    Exercises :   ${config_exercises}"
echo ""
echo "        Experimental features :   ${config_experimental}"
echo "                    Debugging :   ${config_debug}"
echo "                   Debug logs :   ${config_tracing}"
echo "                    Test code :   ${config_tests}"
echo ""

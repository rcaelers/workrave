dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
dnl

m4_define([workrave_major_version], [1])
m4_define([workrave_minor_version], [10])
m4_define([workrave_micro_version], [1])
m4_define([workrave_version_suffix], [2])
m4_define([workrave_version],
          [workrave_major_version.workrave_minor_version.workrave_micro_version.workrave_version_suffix])
#m4_define([workrave_version],
#          [workrave_major_version.workrave_minor_version])
m4_define([workrave_resource_version],
          [workrave_major_version,workrave_minor_version,workrave_micro_version,workrave_version_suffix])

AC_PREREQ([2.67])
AC_INIT([workrave],
        [workrave_version],
        [http://issues.workrave.org/cgi-bin/bugzilla/enter_bug.cgi],
        [workrave],
        [http://www.workrave.org])

AC_CANONICAL_TARGET

AC_CONFIG_SRCDIR([backend/include/ICore.hh])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign silent-rules tar-ustar])
dnl AM_SILENT_RULES([yes])

GNOME_SHELL_VERSION=3.6.2
AC_SUBST(GNOME_SHELL_VERSION)

if test -d $srcdir/.git && test "$TINDERBOX_BUILD" = yes; then
    git_version=`git describe --tags --abbrev=10 --dirty 2>/dev/null`
    AC_DEFINE_UNQUOTED(GIT_VERSION, "$git_version", [GIT Version])
fi


dnl
dnl Command line options.
dnl

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

AC_ARG_ENABLE(debug,
              [AS_HELP_STRING([--enable-debug],
                              [compile with debugging flags set])])

AC_ARG_ENABLE(experimental,
              [AS_HELP_STRING([--enable-experimental],
                              [compile with experimental features])])

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


case x"$target_alias" in
  xNONE | x)
    target_or_host="$host_alias" ;;
  *)
    target_or_host="$target_alias" ;;
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
AM_PROG_AR
AC_PROG_LIBTOOL
AC_CHECK_PROGS(M4, gm4 m4, m4)

AC_CHECK_PROGS(WINDRES, ${target_or_host}-windres,false)

# Libtool
LT_PREREQ([2.2])
LT_INIT([dlopen])

dnl
dnl checks for compiler characteristics
dnl

AC_LANG([C++])
AC_REQUIRE_CPP
AC_C_INLINE
AC_C_CONST

AC_COMPILE_STDCXX_11

AX_BOOST_BASE([1.49])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_TEST_EXEC_MONITOR

dnl
dnl Headers/Types/Functions
dnl

AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdlib.h sys/time.h sys/select.h unistd.h])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCT.hwnd,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCT,,[struct MOUSEHOOKSTRUCT]),, [#include <windows.h>])
AC_CHECK_MEMBER(MOUSEHOOKSTRUCTEX.mouseData,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCTEX,,[struct MOUSEHOOKSTRUCTEX]),, [#include <windows.h>])

AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
dnl AC_TYPE_UINT8_T

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

platform_os_win32=no
platform_os_osx=no
platform_os_unix=no
platform_variant_x11=no

case "$target_os" in
  *mingw*)
    platform_os_win32=yes
    platform_os_name="Windows"
   AC_DEFINE(PLATFORM_OS_WIN32,,[Define if compiling for the Win32 platform])
   AC_DEFINE(HAVE_LANGUAGE_SELECTION,,[Define if the UI language can be defined in the preferences])
    ;;
  *darwin*)
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
AC_MSG_RESULT([$target_os])
AC_MSG_RESULT([$target_alias])
AC_MSG_RESULT([$host_alias])

AM_CONDITIONAL(PLATFORM_OS_WIN32, test "x$platform_os_win32" = "xyes")
AM_CONDITIONAL(PLATFORM_OS_OSX, test "x$platform_os_osx" = "xyes")
AM_CONDITIONAL(PLATFORM_OS_UNIX, test "x$platform_os_unix" = "xyes")
AM_CONDITIONAL(HAVE_LANGUAGE_SELECTION, test "x$platform_os_win32" = "xyes")

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
WIN32LIBS=
WIN32LDFLAGS=
WIN32CONSOLE=
DIRECTSOUNDLIBS=

if test "x$platform_os_win32" = "xyes"
then
    LIBSTDCPP_LIBS=-lstdc++

    AC_DEFINE([_WIN32_IE],    [0x0600], [Windows version])
    AC_DEFINE([_WIN32_WINNT], [0x0600], [Windows version])

    WIN32LIBS="-lshell32 -lole32 -lwinmm -lshlwapi -lwtsapi32"
    WIN32LDFLAGS="-static-libgcc -static-libstdc++"
    
    #if test "x$enable_debug" != "xyes"
    #then
    WIN32CONSOLE=-mwindows
    #else
    #    WIN32CONSOLE=-mconsole
    #fi

    enable_gconf=no
    enable_gnome3=no
    enable_indicator=no
    
   AC_CHECK_HEADER(dsound.h, [ AC_DEFINE(HAVE_DSOUND, 1, [Define if we have DirectSound])
                                        have_dsound=yes DIRECTSOUNDLIBS="-ldsound -luuid -ldxguid -ldxerr8"
                                     ])

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
AC_SUBST(WIN32LDFLAGS)
AC_SUBST(WIN32LIBS)
AC_SUBST(DIRECTSOUNDLIBS)

AM_CONDITIONAL(HAVE_DSOUND, test "x$have_dsound" = "xyes")

dnl
dnl OSX specific checks
dnl

PKG_PROG_PKG_CONFIG

AC_SUBST(OSX_CFLAGS)

if test "x$platform_os_osx" = "xyes"
then

   PKG_CHECK_MODULES(IGE, gtk-mac-integration >= 2.0,
        [AC_DEFINE(HAVE_GTK_MAC_INTEGRATION, 1, [Define if we have gtk-mac-integration 2.0+])],
        [PKG_CHECK_MODULES(IGE, ige-mac-integration >= 0.6, AC_DEFINE(HAVE_IGE_MAC_INTEGRATION, 1, [Define if we have ige-mac-integration 0.6+]))])

   dnl AC_CHECK_LIB(resolv, res_query)

   AC_CHECK_HEADER(CoreFoundation/CoreFoundation.h, [ AC_CHECK_HEADER(IOKit/IOKitLib.h,
                                     [  AC_DEFINE(HAVE_IOKIT, 1, [Define if we have IOKit])
                                        LIBS="$LIBS -framework IOKit -framework CoreFoundation"
                                     ])])

   AC_LANG_PUSH([Objective C++])
   AC_CHECK_HEADER(QTKit/QTKit.h,
        [AC_DEFINE(HAVE_QTKIT, 1, [Define if we have QTKit]) LIBS="$LIBS -framework QTKit" ],
        [AC_DEFINE(HAVE_QUICKTIME, 1, [Define if we have QuickTime]) LIBS="$LIBS -framework QuickTime" ])
   AC_LANG_POP()

    enable_gnome3=no
    enable_gconf=no
    enable_dbus=no
    enable_indicator=no

    LDFLAGS="$LDFLAGS -headerpad_max_install_names"
    LIBS="$LIBS -framework CoreServices -framework Carbon -framework Cocoa -framework Foundation"
    OSX_CFLAGS=""
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

    AC_DEFINE([_XOPEN_SOURCE],    [600], [XOpen version])

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

    if test "x$have_xscreensaver" == "xyes"; then
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

    if test "x$enable_monitors" == "x"; then
        if test "x$have_xrecord" == "xyes" ; then
	  enable_monitors="record"
        fi
        if test "x$have_xscreensaver" == "xyes" ; then
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
have_python_cheetah=no

if test "x$enable_dbus" != "xno"
then
   PKG_CHECK_MODULES([DBUS],
                     [gio-2.0 >= 2.26.0],
                     [config_dbus=yes],
		     [])
fi

if test "x$config_dbus" = "xyes"
then
    AC_DEFINE(HAVE_DBUS, , [DBus support])
    AC_DEFINE(HAVE_DBUS_GIO, , [DBus GIO support])

    DBUS_BACKEND="gio"    
    AC_SUBST(DBUS_BACKEND)

    AS_AC_EXPAND(DATADIR, "$datadir")
    DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
    AC_SUBST(DBUS_SERVICES_DIR)
    AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [Where services dir for DBUS is])

    AC_CHECK_PROG(PYTHON, python, python)
    AC_MSG_CHECKING([for the Cheetah Python package])

    ac_cheetah_result=`$PYTHON -c "from Cheetah.Template import Template" 2>&1`
    if test -z "$ac_cheetah_result"; then
        AC_MSG_RESULT([yes])
        have_python_cheetah=yes
    else
        AC_MSG_ERROR([No, Please install python-cheetah])
    fi
fi

AM_CONDITIONAL(HAVE_DBUS, test x$config_dbus = xyes)
AM_CONDITIONAL(HAVE_PYTHON_CHEETAH, test x$have_python_cheetah = xyes)


dnl
dnl Gnome
dnl
	
config_gnome3=no
config_panelapplet4=no

if test "x$enable_gnome3" != "xno" -a "x$config_dbus" = "xyes"
then
    PKG_CHECK_MODULES([GNOME3],
                      [libpanelapplet-4.0 \
		       gio-2.0 >= 2.26.0],
                      [config_gnome3=yes
                       config_panelapplet4=yes
                       AC_DEFINE(HAVE_PANELAPPLET4, 1, [Support for Gnome panel applet v4])],
                       [if test "x$enable_gnome3" = "xyes"; then
                           AC_MSG_ERROR([Gnome 3 development headers not found.])
                       fi])
fi

if test "x$config_gnome3" = "xyes"
then
    AC_DEFINE([HAVE_GNOME3], , [Define if Gnome 3 is available])
fi

AM_CONDITIONAL(HAVE_GNOME3, test "x$config_gnome3" = "xyes")
AM_CONDITIONAL(HAVE_PANELAPPLET4, test "x$config_panelapplet4" = "xyes")

dnl
dnl GLIB
dnl

PKG_CHECK_MODULES([GLIB],
                  [glib-2.0 >= 2.28 gmodule-2.0 gthread-2.0],
                  [AC_DEFINE(HAVE_GLIB, , [Define if GLib is available])])


config_gsettings=no		  
if test "x$enable_gsettings" != "xno" -a "x$config_gnome3" = "xyes"
then
   if test "x$platform_os_unix" == "xyes"; then		  
      GLIB_GSETTINGS
      config_gsettings=yes
      AC_DEFINE(HAVE_GSETTINGS, 1, [Have GSettings])
   fi
fi

AM_CONDITIONAL(HAVE_GSETTINGS, test "x$config_gsettings" = "xyes")

dnl
dnl SigC++
dnl

PKG_CHECK_MODULES(SIGC, sigc++-2.0 >= 2.2.4.2)


dnl
dnl GTK+
dnl

config_gtk=no
config_gtk_version=none

if test "x$platform_os_win32" = "xno"
then
PKG_CHECK_MODULES(GTK,  glib-2.0 >= 2.28.0
   			gio-2.0 >= 2.26.0
   			gtk+-3.0 >= 3.0.0
   			gdk-3.0 >= 3.0.0
                        sigc++-2.0 >= 2.2.4.2
                        glibmm-2.4 >= 2.28.0
                        gtkmm-3.0 >= 3.0.0,
                     [config_gtk=yes
		 	 config_gtk_version=3
	                 AC_DEFINE(HAVE_GTK3, 1, Support for GTK3)
                      AC_DEFINE([HAVE_APP_GTK], , [Define if GTK+ is available])], [] )
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


dnl
dnl Introspection
dnl

#if test "x$platform_os_win32" != "xyes"
#then
    GOBJECT_INTROSPECTION_CHECK([0.6.7])
#fi
#AM_CONDITIONAL(HAVE_INTROSPECTION, false)

dnl
dnl Indicator applet
dnl

config_indicator=no

INDICATOR_REQUIRED_VERSION=0.3.19
DBUSMENUGLIB_REQUIRED_VERSION=0.1.1
DBUSMENUGTK_REQUIRED_VERSION=0.3.95

if test "x$enable_indicator" != "xno" -a "x$config_gtk_version" = "x3"
then
   if test "x$found_introspection" != "xyes"
   then
	AC_MSG_ERROR([Introspection development headers not found.])
   else
       PKG_CHECK_MODULES(INDICATOR, indicator3-0.4 >= $INDICATOR_REQUIRED_VERSION
                                    dbusmenu-glib-0.4 >= $DBUSMENUGLIB_REQUIRED_VERSION
                                    dbusmenu-gtk3-0.4 >= $DBUSMENUGTK_REQUIRED_VERSION,
                         [ config_indicator=yes
                           AC_DEFINE(HAVE_INDICATOR, 1, [Support for indicator applet])],
                         [if test "x$enable_indicator" = "xyes"; then
                           AC_MSG_ERROR([Indicator development headers not found.])
                          fi])
       
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

AS_IF([test "x$with_localinstall" = "xyes"],
    INDICATORDIR="${libdir}/indicators3/7"
    INDICATORICONSDIR="${datadir}/libindicate/icons"
    ,
    INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4 | sed -e 's%/$%%'`
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
dnl GIO networking
dnl

config_network=no

if test "x$enable_network" != "xno"
then
  dnl AC_SUBST(PROTOBUF_CFLAGS)
  dnl AC_SUBST(PROTOBUF_LIBS)
  dnl PKG_CHECK_MODULES([PROTOBUF], [protobuf >= 2.4])
  dnl AC_PATH_PROG([PROTOC], [protoc], [AC_MSG_ERROR([protoc is not found])])
  
  if test "x$platform_os_unix" == "xyes"; then
     giounix="gio-unix-2.0 >= 2.32"
  fi
  PKG_CHECK_MODULES([GIO],
                    [gio-2.0 >= 2.32.0 ${giounix}],
                    [config_network=yes],
 		    [if test "x$enable_network" = "xyes"; then
                     	  AC_MSG_ERROR([GIO development headers not found.])
                    fi])
fi

if test $config_network = yes
then
  AC_DEFINE([HAVE_NETWORK], , [Define if network support is available])
fi

AM_CONDITIONAL(HAVE_NETWORK, test "x$config_network" = "xyes")

AC_CHECK_HEADER(linux/netlink.h,
have_netlink=yes
AC_DEFINE([HAVE_NETLINK],[],[Define for Linux Netlink is available])
, [], [
#include <sys/socket.h>
#include <asm/types.h>
])

AM_CONDITIONAL(HAVE_NETLINK, test x"$have_netlink" = "xyes")

dnl
dnl GStreamer
dnl

config_gstreamer=no

if test "x$enable_gstreamer" != "xno";
then
    PKG_CHECK_MODULES([GSTREAMER],
                      [gstreamer-1.0],
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

AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

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
dnl Defines
dnl

config_tests=no
if test "x$enable_tests" = "xyes"; then
   config_tests=yes
   AC_DEFINE([HAVE_TESTS], , [Define if test code is enabled])
fi
AM_CONDITIONAL(HAVE_TESTS, test "x$config_tests" = "xyes")

config_experimental=no
if test "x$enable_experimental" = "xyes"; then
   config_experimental=yes
   AC_DEFINE(HAVE_EXPERIMENTAL,,[Define if experimental features are available])
fi
AM_CONDITIONAL(HAVE_EXPERIMENTAL, test "x$config_experimental" = "xyes")

dnl
dnl Generate language list.
dnl

if test "x$platform_os_win32" = "xyes"
then
    LANGFILE="./frontend/gtkmm/win32/setup/lang.iss"

    ALL_LINGUAS="$(cat ./po/LINGUAS |grep -v '^#' | tr '\n' ' ' )"
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
             libs/Makefile
             libs/config/Makefile
             libs/config/include/Makefile
             libs/config/include/config/Makefile
             libs/config/src/Makefile
             libs/config/src/unix/Makefile
             libs/config/src/win32/Makefile
             libs/config/src/osx/Makefile
             libs/utils/Makefile
             libs/utils/bin/Makefile
             libs/utils/include/Makefile
             libs/utils/include/utils/Makefile
             libs/utils/src/Makefile
             libs/utils/src/win32/Makefile
             libs/dbus/Makefile
             libs/dbus/bin/Makefile
             libs/dbus/include/Makefile
             libs/dbus/include/dbus/Makefile
             libs/dbus/src/Makefile
             libs/dbus/data/Makefile
             libs/input-monitor/Makefile
             libs/input-monitor/include/Makefile
             libs/input-monitor/include/input-monitor/Makefile
             libs/input-monitor/src/Makefile
             libs/input-monitor/src/unix/Makefile
             libs/input-monitor/src/win32/Makefile
             libs/input-monitor/src/osx/Makefile
             libs/hooks/Makefile
             libs/hooks/harpoon/Makefile
             libs/hooks/harpoon/include/Makefile
             libs/hooks/harpoon/src/Makefile
             libs/hooks/harpoonHelper/Makefile
             libs/hooks/harpoonHelper/include/Makefile
             libs/hooks/harpoonHelper/src/Makefile
             libs/audio/Makefile
             libs/audio/include/Makefile
             libs/audio/include/audio/Makefile
             libs/audio/src/Makefile
             libs/audio/src/win32/Makefile
             libs/audio/src/osx/Makefile
             backend/Makefile
             backend/test/Makefile
             backend/src/Makefile
	     backend/src/org.workrave.gschema.xml.in
             backend/include/Makefile
             frontend/Makefile
             frontend/common/Makefile
             frontend/common/include/Makefile
             frontend/common/src/Makefile
             frontend/data/Makefile
             frontend/data/sounds/Makefile
             frontend/data/sounds/subtle/Makefile
             frontend/data/sounds/default/Makefile
             frontend/data/images/Makefile
             frontend/data/images/16x16/Makefile
             frontend/data/images/24x24/Makefile
             frontend/data/images/32x32/Makefile
             frontend/data/images/48x48/Makefile
             frontend/data/images/64x64/Makefile
             frontend/data/images/96x96/Makefile
             frontend/data/images/128x128/Makefile
             frontend/data/images/scalable/Makefile
             frontend/data/images/win32/Makefile
             frontend/data/images/osx/Makefile
             frontend/data/exercises/Makefile
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
             frontend/applets/gnome-shell/Makefile
             frontend/applets/gnome-shell/src/Makefile
             frontend/gtkmm/Makefile
             frontend/gtkmm/osx/Makefile
             frontend/gtkmm/win32/Makefile
             frontend/gtkmm/win32/setup/Makefile
             frontend/gtkmm/win32/setup/setup.iss
             frontend/gtkmm/win32/setup/src/Makefile
             frontend/gtkmm/src/Makefile
	     frontend/gtkmm/src/org.workrave.gui.gschema.xml.in
             frontend/gtkmm/src/win32/Makefile
             frontend/gtkmm/src/unix/Makefile
             frontend/gtkmm/src/osx/Makefile
	     frontend/apps/qt5/src/Makefile
	     frontend/apps/qt5/Makefile
	     frontend/apps/common/Makefile
	     frontend/apps/common/src/Makefile
	     frontend/apps/common/include/Makefile
	     frontend/apps/Makefile
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
echo ""
if test "$platform_os_unix" = "yes"; then
echo "            Activity Monitors :   ${enable_monitors}"
echo ""
fi
echo "                      Gtk GUI :   ${config_gtk} (Gtk${config_gtk_version})"
echo "              Gnome 3 support :   ${config_gnome3}"
echo "GObject-Introspection support :   ${found_introspection}"
echo "            Indicator support :   ${config_indicator}"
echo "                 DBUS support :   ${config_dbus}"
echo "            GStreamer support :   ${config_gstreamer}"
if test "$platform_os_unix" = "yes"; then
echo "           Pulseaudio support :   ${config_pulse}"
fi
echo ""
echo "        Experimental features :   ${config_experimental}"
echo "                    Debugging :   ${config_debug}"
echo "                   Debug logs :   ${config_tracing}"
echo "                    Test code :   ${config_tests}"
echo ""

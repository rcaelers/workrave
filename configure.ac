dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
dnl
dnl
dnl $Id$
dnl

AC_INIT(backend/src/Core.cc)

dnl m4_pattern_allow(AM_PATH_ORBIT2)

dnl Demand at least version 2.57 of autoconf.
AC_PREREQ(2.57)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(workrave, 1.6.2)

                                   
dnl some command line options.                           
GCONF_LIBS=""
AC_ARG_ENABLE( app-gtk,
  [  --disable-app-gtk       compile without Gtk/Gnome support],
       config_with_gtk=$enableval
       ,
       config_with_gtk=yes
    )

AC_ARG_ENABLE( app-text,
  [  --enable-app-text       compile with Dummy text GUI support],
       config_with_text=$enableval
       ,
       config_with_text=no
    )
    
AC_ARG_ENABLE( gconf,
  [  --enable-gconf          compile with Gnome gconf2 support],
       config_with_gconf=$enableval
       ,
       config_with_gconf=no
    )

AC_ARG_ENABLE( xml,
  [  --enable-xml            compile with XML support],
       config_with_xml=$enableval
       ,
       config_with_xml=no
    )

AC_ARG_ENABLE( gnome,
  [  --disable-gnome         compile without Gnome support],
       config_with_gnome=$enableval
       ,
       config_with_gnome=yes
    )

AC_ARG_ENABLE( registry,
  [  --enable-registry       compile with Win32 registry support],
       config_with_registry=$enableval
       ,
       config_with_registry=no
    )
    
AC_ARG_ENABLE( debug,
  [  --enable-debug          compile with debugging flags set],
       config_debug=$enableval
       ,
       config_debug=no
    )

AC_ARG_ENABLE( distribution,
  [  --disable-distribution  compile without support for distributed operation],
       config_distribution=$enableval
       ,
       config_distribution=yes
    )

AC_ARG_ENABLE( exercises,
  [  --disable-exercises      compile without exercises support],
       config_exercises=$enableval
       ,
       config_exercises=yes
    )

AC_ARG_ENABLE( experimental,
  [  --disable-experimental  compile without experimental features],
       config_experimental=$enableval
       ,
       config_experimental==yes
    )

if test "x$config_debug" = "xyes"; then
   CFLAGS="${CFLAGS} -g -Wall"
   CXXFLAGS="${CXXFLAGS} -g -Wall"
   LDFLAGS="${LDFLAGS} -g"
else
   CFLAGS="${CFLAGS} -Os -g -DNDEBUG"
   CXXFLAGS="${CXXFLAGS} -Os -g -DNDEBUG"
   LDFLAGS="${LDFLAGS} -g"
fi
                   
AC_ARG_ENABLE( cwd,
  [  --enable-cwd            compile with libcwd],
       config_cwd_debug=$enableval
       ,
       config_cwd_debug=no
    )
                               
dnl Include --enable-maintainer-mode option.
AM_MAINTAINER_MODE
                                                      
dnl Update version and date.                           
AC_SUBST(VERSION)
ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST
AC_PREFIX_PROGRAM(workrave)

dnl Set of available languages.
ALL_LINGUAS="nl de eo pl da es zh_TW ru fr pt_BR no"

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_ISC_POSIX
dnl AM_PROG_LIBTOOL

dnl Checks for programs.
AC_CHECK_PROGS(M4, gm4 m4, m4)
AC_CHECK_DOXYGEN

dnl Copied from the official gtk+-2 configure.in
AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    config_with_gnome=no
    ;;
  *)
    platform_win32=no
    config_with_registry=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

PLATFORM_WIN32=
if test "$platform_win32" = "yes"; then
   AC_DEFINE(PLATFORM_WIN32,,[Define if this is compiled for some Win32 platform])
fi

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    native_win32=yes
    LIBSTDCPP_LIBS=-lstdc++
    ;;
  *)
    native_win32=no
    LIBSTDCPP_LIBS=
    ;;
esac
AC_SUBST(LIBSTDCPP_LIBS)
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(PLATFORM_NATIVE_WIN32, test "$native_win32" = "yes")

PLATFORM_NATIVE_WIN32=
if test "$native_win32" = "yes"; then
   AC_DEFINE(PLATFORM_NATIVE_WIN32,,[Define if this is compiled for a native Win32 platform])
fi

if test "x$config_with_registry" = "xno" -a "x$config_with_xml" = "xno" -a "x$config_with_gconf" = "xno" ; then
    if test "x$platform_win32" = "xyes" ; then
       config_with_registry=yes
    else
       config_with_gconf=yes
    fi
fi
    
dnl Various compiler setup.
AC_C_INLINE
AC_C_CONST

# Checks for header files.
AC_PATH_X
AC_PATH_XTRA
AC_STDC_HEADERS
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdlib.h sys/time.h sys/select.h unistd.h])
AC_CHECK_HEADER(pthread.h,,)
                              
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

AC_CHECK_MEMBER(struct timespec.tv_sec, AC_DEFINE(HAVE_STRUCT_TIMESPEC,,[struct timespec]),, [dnl
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
])

AC_CHECK_MEMBER(struct timezone.tz_minuteswest,AC_DEFINE(HAVE_STRUCT_TIMEZONE,,[struct timezone]),, [dnl
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
])

AC_CHECK_MEMBER(MOUSEHOOKSTRUCT.hwnd,AC_DEFINE(HAVE_STRUCT_MOUSEHOOKSTRUCT,,[struct MOUSEHOOKSTRUCT]),, [#include <windows.h>])

# Checks for library functions.
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([gettimeofday nanosleep select])

PKG_CHECK_MODULES(GLIB, glib-2.0)

AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

if test "x$config_with_gtk" = "xyes"
then
  AC_SUBST(SIGC_LIBS)
  AC_SUBST(SIGC_CFLAGS)

  PKG_CHECK_MODULES(GTK, gtk+-2.0)

  AC_SUBST(GTK_LIBS)
  AC_SUBST(GTK_CFLAGS)

  PKG_CHECK_MODULES(GLIBMM, glibmm-2.4,, :)
  PKG_CHECK_MODULES(GTKMM, gtkmm-2.4,, :)

  HAVE_GTKMM24=
  if test "x$GLIBMM_LIBS" = "x" -o "x$GTKMM_LIBS" = "x"
  then
        PKG_CHECK_MODULES(SIGC, sigc++-1.2)
        PKG_CHECK_MODULES(GLIBMM, glibmm-2.0)
        PKG_CHECK_MODULES(GTKMM, gtkmm-2.0)
  else
        PKG_CHECK_MODULES(SIGC, sigc++-2.0)
        AC_DEFINE(HAVE_GTKMM24,,[Define if gtk2.4 is used])
  fi

  AC_SUBST(GLIBMM_LIBS)
  AC_SUBST(GLIBMM_CFLAGS)

  AC_SUBST(GTKMM_LIBS)
  AC_SUBST(GTKMM_CFLAGS)

  if test "x$config_with_gnome" = "xyes"
  then
    PKG_CHECK_MODULES(GNOMEUIMM, libgnomeuimm-2.6,,:)

    AC_SUBST(GNOMEUIMM)

    if test "x$GNOMEUIMM_LIBS" = "x"
    then
        PKG_CHECK_MODULES(GNOMEUIMM, libgnomeuimm-2.0)
    fi

    PKG_CHECK_MODULES(GNOMEUI, libgnomeui-2.0)
    PKG_CHECK_MODULES(APPLET, libpanelapplet-2.0 >= 2.0.10)
    PKG_CHECK_MODULES(BONOBO, libbonobo-2.0 >= 2.0.0)

    AM_PATH_ORBIT2(2.8.3)

    AC_MSG_CHECKING(for CORBA include paths)
    IDL_INCLUDES="-I "`pkg-config --variable=idldir libbonobo-2.0`" -I "`pkg-config --variable=idldir bonobo-activation-2.0`
    AC_MSG_RESULT($IDL_INCLUDES)
    AC_SUBST(IDL_INCLUDES)

  fi

  dnl
  dnl check if we have gtk+ 2.1.x (again - for multihead support)
  dnl
  AC_MSG_CHECKING([for gtk+ multihead support])
  if test "$platform_win32" != "yes"; then
    if $PKG_CONFIG --atleast-version 2.1.0 gtk+-2.0; then
      AC_DEFINE(HAVE_GTK_MULTIHEAD,,[gtk+ with multihead support found])
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  else
    if $PKG_CONFIG --atleast-version 2.2.4 gtk+-2.0; then
      AC_DEFINE(HAVE_GTK_MULTIHEAD,,[gtk+ with multihead support found])
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi
fi

AC_SUBST(GNOMEUIMM_LIBS)
AC_SUBST(GNOMEUIMM_CFLAGS)
        
AC_SUBST(GNOMEUI_LIBS)
AC_SUBST(GNOMEUI_CFLAGS)

AC_SUBST(APPLET_CFLAGS)
AC_SUBST(APPLET_LIBS)
                
PKG_CHECK_MODULES(GMODULE, gmodule-2.0)

AC_SUBST(GMODULE_LIBS)
AC_SUBST(GMODULE_CFLAGS)

PKG_CHECK_MODULES(GTHREAD, gthread-2.0)

AC_SUBST(GTHREAD_LIBS)
AC_SUBST(GTHREAD_CFLAGS)

GCONF_LIBS=""
GCONF_CFLAGS=""
                        
if test "x$config_with_gconf" = "xyes"
then
  PKG_CHECK_MODULES(GCONF, gconf-2.0)
fi
                                
AC_SUBST(GCONF_LIBS)
AC_SUBST(GCONF_CFLAGS)

if test "x$config_distribution" = "xyes"
then
dnl AM_PATH_GNET_2_0(2.0.0,[] ,[] )
PKG_CHECK_MODULES(GNET, gnet-2.0)
GNET2=yes                          
dnl AM_PATH_GNET(1.1.7,[] ,[] )
fi
        
AC_SUBST(GNET_LIBS)
AC_SUBST(GNET_CFLAGS)

        
if test "x$GNET_LIBS" == "x"; then
   config_distribution=no
fi

                
dnl AC_CHECK_LIB(Xmu,XmuClientWindow,LIBS="$LIBS -lXmu",
dnl   AC_MSG_ERROR(Xmuxd library required))

LIBS_save=$LIBS
LIBS="$LIBS -L $ac_x_libraries"
AC_CHECK_LIB(Xtst, XRecordEnableContext,
                   X_LIBS="$X_LIBS -lXtst" AC_DEFINE(HAVE_XRECORD,,[Define if the RECORD extension is available]),
)
LIBS=$LIBS_save

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_CXX_EH
if test "x$ac_cv_cxx_eh" != "xyes"
then
        AC_MSG_ERROR([$CXX does provide correct exception handling])
fi

AC_CHECK_CXX_NS
if test "x$ac_cv_cxx_ns" != "xyes"
then
        AC_MSG_ERROR([$CXX does not support namespaces])
fi

AC_CHECK_CXX_STL
if test "x$ac_cv_cxx_stl" != "xyes"
then
        AC_MSG_ERROR([STL is not available])
fi
        
dnl Some magic copied from xmms
if test "x$platform_win32" != "xyes"; then
        PTHREAD_LIBS=error
        AC_MSG_CHECKING(for old style FreeBSD -pthread flag)
        AC_EGREP_CPP(yes,
        	[#if defined(__FreeBSD_cc_version) && __FreeBSD_cc_version <= 500001
        	  yes
        	#endif
        	], AC_MSG_RESULT(yes)
        	CFLAGS="$CFLAGS -D_THREAD_SAFE" PTHREAD_LIBS="-pthread",
        	AC_MSG_RESULT(no))
        if test "x$PTHREAD_LIBS" = xerror; then
        	AC_CHECK_LIB(pthread, pthread_attr_init,
        		PTHREAD_LIBS="-lpthread")
        fi
        if test "x$PTHREAD_LIBS" = xerror; then
                AC_CHECK_LIB(pthreads, pthread_attr_init,
        		PTHREAD_LIBS="-lpthreads")
        fi
        if test "x$PTHREAD_LIBS" = xerror; then
                AC_CHECK_LIB(c_r, pthread_attr_init,
        		PTHREAD_LIBS="-lc_r")
        fi
        if test "x$PTHREAD_LIBS" = xerror; then
        	AC_CHECK_FUNC(pthread_attr_init, PTHREAD_LIBS="")
        fi
                
        if test "x$PTHREAD_LIBS" = "xerror" -a "x$platform_win32" = "xno" ; then
        	AC_MSG_ERROR([[no working posix thread library]])
        fi
        AC_SUBST(PTHREAD_LIBS)
fi        
        
dnl NOT-QUITE-YET (macro does not exists on win32)        
dnl AM_PATH_ESD(0.2.28, 
dnl    [have_esd=yes 
dnl     AC_CHECK_LIB(esd, esd_get_latency, 
dnl        AC_DEFINE(HAVE_ESD_GET_LATENCY,,
dnl           [Define if libesd has the esd_get_latency function]))], 
dnl    have_esd=no)

AM_CONDITIONAL(HAVE_ESD,test "x$have_esd" = xyes)

dnl some more magic...
CFLAGS_save=$CFLAGS
LIBS_save=$LIBS
CFLAGS="$CFLAGS $GTK_CFLAGS"
LIBS="$LIBS $GTK_LIBS"
AC_MSG_CHECKING(for thread-safe Xlib)
AC_CACHE_VAL(thread_safe_xlibs,
[AC_TRY_RUN([
#include <X11/Xlib.h>
int main()
{
   exit(!(XInitThreads()));
}],
thread_safe_xlibs=yes,
thread_safe_xlibs=no,
thread_safe_xlibs=no)])
AC_MSG_RESULT($thread_safe_xlibs)
CFLAGS=$CFLAGS_save
LIBS=$LIBS_save

dnl internationalization macros
AM_GNU_GETTEXT([external])
dnl AM_GLIB_GNU_GETTEXT

dnl Hack because windows gettext package is SEVERELY broken...
if test "x$platform_win32" == "xyes"; then
        USE_NLS=yes
        AC_DEFINE(ENABLE_NLS,,[Define to 1 if translation of program messages to the user's native
   language is requested.])
fi

GETTEXT_PACKAGE=workrave
AC_SUBST(GETTEXT_PACKAGE)

AC_DEFINE(GETTEXT_PACKAGE, "workrave", [Blaat])
AC_PROG_INTLTOOL


if test "x$config_with_xml" = "xyes"
then
    AC_PATH_PROG(GDOME_CONFIG, gdome-config, no)
    if test "$GDOME_CONFIG" = no; then
    	if test x$2 = xfailure; then
    		AC_MSG_ERROR(Could not find gdome-config)
    	fi
        AC_MSG_ERROR(Need GDome2)
    fi
    GDOME_CFLAGS=`$GDOME_CONFIG --cflags`
    AC_CHECK_LIB(gdome, gdome_el_nodeType, [
    	dnl $1
    	GDOME_LIB=`$GDOME_CONFIG --libs`
    ], [
    	if test x$2 = xfailure; then 
    		AC_MSG_ERROR(Could not link sample gdome program)
    	fi
    ], `$GDOME_CONFIG --libs`)
else
    GDOME_LIB="";
    GDOME_CFLAGS=""
fi
AC_SUBST(GDOME_CFLAGS)
AC_SUBST(GDOME_LIB)
    
dnl Set language requirements for the tests below.
AC_LANG_CPLUSPLUS
AC_REQUIRE_CPP
    
dnl Configuration defines
if test "$config_cwd_debug" = yes; then

  dnl Check if we have libcwd
  AC_CHECK_LIB(cwd, __libcwd_version, [LIBS="-lcwd $LIBS"], [AC_CHECK_LIB(cwd, main,,
      [AC_MSG_ERROR([
  --enable-debug: You need to have libcwd installed to enable debugging.
  Or perhaps you need to add its location to LDFLAGS and CPPFLAGS, for example:
  LDFLAGS=\"-L/usr/local/lib\" CPPFLAGS=\"-I/usr/local/include\" ./configure])])])

  CXXFLAGS="-DCWDEBUG $CXXFLAGS"
  AC_DEFINE(HAVE_LIBCWD)
fi

          
WR_BACKEND_LIBS="\$(top_srcdir)/backend/src/libworkrave-backend.a"
WR_BACKEND_CFLAGS="-I \$(top_srcdir)/backend/include -I \$(top_srcdir)/backend/src/"
WR_BACKEND_INCLUDES="-I \$(top_srcdir)/backend/include"
WR_COMMON_LIBS="\$(top_srcdir)/common/src/libworkrave-common.a"
WR_COMMON_CFLAGS="-I \$(top_srcdir)/common/include"

if test "x$platform_win32" = "xyes"; then
   WR_COMMON_LIBS="\$(top_srcdir)/common/src/win32/libworkrave-common-win32.a $WR_COMMON_LIBS"
   WR_COMMON_CFLAGS="$WR_COMMON_CFLAGS -I \$(top_srcdir)/common/include/win32"

   WR_BACKEND_LIBS="$WR_BACKEND_LIBS \$(top_srcdir)/backend/src/win32/libworkrave-backend-win32.a"
   WR_BACKEND_CFLAGS="$WR_BACKEND_CFLAGS -I \$(top_srcdir)/backend/src/win32"
else
   WR_COMMON_LIBS="\$(top_srcdir)/common/src/unix/libworkrave-common-unix.a $WR_COMMON_LIBS"
   WR_COMMON_CFLAGS="$WR_COMMON_CFLAGS -I \$(top_srcdir)/common/include/unix"

   WR_BACKEND_LIBS="$WR_BACKEND_LIBS \$(top_srcdir)/backend/src/unix/libworkrave-backend-unix.a"
   WR_BACKEND_CFLAGS="$WR_BACKEND_CFLAGS -I \$(top_srcdir)/backend/src/unix"
fi   

WR_FRONTEND_LIBS="$WR_BACKEND_LIBS $WR_COMMON_LIBS \$(top_srcdir)/frontend/common/src/libworkrave-frontend-common.a $WR_COMMON_LIBS"
WR_FRONTEND_CFLAGS="$WR_COMMON_CFLAGS $WR_BACKEND_CFLAGS -I \$(top_srcdir)/frontend/common/include/"
if test "x$platform_win32" = "xyes"; then
   WR_FRONTEND_LIBS="$WR_FRONTEND_LIBS \$(top_srcdir)/frontend/common/src/win32/libworkrave-frontend-common-win32.a"
   WR_FRONTEND_CFLAGS="$WR_FRONTEND_CFLAGS -I \$(top_srcdir)/frontend/common/include/win32"
fi
if test "x$config_with_gnome" = "xyes"; then
   WR_FRONTEND_LIBS="$WR_FRONTEND_LIBS \$(top_srcdir)/frontend/common/src/gnome/libworkrave-frontend-common-gnome.a"
   WR_FRONTEND_CFLAGS="$WR_FRONTEND_CFLAGS -I \$(top_srcdir)/frontend/common/include/gnome"
fi   
                                             
AC_SUBST(WR_FRONTEND_LIBS)
AC_SUBST(WR_FRONTEND_CFLAGS)

AC_SUBST(WR_COMMON_DIRS)
AC_SUBST(WR_COMMON_LIBS)
AC_SUBST(WR_COMMON_CFLAGS)
 
AC_SUBST(WR_BACKEND_DIRS)
AC_SUBST(WR_BACKEND_LIBS)
AC_SUBST(WR_BACKEND_CFLAGS)
AC_SUBST(WR_BACKEND_INCLUDES)
       
HAVE_X=
if test x$have_x = xyes ; then
   AC_DEFINE(HAVE_X,,[Define if X is available])
fi

HAVE_GCONF=
if test "x$GCONF_LIBS" != "x" -a "x$config_with_gconf" = "xyes"; then
   AC_DEFINE(HAVE_GCONF,,[Define if GConf is available])
fi

HAVE_GDOME=
if test "x$GDOME_LIB" != "x" -a "x$config_with_xml" = "xyes"; then
   AC_DEFINE(HAVE_GDOME,,[Define if GDome is available])
fi

HAVE_REGISTRY=
if test "x$config_with_registry" = "xyes"; then
   AC_DEFINE(HAVE_REGISTRY,,[Define if registry is available])
fi

HAVE_GNOME=
if test "x$config_with_gnome" = "xyes"; then
   AC_DEFINE(HAVE_GNOME,,[Define if Gnome is available])
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
        
HAVE_GNET=
if test "x$GNET_LIBS" != "x"; then
   AC_DEFINE(HAVE_GNET,,[Define if GNet is available])
fi

HAVE_GNET2=
if test "x$GNET2" != "x"; then
   AC_DEFINE(HAVE_GNET2,,[Define if GNet2 is available])
fi
        
HAVE_APP_GTK=
if test "x$GTKMM_LIBS" != "x"; then
   AC_DEFINE(HAVE_APP_GTK,,[Define if Gtk is available])
fi

HAVE_APP_TEXT=
if test "x$config_with_text" = "xyes"; then
   AC_DEFINE(HAVE_APP_TEXT,,[Define if Text is available])
fi
                                
WIN32CONSOLE=
if test "x$config_debug" != "xyes" -a "x$platform_win32" = "xyes" ; then
        WIN32CONSOLE=-mwindows
fi
AC_SUBST(WIN32CONSOLE)

AM_CONDITIONAL(HAVE_X, test x$have_x = xyes)
AM_CONDITIONAL(HAVE_GCONF, test "x$GCONF_LIBS" != "x" -a "x$config_with_gconf" = "xyes")
AM_CONDITIONAL(HAVE_GDOME, test "x$GDOME_LIB" != "x" -a "x$config_with_xml" = "xyes")
AM_CONDITIONAL(HAVE_GNOME, test "x$config_with_gnome" = "xyes")
AM_CONDITIONAL(HAVE_GNET, test "x$GNET_LIBS" != "x")
AM_CONDITIONAL(HAVE_REGISTRY, test "x$config_with_registry" = "xyes")
AM_CONDITIONAL(NDEBUG, test "x$config_debug" != "xyes")
AM_CONDITIONAL(HAVE_WIN32, test "x$platform_win32" = "xyes")
AM_CONDITIONAL(HAVE_UNIX, test "x$platform_win32" != "xyes")
AM_CONDITIONAL(HAVE_APP_GTKMM, test "x$GTKMM_LIBS" != "x")
AM_CONDITIONAL(HAVE_APP_TEXT, test "x$config_with_text" = "xyes")
AM_CONDITIONAL(HAVE_DISTRIBUTION, test "x$config_distribution" = "xyes")
AM_CONDITIONAL(HAVE_EXERCISES, test "x$config_exercises" = "xyes")
AM_CONDITIONAL(HAVE_EXPERIMENTAL, test "x$config_experimental" = "xyes")
                                                           

AC_OUTPUT(Makefile \
          backend/Makefile \
          backend/src/Makefile \
          backend/src/unix/Makefile \
          backend/src/win32/Makefile \
          backend/include/Makefile \
          common/Makefile \
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
          frontend/Makefile \
          frontend/common/Makefile \
          frontend/common/include/Makefile \
          frontend/common/share/Makefile \
          frontend/common/share/sounds/Makefile \
          frontend/common/share/images/Makefile \
          frontend/common/src/Makefile \
          frontend/common/src/gnome/Makefile \
          frontend/common/src/win32/Makefile \
          frontend/win32/Makefile \
          frontend/win32/applet/Makefile \
          frontend/win32/applet/src/Makefile \
          frontend/win32/applet/include/Makefile \
          frontend/gtkmm/Makefile \
          frontend/gtkmm/win32/Makefile \
          frontend/gtkmm/win32/setup/Makefile \
          frontend/gtkmm/win32/setup/setup.iss \
          frontend/gtkmm/src/Makefile \
          frontend/gtkmm/src/win32/Makefile \
          frontend/gtkmm/src/unix/Makefile \
          frontend/gtkmm/src/gnome_applet/Makefile \
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
          po/POTFILES \
          po/Makefile.in \
          po/Makefile \
          m4/Makefile \
          workrave.spec)

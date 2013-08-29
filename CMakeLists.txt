cmake_minimum_required(VERSION 2.8)

set (VERSION "1.11.001")
set (RESOURCE_VERSION "1,11,001,0")

set (PACKAGE_VERSION ${VERSION})

#
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build/cmake/modules")

#
set(CMAKE_BUILD_TYPE_INIT "Debug")

# 
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
#include(build/cmake/macros.cmake)

find_package(PkgConfig REQUIRED)

include(CheckLibraryExists)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag) 
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
#----------------------------------------------------------------------------------------------------

project(Workrave)

# TODO: GIT_VERSION

#----------------------------------------------------------------------------------------------------
# Define output location.

#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)

#----------------------------------------------------------------------------------------------------
# Macros

macro(pkgconfig_get_var OUT PKG VARNAME)
  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${VARNAME} ${PKG}
    OUTPUT_VARIABLE _result
    RESULT_VARIABLE _null
  )

  if (_null)
  else()
    string(REGEX REPLACE "[\r\n]" " " _result "${_result}")
    string(REGEX REPLACE " +$" ""  _result "${_result}")
    separate_arguments(_result)
    set(${OUT} ${_result} CACHE INTERNAL "")
  endif()
endmacro()

#----------------------------------------------------------------------------------------------------
# Compiler

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11) 
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
CHECK_C_COMPILER_FLAG("-std=c99" COMPILER_SUPPORTS_C99)

if(COMPILER_SUPPORTS_CXX11) 
  set(CXX11_FLAGS -std=c++11)
elseif(COMPILER_SUPPORTS_CXX0X) 
  set(CXX11_FLAGS -std=c++01)
else() 
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.") 
endif()

if (CXX11_FLAGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")
  CHECK_CXX_COMPILER_FLAG("${CXX11_FLAGS} -stdlib=libc++" HAS_LIBCXX11)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O0")

if(COMPILER_SUPPORTS_C99) 
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

#----------------------------------------------------------------------------------------------------
# GTK

option(WITH_GTK "Enable GTK Workrave application" ON)

if (WITH_GTK)
  pkg_check_modules(
    GTK REQUIRED
    gtkmm-3.0>=3.6.0
    gtk+-3.0>=3.6.0)
  
  set (HAVE_GSETTINGS ON)
  set (HAVE_GLIB ON)
  set (HAVE_GTK ON)
  set (HAVE_GTK3 ON)
  set (HAVE_DBUS ON)
  set (HAVE_APP_GTK ON)

  pkg_check_modules(
    GLIB REQUIRED
    glib-2.0>=2.36.0
    gio-2.0>=2.26.0)
 
  include_directories(${GLIB_INCLUDE_DIRS})
endif()

include (GSettings)

#----------------------------------------------------------------------------------------------------
# GDome XML support

option(WITH_XML "Enable XML Support" ON)

if (WITH_XML)
  pkg_check_modules(GDOME gdome2)
  if (GDOME_FOUND)
    set (HAVE_GDOME ON)
  else()
    set (WITH_XML OFF)
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Gnome

option(WITH_GCONF "Enable GConf support" ON)

if (WITH_GCONF)
  pkg_check_modules(GCONF gconf-2.0)
  if (GCONF_FOUND)
    set (HAVE_GCONF ON)
  else()
    set (WITH_GCONF OFF)
  endif()
endif()

option(WITH_GNOME_PANEL "Enable Gnome 3 Classic pannel applet support" ON)

if (WITH_PANELAPPLET4)
  pkg_check_modules(PANELAPPLET4 libpanelapplet-4.0 gio-2.0>=2.26.0)
  if (PANELAPPLET4_FOUND)
    set (HAVE_PANELAPPLET4 ON)
    set (HAVE_GNOME3 ON)
  else()
    set (WITH_PANELAPPLET4 OFF)
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Qt5

option(WITH_QT5 "Enable Qt5 Workrave application" ON)

if (WITH_QT5)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)

  pkg_check_modules(
    GLIB REQUIRED
    glib-2.0>=2.36.0)

  include_directories(${Qt5Core_INCLUDE_DIRS})
  include_directories(${Qt5Gui_INCLUDE_DIRS})
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  

  # Can be removed with Qt 5.1
  include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -Wno-deprecated-register")
  set (HAVE_QT5 ON)
  set (HAVE_APP_QT5 ON)

   # FIXME: temporary
  set (HAVE_GLIB ON)
  include_directories(${GLIB_INCLUDE_DIRS})

endif()


#----------------------------------------------------------------------------------------------------
# DBus

if (HAVE_DBUS)
  find_package(PythonInterp)
  if (NOT PYTHONINTERP_FOUND)
    set (HAVE_DBUS FALSE)
  else()
    include(DBus)
  endif()
endif()

# TODO: check python Cheetah

#----------------------------------------------------------------------------------------------------
# DBus

option(WITH_GSTREAMER "Enable GStreamer audio" ON)

if (WITH_GSTREAMER)
  pkg_check_modules(GSTREAMER gstreamer-1.0)
  if (GSTREAMER_FOUND)
    set (HAVE_GSTREAMER ON)
  else()
    pkg_check_modules(GSTREAMER gstreamer-0.10)
    if (GSTREAMER_FOUND)
      set (HAVE_GSTREAMER ON)
    else()
      set (WITH_GSTREAMER OFF)
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Pulseaudio

option(WITH_PULSE "Enable Pulseaudio Mixer support" ON)

if (WITH_PULSE)
  pkg_check_modules(PULSE glib-2.0 libpulse libpulse-mainloop-glib)
  if (PULSE_FOUND)
    set (HAVE_PULSE ON)
  else()
    set (WITH_PULSE OFF)
  endif()
endif()


#----------------------------------------------------------------------------------------------------
# Indicator

option(WITH_INDICATOR "Enable Unity indicator support" ON)
option(LOCALINSTALL "Install file locally instead of to system location" OFF)

if (WITH_INDICATOR)
  find_package(GObjectIntrospection 0.9.12)
  set(HAVE_INTROSPECTION ${INTROSPECTION_FOUND})

  if (HAVE_INTROSPECTION)
    pkg_check_modules(INDICATOR indicator3-0.4 dbusmenu-glib-0.4 dbusmenu-gtk3-0.4)
  endif()
    
  if (INDICATOR_FOUND)
    set(HAVE_INDICATOR ON)
  else()
    set(WITH_INDICATOR OFF)
  endif()
endif()

if (HAVE_INDICATOR)
  if (LOCALINSTALL)
    set(INDICATORDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/indicators3/7")
    set(INDICATORICONSDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/libindicate/icons")
  else()
    pkgconfig_get_var(INDICATORDIR indicator3-0.4 indicatordir)
    pkgconfig_get_var(INDICATORICONSDIR indicator3-0.4 iconsdir)

    #string(REGEX REPLACE "/$" "" INDICATORDIR "${INDICATORDIR}")
    #set(INDICATORDIR ${INDICATORDIR} CACHE INTERNAL "")
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Misc

find_package(Boost 1.50.0 REQUIRED COMPONENTS system thread)

#----------------------------------------------------------------------------------------------------
# Translations

set(ALL_LINGUAS "")
file(STRINGS "${CMAKE_SOURCE_DIR}/po/LINGUAS" pofile)
foreach(line ${pofile})
  if("${line}" MATCHES "^[a-zA-Z]")
    set(ALL_LINGUAS "${ALL_LINGUAS}${line} ")
  endif()
endforeach()
string(STRIP ${ALL_LINGUAS} ALL_LINGUAS)

find_package(Gettext)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

if (NOT GETTEXT_FOUND)
  message("=== GetText not found - disabling i18n")
else()
  set(ENABLE_NLS ON)
endif (NOT GETTEXT_FOUND)

set (GETTEXT_PACKAGE "workrave")

# TODO: generate languages.iss for InnoSetup

#----------------------------------------------------------------------------------------------------
# Options

include(Intltool)

# TODO: HAVE_TESTS

#----------------------------------------------------------------------------------------------------
# Tracing

option(WITH_TRACING "Enable debug tracing" OFF)

if (WITH_TRACING)
  set(TRACING ON)
  #SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTRACING")
endif()

#----------------------------------------------------------------------------------------------------
# platform

if (WIN32)
  set (PLATFORM_OS_WIN32 ON)
  if (MSVC)
    set(PLATFORM_OS_WIN32_NATIVE 1)
  endif (MSVC)
  
  set(HAVE_LANGUAGE_SELECTION TRUE)

  if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  
    SET(MSVC_WARNINGS "/W3 /wd4127 /wd4250 /wd4413 /wd4800")
    SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /FIconfig.h ${MSVC_WARNINGS}")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /FIconfig.h ${MSVC_WARNINGS}")

    add_definitions( 
      -DWIN32
      -D_WINDOWS
      -D_WIN32_WINNT=0x0600
      ) 
  endif()

  check_include_files(dsound.h HAVE_DSOUND)

  check_c_source_compiles(
    "#include <mmdeviceapi.h>
     #include <endpointvolume.h>

     static REFIID foo = __uuidof(IAudioEndpointVolume);
     IMMDeviceEnumerator *device_enum = NULL;"
    HAVE_MMDEVICEAPI_H
    )

  check_c_source_compiles(
    "#include <shlobj.h>
     IShellDispatch* pShellDispatch = NULL;"
    HAVE_ISHELLDISPATCH
    )

 check_struct_has_member("struct MOUSEHOOKSTRUCT" hwnd "windows.h" HAVE_STRUCT_MOUSEHOOKSTRUCT)  
 check_struct_has_member("struct MOUSEHOOKSTRUCTEX" mouseData "windows.h" HAVE_STRUCT_MOUSEHOOKSTRUCTEX)  
  
endif()

if (APPLE)
  set (PLATFORM_OS_OSX ON)

  if (HAS_LIBCXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
  
endif()

if (UNIX AND NOT APPLE)
  set (PLATFORM_OS_UNIX ON)

  # TODO: make option.
  set (HAVE_MONITORS "record,screensaver,x11events")

  find_package(X11)
  
  check_library_exists(Xtst XRecordEnableContext "" HAVE_XRECORD)

  check_library_exists(Xext XScreenSaverRegister "" SCREENSAVER_IN_XEXT)
  if(SCREENSAVER_IN_XEXT)
    set(XSS_LIB "Xext")			
  else()
    check_library_exists(Xss XScreenSaverRegister "" SCREENSAVER_IN_XSS)
    if(SCREENSAVER_IN_XSS)
      set(XSS_LIB "Xss")
    endif()
  endif()
  
  if (XSS_LIB)
    check_include_files(X11/extensions/scrnsaver.h HAVE_SCREENSAVER)
  endif()
endif()  

#----------------------------------------------------------------------------------------------------
# 

check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/select.h HAVE_SYS_SELECT_H)
check_include_files(stdlib.h stddef.h STDC_HEADERS)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)

check_function_exists(nanosleep HAVE_NANOSLEEP)
check_function_exists(realpath HAVE_REALPATH)
check_function_exists(setlocale HAVE_SETLOCALE)

set (PREFIX ${CMAKE_INSTALL_PREFIX}                       CACHE PATH "prefix")
set (BINDIR ${CMAKE_INSTALL_PREFIX}/bin                   CACHE PATH "bindir")
set (LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}      CACHE PATH "libdir")
set (DATADIR ${CMAKE_INSTALL_PREFIX}/share                CACHE PATH "datadir")
set (LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/local        CACHE PATH "localedir")
set (PKGDOCDIR ${CMAKE_INSTALL_PREFIX}/share/doc/workrave CACHE PATH "pkgdocdir")
set (PKGDATADIR ${CMAKE_INSTALL_PREFIX}/share/workrave    CACHE PATH "pkgdatadir")
set (DBUS_SERVICES_DIR ${DATADIR}/dbus-1/services         CACHE PATH "dbusservicedir")

#----------------------------------------------------------------------------------------------------
# Autoconf compatibility

# compiler definitions 
add_definitions(-DHAVE_CONFIG_H=1)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate config.h
configure_file(${CMAKE_SOURCE_DIR}/build/cmake/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#----------------------------------------------------------------------------------------------------
# Make dist.

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PACKAGE_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#----------------------------------------------------------------------------------------------------
# 

add_subdirectory(po)
add_subdirectory(libs)
add_subdirectory(backend)
add_subdirectory(frontend)

#----------------------------------------------------------------------------------------------------
# Install

if(PLATFORM_OS_WIN32)
  configure_file(README  README.txt  COPYONLY NEWLINE_STYLE WIN32)
  configure_file(NEWS    NEWS.txt    COPYONLY NEWLINE_STYLE WIN32)
  configure_file(AUTHORS AUTHORS.txt COPYONLY NEWLINE_STYLE WIN32)
  configure_file(COPYING COPYING.txt COPYONLY NEWLINE_STYLE WIN32)

  install(FILES README.txt NEWS.txt AUTHORS.txt COPYING.txt DESTINATION share/doc/workrave)
else()
  install(FILES README NEWS AUTHORS COPYING DESTINATION share/doc/workrave)
endif()

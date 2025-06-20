cmake_minimum_required(VERSION 3.22)
cmake_policy(VERSION 3.22)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0100)
  cmake_policy(SET CMP0100 NEW)
endif()
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(Workrave)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Require out of source build.
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build(
 "${PROJECT_NAME} requires an out of source build. Please create a separate
 build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there."
 )

# Set Workrave version
set (WORKRAVE_VERSION "1.11.0-rc.2" )
set (WORKRAVE_RESOURCE_VERSION "1,10,999,202" )
set (WORKRAVE_CHANNEL "candidate" )

set (CMAKE_BUILD_TYPE_INIT "Release")
set (CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckCSourceCompiles)
include(FindPythonModule)
include(ExternalProject)
include(FindPkgConfig)

#-----------------------------------------------------------------------------------------------------
# Version

if(NOT DEFINED ENV{WORKRAVE_GIT_VERSION})
  find_package(Git QUIET)
  if(Git_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --is-inside-work-tree
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE GIT_RESULT
      OUTPUT_QUIET
      ERROR_QUIET
    )

    if(GIT_RESULT EQUAL 0)
      execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=10
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION_OUTPUT
        RESULT_VARIABLE GIT_VERSION_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
      )

      if(GIT_VERSION_RESULT EQUAL 0 AND GIT_VERSION_OUTPUT)
        string(REGEX REPLACE "-g.*" "" WORKRAVE_GIT_VERSION "${GIT_VERSION_OUTPUT}")
        message(STATUS "Using git version: ${WORKRAVE_GIT_VERSION}")
      endif()
    endif()
  endif()
else()
  # Use environment variable
  set(WORKRAVE_VERSION "$ENV{WORKRAVE_GIT_VERSION}")
  message(STATUS "Using WORKRAVE_GIT_VERSION from environment: ${WORKRAVE_VERSION}")
endif()

#----------------------------------------------------------------------------------------------------
# Build DateTime

if(NOT DEFINED ENV{WORKRAVE_BUILD_DATETIME})
  # Get current datetime in ISO format
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    # Windows PowerShell command for ISO datetime
    execute_process(
      COMMAND powershell -Command "Get-Date -Format 'yyyy-MM-ddTHH:mm:sszzz'"
      OUTPUT_VARIABLE BUILD_DATETIME_OUTPUT
      RESULT_VARIABLE BUILD_DATETIME_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  else()
    # Unix/Linux date command for ISO datetime
    execute_process(
      COMMAND date --iso-8601=seconds
      OUTPUT_VARIABLE BUILD_DATETIME_OUTPUT
      RESULT_VARIABLE BUILD_DATETIME_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  endif()

  if(BUILD_DATETIME_RESULT EQUAL 0 AND BUILD_DATETIME_OUTPUT)
    set(WORKRAVE_BUILD_DATETIME "${BUILD_DATETIME_OUTPUT}")
    message(STATUS "Using generated build datetime: ${WORKRAVE_BUILD_DATETIME}")
  else()
    # Fallback to current date/time using CMake's string(TIMESTAMP)
    string(TIMESTAMP WORKRAVE_BUILD_DATETIME "%Y-%m-%dT%H:%M:%S%z")
    message(STATUS "Using CMake timestamp fallback: ${WORKRAVE_BUILD_DATETIME}")
  endif()
else()
  set(WORKRAVE_BUILD_DATETIME "$ENV{WORKRAVE_BUILD_DATETIME}")
  message(STATUS "Using WORKRAVE_BUILD_DATETIME from environment: ${WORKRAVE_BUILD_DATETIME}")
endif()

message(STATUS "WORKRAVE_VERSION: ${WORKRAVE_VERSION}")
message(STATUS "WORKRAVE_GIT_VERSION: ${WORKRAVE_GIT_VERSION}")
message(STATUS "WORKRAVE_BUILD_DATETIME: ${WORKRAVE_BUILD_DATETIME}")

#----------------------------------------------------------------------------------------------------
# Macros

macro(pkgconfig_get_var OUT PKG VARNAME)
  if (LOCALINSTALL)
    set(NEWPREFIX "--define-variable=prefix=${CMAKE_INSTALL_PREFIX}")
  endif()

  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${VARNAME} ${NEWPREFIX} ${PKG}
    OUTPUT_VARIABLE _result
    RESULT_VARIABLE _null
  )

  if (_null)
  else()
    string(REGEX REPLACE "[\r\n]" " " _result "${_result}")
    string(REGEX REPLACE " +$" ""  _result "${_result}")
    separate_arguments(_result)
    set (${OUT} ${_result} CACHE INTERNAL "")
  endif()
endmacro()

macro(pkgconfig_get_requires OUT PKG)
  if (LOCALINSTALL)
    set(NEWPREFIX "--define-variable=prefix=/")
  endif()

  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} --print-requires ${NEWPREFIX} ${PKG}
    OUTPUT_VARIABLE _result
    RESULT_VARIABLE _null
  )

  if (_null)
  else()
    string(REGEX REPLACE "[\r\n]" " " _result "${_result}")
    string(REGEX REPLACE " +$" ""  _result "${_result}")
    separate_arguments(_result)
    set (${OUT} ${_result} CACHE INTERNAL "")
  endif()
endmacro()

macro(files_exists OUT DIRECTORY FILES)
set (${OUT} TRUE)
  foreach(file ${FILES})
    if (NOT EXISTS "${DIRECTORY}/${file}")
      set (${OUT} FALSE)
    endif()
  endforeach()
endmacro()

#----------------------------------------------------------------------------------------------------
# Compiler

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    list(APPEND EXTRA_LIBRARIES stdc++fs)
  endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-fconcepts" COMPILER_SUPPORTS_FCONCEPTS)
if(COMPILER_SUPPORTS_FCONCEPTS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")
else()
  CHECK_CXX_COMPILER_FLAG("-fconcepts-ts" COMPILER_SUPPORTS_FCONCEPTS_TS)
  if(COMPILER_SUPPORTS_FCONCEPTS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-ts")
  endif()
endif()

set(CMAKE_CXX_COMPILER_VERSION_MIN "9.1.0")
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${CMAKE_CXX_COMPILER_VERSION_MIN})
  # gcc-8.5.0 supports C++20 but doesn't support contains() in std:map.
  # For more details visit https://gcc.gnu.org/onlinedocs/gcc-8.5.0/libstdc++/api/a00560_source.html
  message(FATAL_ERROR "Please use gcc version ${CMAKE_CXX_COMPILER_VERSION_MIN} or above.")
endif()

#----------------------------------------------------------------------------------------------------
# Platform

set (PLATFORM_ARCH 32)
if (VCPKG_TOOLCHAIN)
  if(CMAKE_CXX_COMPILER MATCHES "x64/cl.exe")
    set (PLATFORM_ARCH 64)
  endif()
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (PLATFORM_ARCH 64)
endif()

if (WIN32 AND (NOT MSVC) AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-target-triple OUTPUT_VARIABLE CLANG_TRIPLE)
  string(REGEX MATCH "x86_64-.*" _clang_is_x86 ${CLANG_TRIPLE})
  if (CLANG_TRIPLE MATCHES "x86_64")
    set(CMAKE_SYSTEM_PROCESSOR "amd64")
    message(STATUS "Clang detected as x86_64")
  else()
    set(CMAKE_SYSTEM_PROCESSOR "arm64")
    message(STATUS "Clang detected as arm64")
  endif()
endif()

if (WIN32 AND MSVC)
  if(VCPKG_TARGET_TRIPLET MATCHES "^x64-windows")
      set(CMAKE_SYSTEM_PROCESSOR AMD64)
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Testing

option(WITH_TESTS "Enable unit tests" OFF)

if (WITH_TESTS)
  set (HAVE_TESTS ON)
  enable_testing()
endif()

include(code-coverage)

#----------------------------------------------------------------------------------------------------
# Toolkit selection

set (WITH_UI "Gtk+3" CACHE STRING "UI Toolkit to use")
set (WITH_UI_VALUES "Qt" "Gtk+3")
set_property(CACHE WITH_UI PROPERTY STRINGS ${WITH_UI_VALUES})
list(FIND WITH_UI_VALUES ${WITH_UI} WITH_UI_INDEX)
if (${WITH_UI_INDEX} EQUAL -1)
  message(FATAL_ERROR "Option ${WITH_UI} not supported for WITH_UI, valid entries are ${WITH_UI_VALUES}")
endif()

set (WITH_UI_QT FALSE)
if ("${WITH_UI}" STREQUAL "Qt")
  set (WITH_UI_QT TRUE)
endif()

set (WITH_UI_GTK FALSE)
if ("${WITH_UI}" STREQUAL "Gtk+3")
  set (WITH_UI_GTK TRUE)
endif()

#----------------------------------------------------------------------------------------------------
# Locations

if (UNIX AND NOT APPLE)
  set (BINDIR bin)
  set (DATADIR share                                        CACHE PATH "datadir")
  set (LIBDIR "${CMAKE_INSTALL_LIBDIR}"                     CACHE PATH "libdir")
  set (PKGDATADIR share/workrave                            CACHE PATH "pkgdatadir")
  set (SOUNDSDIR "${DATADIR}/sounds/workrave")
  set (EXERCISESDIR "${PKGDATADIR}/exercises")
  set (IMAGESDIR "${PKGDATADIR}/images")
  set (LOCALEDIR "${DATADIR}/locale")
  set (SYSCONFDIR "${CMAKE_INSTALL_SYSCONFDIR}"             CACHE PATH "system conf. dir (etc)")
  set (XDGCONFDIR "${SYSCONFDIR}/xdg")
  set (XDGAUTOSTARTDIR "${XDGCONFDIR}/autostart")
elseif (APPLE)
  set (BUNDLEDIR Workrave.app/Contents/)
  set (BINDIR ${BUNDLEDIR}/MacOS/)
  set (RESOURCESDIR ${BUNDLEDIR}/Resources)
  set (DATADIR ${RESOURCESDIR}/share                        CACHE PATH "datadir")
  set (LIBDIR ${RESOURCESDIR}/lib                           CACHE PATH "libdir")
  set (PKGDATADIR ${DATADIR}                                CACHE PATH "pkgdatadir")
  set (SOUNDSDIR "${DATADIR}/sounds")
  set (EXERCISESDIR "${DATADIR}/exercises")
  set (IMAGESDIR "${DATADIR}/images")
  set (LOCALEDIR "${DATADIR}/locale")
elseif (WIN32)
  set (BINDIR bin)
  set (BINDIR32 bin32)
  set (DATADIR share                                        CACHE PATH "datadir")
  set (LIBDIR lib                                           CACHE PATH "libdir")
  set (PKGDATADIR share/workrave                            CACHE PATH "pkgdatadir")
  set (SOUNDSDIR "${DATADIR}/sounds")
  set (EXERCISESDIR "${DATADIR}/exercises")
  set (IMAGESDIR "${DATADIR}/images")
  set (LOCALEDIR "${DATADIR}/locale")
endif()

add_compile_definitions(
  WORKRAVE_PKGDATADIR="${CMAKE_INSTALL_PREFIX}/${PKGDATADIR}"
  WORKRAVE_DATADIR="${CMAKE_INSTALL_PREFIX}/${DATADIR}"
  WORKRAVE_BINDIR32="${BINDIR32}"
)

#----------------------------------------------------------------------------------------------------
# Core selection

option (WITH_CORE_NEXT "Enable Core Next" OFF)

if (WITH_CORE_NEXT)
  set(HAVE_CORE_NEXT ON)
endif()

if (WITH_UI_QT)
  message("Using CoreNext With with Qt")
  set (HAVE_CORE_NEXT ON)
endif()

#----------------------------------------------------------------------------------------------------
# SBOM

include(SBOM)

if (WIN32)
  option(WITH_SBOM "Enable generation of BSOM when using MSYS2/MinGW" OFF)
  file(WRITE "${EXTERNAL_SBOM_FILE}" "")
  if (WITH_SBOM)
    set (HAVE_SBOM ON)
    file(APPEND "${EXTERNAL_SBOM_FILE}" "semver,v0.3.1,MIT,Semantic Versioning for modern C++,https://github.com/Neargye/semver\n") # from unfold
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Pkg-config

if ("${WITH_UI_QT}" AND (NOT VCPKG_TOOLCHAIN))
  find_package(PkgConfig REQUIRED)
endif()

#----------------------------------------------------------------------------------------------------
# Python

set(Python3_FIND_REGISTRY "LAST")
find_package(Python3 COMPONENTS Interpreter)

#----------------------------------------------------------------------------------------------------
# DBus

option(WITH_DBUS "Enable dbus" ON)

if (WITH_DBUS)
  set(HAVE_JINJA2 OFF)
  message(STATUS "Checking for: Jinja2")
  if (Python3_FOUND)
    find_python_module(jinja2)
  endif()
  if (PY_JINJA2)
     set(HAVE_JINJA2 ON)
  endif()
  include(DBus)
endif()

#----------------------------------------------------------------------------------------------------
# GTK

if ("${WITH_UI}" STREQUAL "Gtk+3")
  message(STATUS "Checking for: Gtk 3")

  pkg_check_modules(
    GTK REQUIRED
    gtkmm-3.0>=3.22.0
    gtk+-3.0>=3.22.0)

  pkg_check_modules(
    GLIB REQUIRED
    glib-2.0>=2.56.0
    gio-2.0>=2.56.0)

  if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_REQUIRED_INCLUDES ${GTK_INCLUDE_DIRS})
    set(CMAKE_REQUIRED_LIBRARIES ${GTK_LIBRARIES})
    foreach(GLIBCXX_ABI_DEFINITION "" "-D_GLIBCXX_USE_CXX11_ABI")
      set(CMAKE_REQUIRED_DEFINITIONS ${GLIBCXX_ABI_DEFINITION})

      check_cxx_source_compiles(
        "#include <glibmm/ustring.h>
         #include <string>

         int
         main(int argc, char **argv)
         {
           Glib::ustring(std::string(\"c++\"));
           return 0;
         }"
        DUAL_ABI_RESOLVED
      )
      if (DEFINED DUAL_ABI_RESOLVED AND "${DUAL_ABI_RESOLVED}" STREQUAL "1")
        if (NOT ${GLIBCXX_ABI_DEFINITION} STREQUAL "")
          add_definitions(${GLIBCXX_ABI_DEFINITION})
        endif()
        break()
      endif()
    endforeach()

    if (NOT DEFINED DUAL_ABI_RESOLVED OR "${DUAL_ABI_RESOLVED}" STREQUAL "")
      message(FATAL_ERROR "Linked GTK libraries don't support C++11 and your compiler doesn't support dual ABI.") #https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html
    endif()
  endif()

  set (HAVE_GSETTINGS ON)
  set (HAVE_GLIB ON)
  set (HAVE_GTK ON)
  set (HAVE_APP_GTK ON)

  if (WITH_DBUS)
    set (HAVE_DBUS ON)
    set (HAVE_DBUS_GIO ON)
    set (DBUS_BACKEND "gio")
  endif()

  find_package(GSettings)
endif()


#----------------------------------------------------------------------------------------------------
# Qt

if ("${WITH_UI}" STREQUAL "Qt")
  message(STATUS "Checking for: Qt")

  find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui Svg LinguistTools Xml REQUIRED)
  qt6_standard_project_setup()
  set(CMAKE_AUTORCC ON)

  get_target_property(QT_LRELEASE_EXECUTABLE Qt::lrelease LOCATION)

  if (TARGET Qt::lconvert)
    get_target_property(QT_LCONVERT_EXECUTABLE Qt::lconvert LOCATION)
  else()
    get_filename_component(QT_LRELEASE_PATH ${QT_LRELEASE_EXECUTABLE} DIRECTORY)
    find_program(QT_LCONVERT_EXECUTABLE
                 NAMES lconvert-qt6 lconvert
                 PATHS ${QT_LRELEASE_PATH}
                 NO_DEFAULT_PATH)
  endif()

  get_target_property(QT_QTPATHS_EXECUTABLE Qt6::qtpaths LOCATION)

  execute_process(COMMAND ${QT_QTPATHS_EXECUTABLE} --plugin-dir OUTPUT_VARIABLE QT_PLUGINS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "QT Plugin path: " ${QT_PLUGINS_DIR})

  if (WITH_DBUS)
    find_package(Qt6DBus QUIET)
  endif()

  if (Qt6DBus_FOUND)
      set (HAVE_DBUS ON)
      set (HAVE_DBUS_QT ON)
      set (DBUS_BACKEND "qt")
  endif()

  get_target_property(QtCore_location_Release Qt${QT_VERSION_MAJOR}::Core LOCATION_Release)
  get_filename_component(QT_BIN_DIR "${QtCore_location_Release}" DIRECTORY)
  if (APPLE)
    get_filename_component(QT_INSTALL_PREFIX "${QT_BIN_DIR}/../../../" REALPATH)
  endif()

  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")

  set (HAVE_QT ON)
  set (HAVE_APP_QT ON)

  if (Qt6_POSITION_INDEPENDENT_CODE)
    set (CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()

  if ((NOT APPLE) AND (NOT WIN32))
    # glib is required for ui/common/src/System.cc
    pkg_check_modules(
      GLIB REQUIRED
      glib-2.0>=2.28.0
      gio-2.0>=2.26.0)
    set (HAVE_GLIB ON)
  endif()
  if (WIN32 AND ${PLATFORM_ARCH} EQUAL 32)
    message(FATAL_ERROR "32 bit compilation of Qt UI not supported on Windows")
  endif()
endif()



#----------------------------------------------------------------------------------------------------
# GStreamer

if (UNIX AND NOT APPLE)
  option(WITH_GSTREAMER "Enable GStreamer audio" ON)

  if (WITH_GSTREAMER)
    message(STATUS "Checking for: GStreamer")

    pkg_check_modules(GSTREAMER QUIET gstreamer-1.0)
    if (GSTREAMER_FOUND)
      set (HAVE_GSTREAMER ON)
    else()
      pkg_check_modules(GSTREAMER QUIET gstreamer-0.10)
      if (GSTREAMER_FOUND)
        set (HAVE_GSTREAMER ON)
      else()
        set (WITH_GSTREAMER OFF)
      endif()
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Pulseaudio

if (UNIX AND NOT APPLE)
  option(WITH_PULSE "Enable Pulseaudio Mixer support" ON)

  if (WITH_PULSE)
    message(STATUS "Checking for: Pulseaudio")

    pkg_check_modules(PULSE glib-2.0 libpulse libpulse-mainloop-glib)

    if (PULSE_FOUND)
      set (HAVE_PULSE ON)
    else()
      set (WITH_PULSE OFF)
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Gnome

include(GDbusCodegen)

if (UNIX AND NOT APPLE)
  set (WITH_GNOME_CLASSIC_PANEL_DEFAULT ON)
else()
  set (WITH_GNOME_CLASSIC_PANEL_DEFAULT OFF)
endif()

option(WITH_GNOME_CLASSIC_PANEL "Enable Gnome 3 Classic panel applet support" ${WITH_GNOME_CLASSIC_PANEL_DEFAULT})
option(WITH_GNOME45 "Use GNOME 45 API" OFF)

if (WITH_GNOME45)
  set (HAVE_GNOME45 ON)
endif()

if (WITH_GNOME_CLASSIC_PANEL AND HAVE_GTK)
  message(STATUS "Checking for: Gnome 3 classic panel applets")
  set (PANELAPPLET_FOUND NO)

  pkg_check_modules(PANELAPPLET QUIET libgnome-panel>=3.36.0 gio-2.0>=2.26.0)

  if (PANELAPPLET_FOUND)
    set (HAVE_GNOME3_PANEL_APPLET ON)
    set (HAVE_LIBGNOME_PANEL ON)
  else()
    pkg_check_modules(PANELAPPLET QUIET libpanel-applet>=3.16.1 gio-2.0>=2.26.0)

    if (PANELAPPLET_FOUND)
      set (HAVE_GNOME3_PANEL_APPLET ON)
      set (HAVE_LIBPANEL_APPLET ON)
    else()
        set (WITH_GNOME_CLASSIC_PANEL OFF)
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# DBus Menu

if (UNIX AND NOT APPLE)
  option(WITH_DBUSMENU "Enable DBUS Menu support" ON)

  if (WITH_DBUSMENU)
    message(STATUS "Checking for: DBUS Menu")

    pkg_check_modules(DBUSMENU dbusmenu-glib-0.4 dbusmenu-gtk3-0.4)
    if (DBUSMENU_FOUND)
      set (HAVE_DBUSMENU ON)
    else()
      set (WITH_DBUSMENU OFF)
    endif()
  endif()
endif()


#----------------------------------------------------------------------------------------------------
# Indicator

if (UNIX AND NOT APPLE)
  option(WITH_INDICATOR "Enable Ayatana indicator support" ON)
  option(LOCALINSTALL "Install file locally instead of to system location" OFF)

  pkg_check_modules(
    GTK3
    gtk+-3.0>=3.10.0)

  if (WITH_INDICATOR AND HAVE_DBUSMENU)
    message(STATUS "Checking for: Ayatana Indicators")

    find_package(GObjectIntrospection 0.9.12)
    set (HAVE_INTROSPECTION ${INTROSPECTION_FOUND})

    if (HAVE_INTROSPECTION)
      pkg_check_modules(INDICATOR ayatana-indicator3-0.4)
    endif()

    if (INDICATOR_FOUND)
      set (HAVE_INDICATOR ON)
      set (HAVE_INDICATOR_AYATANA ON)
    else()
      pkg_check_modules(INDICATOR indicator3-0.4)
      if (INDICATOR_FOUND)
        set (HAVE_INDICATOR ON)
        set (HAVE_INDICATOR_AYATANA OFF)
      else()
        set (WITH_INDICATOR OFF)
        set (HAVE_INDICATOR_AYATANA OFF)
      endif()
    endif()
  endif()

  if (HAVE_INDICATOR)
    if (LOCALINSTALL)
      set (INDICATORDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/indicators3/7")
      set (INDICATORICONSDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/libindicate/icons")
    else()
      if (HAVE_INDICATOR_AYATANA)
        pkgconfig_get_var(INDICATORDIR ayatana-indicator3-0.4 indicatordir)
        pkgconfig_get_var(INDICATORICONSDIR ayatana-indicator3-0.4 iconsdir)
      else()
        pkgconfig_get_var(INDICATORDIR indicator3-0.4 indicatordir)
        pkgconfig_get_var(INDICATORICONSDIR indicator3-0.4 iconsdir)
      endif()
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# AppIndicator

if (UNIX AND NOT APPLE)
  option(WITH_APPINDICATOR "Enable Appindicator support" ON)

  if (WITH_APPINDICATOR AND HAVE_DBUSMENU)
    message(STATUS "Checking for: AppIndicator")
    pkg_check_modules(APPINDICATOR ayatana-appindicator3-0.1)

    if (APPINDICATOR_FOUND )
      set (HAVE_APPINDICATOR ON)
      set (HAVE_APPINDICATOR_AYATANA ON)
      set (HAVE_STATUSICON OFF)
    else()
      pkg_check_modules(APPINDICATOR appindicator3-0.1)
      if (APPINDICATOR_FOUND)
        set (HAVE_APPINDICATOR ON)
        set (HAVE_APPINDICATOR_AYATANA OFF)
        set (HAVE_STATUSICON OFF)
        else()
        set (WITH_APPINDICATOR OFF)
        set (HAVE_APPINDICATOR OFF)
        set (HAVE_APPINDICATOR_AYATANA OFF)
        set (HAVE_STATUSICON ON)
        endif()
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# MATE

if (UNIX AND NOT APPLE)
  option(WITH_MATE "Enable Mate applet support" ON)

  if (WITH_MATE)
    message(STATUS "Checking for: Mate")
    pkg_check_modules(MATE libmatepanelapplet-4.0>=1.20.0)

    if (MATE_FOUND)
      set (HAVE_MATE ON)
    else()
      set (WITH_MATE OFF)
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# XFCE

if (UNIX AND NOT APPLE)
  option(WITH_XFCE4 "Enable Xfce applet support" ON)

  if (WITH_XFCE4)
    message(STATUS "Checking for: Xfce 4")
    pkg_check_modules(XFCE4 libxfce4panel-2.0>=4.12)

    if (XFCE4_FOUND)
      set (HAVE_XFCE4 ON)
    else()
      set (WITH_XFCE4 OFF)
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Lang info

check_cxx_source_compiles("
    #include <langinfo.h>
    int main(int argc, char **argv)
    {
        const char *r = nl_langinfo(_NL_TIME_FIRST_WEEKDAY);
        const char *s = nl_langinfo(_NL_TIME_WEEK_1STDAY);

        return 0;
    }
" HAVE_NL_TIME_FIRST_WEEKDAY_AND_WEEK_1STDAY)

#----------------------------------------------------------------------------------------------------
# Boost

if (MSVC)
  if (NOT DEFINED BOOST_ALL_DYN_LINK)
    set (BOOST_ALL_DYN_LINK TRUE)
  endif()

  set (BOOST_ALL_DYN_LINK "${BOOST_ALL_DYN_LINK}" CACHE BOOL "boost enable dynamic linking")
  if (BOOST_ALL_DYN_LINK)
    add_compile_definitions(BOOST_ALL_DYN_LINK)
  endif()
endif()

set (BOOST_COMPONENTS date_time program_options serialization)
#set (Boost_DEBUG ON)

if (WIN32)
  if (${PLATFORM_ARCH} EQUAL 64)
    set (Boost_ARCHITECTURE "-x64")
  elseif(${PLATFORM_ARCH} EQUAL 32)
    set (Boost_ARCHITECTURE "-x32")
  endif()
endif()

if (HAVE_TESTS)
  set(BOOST_COMPONENTS ${BOOST_COMPONENTS} unit_test_framework )
endif()

set (BOOST_MIN_VERSION "1.54.0")
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
  set (BOOST_MIN_VERSION "1.73.0") #https://github.com/boostorg/signals2/commit/15fcf213563718d2378b6b83a1614680a4fa8cec
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_SHARED_LIBS ON)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS})

if (HAVE_TESTS)
  if (WIN32)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_SHARED_LIBS OFF)
  endif()
  find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS test_exec_monitor)
endif()


#----------------------------------------------------------------------------------------------------
# Wayland

option(WITH_WAYLAND "Enable Wayland support" ON)

if (UNIX AND NOT APPLE)
  if (WITH_WAYLAND)
    message(STATUS "Checking for: Wayland")

    pkg_check_modules(WAYLAND_CLIENT wayland-client)

    find_program(WAYLAND_SCANNER wayland-scanner)
    if (NOT WAYLAND_SCANNER)
      message(ERROR "wayland-scanner not found")
    endif()

    if (WAYLAND_CLIENT_FOUND AND WAYLAND_SCANNER)
      set (HAVE_WAYLAND ON)
    else()
      set (WITH_WAYLAND OFF)
    endif()
  endif()
endif()


#----------------------------------------------------------------------------------------------------
# Translations

set (ALL_LINGUAS "")
file(STRINGS "${CMAKE_SOURCE_DIR}/po/LINGUAS" pofile)
foreach(line ${pofile})
  if ("${line}" MATCHES "^[a-zA-Z]")
    set (ALL_LINGUAS "${ALL_LINGUAS}${line} ")
  endif()
endforeach()
string(STRIP ${ALL_LINGUAS} ALL_LINGUAS)

find_package(Gettext)
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

if (HAVE_QT)
  set (HAVE_LIBINTL OFF)
else()
  find_package(Libintl)

  # TODO: Gettext cannot be found in 32bit
  if (NOT (GETTEXT_FOUND AND LIBINTL_FOUND))
    message(FATAL_ERROR "Libintl not found")
  endif()
  set (HAVE_LIBINTL ON)
endif()

set (GETTEXT_PACKAGE "workrave")

# TODO: generate languages.iss for InnoSetup

#----------------------------------------------------------------------------------------------------
# Tracing

option(WITH_TRACING "Enable debug tracing" OFF)

if (WITH_TRACING)
  set (HAVE_TRACING ON)
  #SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTRACING")
endif()


#----------------------------------------------------------------------------------------------------
# platform - windows


if (WIN32)
  option(WITH_CRASHPAD "Use crashpad" OFF)
  option(WITH_AUTO_UPDATE "Use unfold auto-update" OFF)
  option(ISCC "Inno Setup Compiler")
  set(ISCC_FLAGS "" CACHE STRING "Inno Setup Compiler Arguments")

  cmake_file_api(
    QUERY
    API_VERSION 1
    CODEMODEL 2.3
    TOOLCHAINS 1
  )

  if (WITH_AUTO_UPDATE)
    set (HAVE_UNFOLD_AUTO_UPDATE ON)
  endif()

  set (PLATFORM_OS_WINDOWS ON)
  set (PLATFORM_OS_NAME "Windows")
  if (MSVC)
    set (PLATFORM_OS_WINDOWS_NATIVE 1)
  endif()

  set (HAVE_HARPOON ON)

  if (NOT MSVC)
    find_program(MSYS2_SHELL
                NAMES msys2_shell.cmd
                PATHS
                "$ENV{RUNNER_TEMP}/msys64")
    if (MSYS2_SHELL)
      set(MSYS_CMD ${MSYS2_SHELL})
    else()
      include(FindMsys)
    endif()
    set(MSYS64_CMD ${MSYS_CMD} -here -mingw64 -no-start -defterm -c)
    set(MSYS32_CMD ${MSYS_CMD} -here -mingw32 -no-start -defterm -c)
  endif()

  find_library(SSP_LIBRARY libssp)

  if (NOT ISCC)
    if (NOT WINE)
      get_filename_component(PROGRAM_FILES_PATH
        "[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion;ProgramFilesDir]" ABSOLUTE)

      get_filename_component(PROGRAM_FILES_X86_PATH
        "[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion;ProgramFilesDir (x86)]" ABSOLUTE)

      file(GLOB ISCC_PATHS "${PROGRAM_FILES_PATH}/Inno Setup 6" "${PROGRAM_FILES_X86_PATH}/Inno Setup 6")
      find_program(INNOCC NAMES iscc.exe ISCC.exe PATHS ${ISCC_PATHS})
      set(ISCC "${INNOCC}")
    else()
      set(ISCC "C:/Program Files/Inno Setup 6/ISCC.exe")
    endif()
  endif()

  set (HAVE_GSETTINGS OFF)
  set (HAVE_LANGUAGE_SELECTION TRUE)
  set (HAVE_STATUSICON ON)

  if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _SCL_SECURE_NO_WARNINGS)

    set (MSVC_WARNINGS "/W2 /wd4127 /wd4250 /wd4413 /wd4800 /wd5045")
    set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${MSVC_WARNINGS}")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MSVC_WARNINGS}")

    add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/bigobj$<SEMICOLON>/Zc:__cplusplus>)

    add_compile_definitions(
       NOMINMAX
       WIN32
       WINVER=0xA00
       _WIN32_WINNT=0x0A00
       _WINDOWS)

    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")

  endif()

  if (VCPKG_TOOLCHAIN)
    set(SYS_ROOT ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})
  endif()

  check_c_source_compiles(
    "#include <mmdeviceapi.h>
     #include <endpointvolume.h>

     IAudioEndpointVolume *audio_endpoint_volume = NULL;
     IMMDeviceEnumerator *device_enum = NULL;
     int main(){return 0;}"
     HAVE_MMDEVICEAPI_H
    )

  check_c_source_compiles(
    "#include <windows.h>
     #include <shlobj.h>

     IShellDispatch* pShellDispatch = NULL;
     int main(){return 0;}"
     HAVE_ISHELLDISPATCH
    )

  check_struct_has_member("MOUSEHOOKSTRUCT" hwnd "windows.h" HAVE_STRUCT_MOUSEHOOKSTRUCT)
  check_struct_has_member("MOUSEHOOKSTRUCTEX" mouseData "windows.h" HAVE_STRUCT_MOUSEHOOKSTRUCTEX)

  set(LIBS winmm wtsapi32 shlwapi dsound dxguid)
  foreach(lib ${LIBS})
   find_library(${lib}_PATH ${lib} RERQUIRED)
    if(${lib}_PATH)
      set(WIN32_LIBRARIES ${WIN32_LIBRARIES} ${${lib}_PATH})
    else()
      message(STATUS "Could not find ${lib}")
    endif()
  endforeach()

endif()

#----------------------------------------------------------------------------------------------------
# platform - macOS

if (APPLE)
  set (PLATFORM_OS_MACOS ON)
  set (PLATFORM_OS_NAME "MacOS")

  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  find_library(IOKIT_LIBRARY IOKit REQUIRED)
  include_directories(/usr/local/include)
  include_directories(${CMAKE_SYSTEM_INCLUDE_PATH})

  set(WORKRAVE_SIGN_IDENTITY "" CACHE STRING "")
endif()

#----------------------------------------------------------------------------------------------------
# platform - Unix

if (UNIX AND NOT APPLE)
  set (PLATFORM_OS_UNIX ON)
  set (PLATFORM_OS_NAME "Unix")

  set (HAVE_MONITORS "mutter,wayland,screensaver,record,x11events")

  find_package(X11)

  check_library_exists(Xtst XRecordEnableContext "" HAVE_XRECORD)

  check_library_exists(Xext XScreenSaverRegister "" SCREENSAVER_IN_XEXT)
  if (SCREENSAVER_IN_XEXT)
    set (XSS_LIB "Xext")
  else()
    check_library_exists(Xss XScreenSaverRegister "" SCREENSAVER_IN_XSS)
    if (SCREENSAVER_IN_XSS)
      set (XSS_LIB "Xss")
    endif()
  endif()

  if (XSS_LIB)
    check_include_files(X11/extensions/scrnsaver.h HAVE_SCREENSAVER)
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# Logging

include(FetchContent)

find_package(fmt 6.1.2 CONFIG QUIET)
if(${fmt_FOUND})
  find_package(spdlog 1.5.0 CONFIG QUIET)
else()
  FetchContent_Declare_Logged(
      fmt
      "an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams"
      "MIT"
      GIT_REPOSITORY https://github.com/fmtlib/fmt.git
      GIT_SHALLOW TRUE
      GIT_TAG 10.0.0
    )
  FetchContent_MakeAvailable(fmt)
  target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)
endif()

if((NOT ${spdlog_FOUND}) OR (NOT ${fmt_FOUND}))
  FetchContent_Declare_Logged(
      spdlog
      "Fast C++ logging library"
      "MIT"
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_SHALLOW TRUE
      GIT_TAG v1.12.0
    )
  if (WIN32)
    set(SPDLOG_INSTALL ON)
    set(SPDLOG_BUILD_SHARED ON)
  endif()
  set(SPDLOG_FMT_EXTERNAL ON)
  FetchContent_MakeAvailable(spdlog)
endif()

#----------------------------------------------------------------------------------------------------
# Crash reporting - Windows

set(HAVE_CRASH_REPORT OFF)
set(HAVE_CRASHPAD OFF)

if (WITH_CRASHPAD)
  if (MINGW)
    if (NOT "$ENV{MSYSTEM}" STREQUAL "CLANG64")
      message(FATAL_ERROR "Crashpad requires MINGW Clang64")
    endif()
  endif()

  FetchContent_Declare_Logged(
    crashpad
    "A crash-reporting system"
    "Apache-2.0"
    GIT_REPOSITORY https://github.com/rcaelers/crashpad.git
    # GIT_SHALLOW TRUE
    GIT_TAG d52c137a3d8b4adc20b33685460bf8cebd3c4634
  )
  FetchContent_MakeAvailable(crashpad)

  find_program(CARGO "cargo")
  if (NOT CARGO)
    message(FATAL_ERROR "Could not find cargo. Please install rust and cargo.")
  endif()

  # TODO: add for upload of symbols
  # FetchContent_Declare(
  #   breakpad
  #   GIT_REPOSITORY
  #   GIT_SHALLOW TRUE
  #   GIT_TAG c161459d7e94d7ef5840ba90aa26a4d5f955c976
  # )
  # FetchContent_MakeAvailable(breakpad)

  FetchContent_Declare(
    dump_syms
    GIT_REPOSITORY https://github.com/mozilla/dump_syms.git
    GIT_TAG v2.3.4
  )
  FetchContent_MakeAvailable(dump_syms)

  set(DUMP_SYMS ${dump_syms_SOURCE_DIR}/target/release/dump_syms.exe)
  add_custom_target(dump_syms ALL DEPENDS ${DUMP_SYMS})
  add_custom_command(
    OUTPUT ${DUMP_SYMS}
    COMMAND ${MSYS_CMD} -here -mingw64 -no-start -defterm -c "cargo build --release"
    WORKING_DIRECTORY ${dump_syms_SOURCE_DIR}
    )

  set(HAVE_CRASHPAD ON)
  set(HAVE_CRASH_REPORT ON)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-pdb=")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-pdb=")
    add_compile_options(-gdwarf -gcodeview)
  elseif(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message(STATUS "Crashpad requires Clang or MSVC")
  endif()
endif()

#----------------------------------------------------------------------------------------------------
#  misc checks

check_function_exists(setlocale HAVE_SETLOCALE)

#----------------------------------------------------------------------------------------------------
# Autoconf compatibility

# compiler definitions
add_compile_definitions(HAVE_CONFIG_H=1)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Generate config.h
configure_file(${CMAKE_SOURCE_DIR}/cmake/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#----------------------------------------------------------------------------------------------------
# Make dist.

set (ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${WORKRAVE_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

#----------------------------------------------------------------------------------------------------
#

add_subdirectory(po)
add_subdirectory(libs)
add_subdirectory(ui)

#----------------------------------------------------------------------------------------------------
# Install

if (WIN32)
  configure_file(README  README.txt  NEWLINE_STYLE WIN32)
  configure_file(NEWS    NEWS.txt    NEWLINE_STYLE WIN32)
  configure_file(AUTHORS AUTHORS.txt NEWLINE_STYLE WIN32)
  configure_file(COPYING COPYING.txt NEWLINE_STYLE WIN32)

  install(FILES ${CMAKE_BINARY_DIR}/README.txt
                ${CMAKE_BINARY_DIR}/NEWS.txt
                ${CMAKE_BINARY_DIR}/AUTHORS.txt
                ${CMAKE_BINARY_DIR}/COPYING.txt
          DESTINATION .)
endif()

#----------------------------------------------------------------------------------------------------
# App

if (WITH_UI_QT)
  add_subdirectory(ui/app/toolkits/qt/dist)
elseif (WITH_UI_GTK)
  add_subdirectory(ui/app/toolkits/gtkmm/dist)
endif()

#----------------------------------------------------------------------------------------------------
# Summary

function(feature_name name out)
	set(msg "${name}")
	string(LENGTH "${msg}" len)
	while("32" GREATER "${len}")
		set(msg " ${msg}")
		math(EXPR len "${len} + 1")
	endwhile()

	set(${out} "${msg}" PARENT_SCOPE)
endfunction()

function(feature_text name setting)
  set(str "")
  feature_name(${name} str)
  message(STATUS "${str} : ${${setting}}")
endfunction()

function(feature_string name setting)
  set(str "")
  feature_name(${name} str)
  message(STATUS "${str} : ${setting}")
endfunction()

function(feature_bool name setting)
  set(str "")
  feature_name(${name} str)
  if (${${setting}})
    message(STATUS "${str} : Yes")
  else()
    message(STATUS "${str} : No")
  endif()
endfunction()

message(STATUS "")
message(STATUS "Workrave ${WORKRAVE_VERSION}")
message(STATUS "")

feature_text("Workrave" WORKRAVE_VERSION)
feature_text("Install prefix" CMAKE_INSTALL_PREFIX)
feature_text("OS" PLATFORM_OS_NAME)
feature_text("UI" WITH_UI)
message(STATUS "")
feature_bool("GSettings configuration" HAVE_GSETTINGS)
if (PLATFORM_OS_UNIX)
feature_text("ActivityMonitors" HAVE_MONITORS)
feature_bool("Gnome 3 Applet"  HAVE_GNOME3_PANEL_APPLET)
if (HAVE_GNOME45)
  feature_string("Gnome Shell API" ">=45")
else()
  feature_string("Gnome Shell API" "<45")
endif()
feature_bool("Indicator Applet" HAVE_INDICATOR)
feature_bool("XFCE Applet" HAVE_XFCE4)
feature_bool("MATE Applet" HAVE_MATE)
feature_bool("AppIndicator" HAVE_APPINDICATOR)
feature_bool("Legacy StatusIcon" HAVE_STATUSICON  )
feature_bool("DBus Menus" HAVE_DBUSMENU)
feature_bool("GStreamer" HAVE_GSTREAMER)
feature_bool("Pulseaudio" HAVE_PULSE)
feature_bool("Wayland" HAVE_WAYLAND)
endif()
feature_bool("DBUS" HAVE_DBUS)
if(PLATFORM_OS_WINDOWS)
feature_bool("Crash reporting" HAVE_CRASHPAD)
feature_bool("Auto update" HAVE_UNFOLD_AUTO_UPDATE)
endif()
feature_bool("Debug logs" HAVE_TRACING)
feature_bool("Tests" HAVE_TESTS)

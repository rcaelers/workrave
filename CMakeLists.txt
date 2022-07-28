if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(Workrave)

cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Require out of source build.
include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build(
 "${PROJECT_NAME} requires an out of source build. Please create a separate
 build directory and run 'cmake /path/to/${PROJECT_NAME} [options]' there."
 )

# Set Workrave version
set (WORKRAVE_VERSION "1.11.0-alpha.3" )
set (WORKRAVE_RESOURCE_VERSION "1,10,999,3" )

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

if (NOT VCPKG_TOOLCHAIN)
  include(FindPkgConfig)
endif()

#----------------------------------------------------------------------------------------------------
# Macros

if (NOT VCPKG_TOOLCHAIN)
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
endif()

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

#----------------------------------------------------------------------------------------------------
# Platform

set (PLATFORM_ARCH 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (PLATFORM_ARCH 64)
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
elseif (APPLE)
  set (BUNDLEDIR Workrave.app/Contents/)
  set (BINDIR ${BUNDLEDIR}/MacOS/)
  set (RESOURCESDIR ${BUNDLEDIR}/Resources)
  set (DATADIR ${RESOURCESDIR}/share                        CACHE PATH "datadir")
  set (LIBDIR ${RESOURCESDIR}/lib                           CACHE PATH "libadir")
  set (PKGDATADIR ${DATADIR}                                CACHE PATH "pkgdatadir")
  set (SOUNDSDIR "${DATADIR}/sounds")
  set (EXERCISESDIR "${DATADIR}/exercises")
  set (IMAGESDIR "${DATADIR}/images")
  set (LOCALEDIR "${DATADIR}/locale")
elseif (WIN32)
  set (BINDIR lib)
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
)

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
set (WITH_UI_VALUES "Qt" "Gtk+3" "None")
set_property(CACHE WITH_UI PROPERTY STRINGS ${WITH_UI_VALUES})
list(FIND WITH_UI_VALUES ${WITH_UI} WITH_UI_INDEX)
if (${WITH_UI_INDEX} EQUAL -1)
  message(FATAL_ERROR "Option ${WITH_UI} not supported for WITH_UI, valid entries are ${WITH_UI_VALUES}")
endif()

if ("${WITH_UI}" STREQUAL "None"
    AND (${PLATFORM_ARCH} EQUAL 32)
    AND WIN32)
  set (BUILD_ONLY_32_BIT_FOR_WIN64 TRUE)
  message(STATUS "Building ONLY 32 bit support ")
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

include(DBus)

if (WITH_DBUS)
  message(STATUS "Checking for: Jinja2")
  if (Python3_FOUND)
    find_python_module(jinja2)
  endif()
  if (NOT PY_JINJA2)
    set (WITH_DBUS FALSE)
  endif()
endif()

#----------------------------------------------------------------------------------------------------
# GTK

if ("${WITH_UI}" STREQUAL "Gtk+3")
  message(STATUS "Checking for: Gtk 3")

  if (NOT VCPKG_TOOLCHAIN)
    pkg_check_modules(
      GTK REQUIRED
      gtkmm-3.0>=3.22.0
      gtk+-3.0>=3.22.0)

    pkg_check_modules(
      GLIB REQUIRED
      glib-2.0>=2.56.0
      gio-2.0>=2.56.0)
  else()
    set(LIBS atk-1.0 atkmm cairo cairomm-1.0 gdk_pixbuf-2.0 gdk-3.0 gdkmm gio-2.0 giomm glib-2.0 glibmm gobject-2.0 gtk-3.0 gtkmm libintl pango-1.0 pangomm sigc-2.0)
    foreach(lib ${LIBS})
     find_library(${lib}_PATH ${lib} RERQUIRED)
      if(${lib}_PATH)
        set(GTK_LIBRARIES ${GTK_LIBRARIES} ${${lib}_PATH})
      else()
        message(STATUS "Could not find ${lib}")
      endif()
    endforeach()
    find_path(GTK_INCLUDE_DIRS gtkmm.h)
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

  include_directories(${GLIB_INCLUDE_DIRS})
  find_package(GSettings)
endif()


#----------------------------------------------------------------------------------------------------
# Qt

if ("${WITH_UI}" STREQUAL "Qt")
  message(STATUS "Checking for: Qt")

  find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui Svg LinguistTools REQUIRED)

  get_target_property(QT_LRELEASE_EXECUTABLE Qt::lrelease LOCATION)

  if (TARGET Qt::lconvert)
    get_target_property(QT_LCONVERT_EXECUTABLE Qt::lconvert LOCATION)
  else()
    get_filename_component(QT_LRELEASE_PATH ${QT_LRELEASE_EXECUTABLE} PATH)
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

option(WITH_GNOME_CLASSIC_PANEL "Enable Gnome 3 Classic pannel applet support" ${WITH_GNOME_CLASSIC_PANEL_DEFAULT})

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
# Indicator

if (UNIX AND NOT APPLE)
  option(WITH_INDICATOR "Enable Ayatana indicator support" ON)
  option(LOCALINSTALL "Install file locally instead of to system location" OFF)

  pkg_check_modules(
    GTK3
    gtk+-3.0>=3.10.0)

  if (WITH_INDICATOR)
    message(STATUS "Checking for: Unity Indicators")

    find_package(GObjectIntrospection 0.9.12)
    set (HAVE_INTROSPECTION ${INTROSPECTION_FOUND})

    if (HAVE_INTROSPECTION)
      pkg_check_modules(INDICATOR QUIET ayatana-indicator3-0.4 dbusmenu-glib-0.4 dbusmenu-gtk3-0.4)
    endif()

    if (INDICATOR_FOUND)
      set (HAVE_INDICATOR ON)
      set (HAVE_INDICATOR_AYATANA ON)
    else()
      set (WITH_INDICATOR OFF)
      set (HAVE_INDICATOR_AYATANA OFF)
    endif()
  endif()

  if (HAVE_INDICATOR)
    if (LOCALINSTALL)
      set (INDICATORDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/indicators3/7")
      set (INDICATORICONSDIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/libindicate/icons")
    else()
      pkgconfig_get_var(INDICATORDIR ayatana-indicator3-0.4 indicatordir)
      pkgconfig_get_var(INDICATORICONSDIR ayatana-indicator3-0.4 iconsdir)
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

set(Boost_NO_BOOST_CMAKE TRUE)
set (BOOST_COMPONENTS date_time program_options serialization)
#set (Boost_DEBUG ON)

if (WIN32)
  if (${PLATFORM_ARCH} EQUAL 64)
    set (Boost_ARCHITECTURE "-x64")
  elseif(${PLATFORM_ARCH} EQUAL 32)
    set (Boost_ARCHITECTURE "-x32")
  endif()
endif()

if (MSVC AND (NOT Boost_INCLUDE_DIR OR NOT Boost_LIBRARY_DIR))
  if (CMAKE_CL_64)
    set (_arch lib64)
  else()
    message(FATAL_ERROR "32 bit compilation not supported on Windows")
  endif()
  # if (MSVC_VERSION EQUAL 1900)
  #   set (_vsver 14)
  # else()
  #     message(FATAL_ERROR "Visual Studio 2015 is required to compile Workrave on Windows")
  # endif()
  # set (_libdir "${_arch}-msvc-${_vsver}.0")

  # if (EXISTS "${BOOST_ROOT}/${_libdir}")
  #   set (BOOST_LIBRARYDIR "${BOOST_ROOT}/${_libdir}" CACHE PATH "")
  # endif()
endif()

if (HAVE_TESTS)
  set (BOOST_COMPONENTS ${BOOST_COMPONENTS} test_exec_monitor)
endif()

find_package(Boost 1.54.0 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
add_compile_definitions(BOOST_LOG_DYN_LINK)

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

if(CMAKE_TOOLCHAIN_FILE MATCHES vcpkg)
  find_package(unofficial-gettext REQUIRED)
  set (ENABLE_NLS ON) # TODO: remove ENABLE_NLS
  set (HAVE_LIBINTL ON)
else()
  find_package(Gettext)
  find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)

  if (HAVE_QT)
    set (ENABLE_NLS ON)
    set (HAVE_LIBINTL OFF)
  else()
    find_package(Libintl)

    if (NOT (GETTEXT_FOUND AND LIBINTL_FOUND))
      message(STATUS "GetText not found - disabling i18n")
    else()
      set (ENABLE_NLS ON)
      set (HAVE_LIBINTL ON)
    endif()
  endif()
endif()

set (GETTEXT_PACKAGE "workrave")

# TODO: generate languages.iss for InnoSetup

#----------------------------------------------------------------------------------------------------
# Tracing

option(WITH_TRACING "Enable debug tracing" OFF)

if (WITH_TRACING)
  set (TRACING ON)
  #SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DTRACING")
endif()


#----------------------------------------------------------------------------------------------------
# platform - windows


if (WIN32)
  option(WITH_CRASHPAD "Use crashpad" OFF)
  option(WITH_AUTO_UPDATE "Use unfold auto-update" OFF)
  option(ISCC "Inno Setup Compiler")

  if (WITH_AUTO_UPDATE)
    set (HAVE_UNFOLD_AUTO_UPDATE ON)
  endif()

  set (PLATFORM_OS_WINDOWS ON)
  set (PLATFORM_OS_NAME "Windows")
  if (MSVC)
    set (PLATFORM_OS_WINDOWS_NATIVE 1)
  endif()

  set (HAVE_HARPOON ON)

  find_program(MSYS2_SHELL
               NAMES msys2_shell.cmd
               PATHS
               "$ENV{RUNNER_TEMP}/msys64")
  if (MSYS2_SHELL)
    set(MSYS_CMD ${MSYS2_SHELL})
  else()
    include(FindMsys)
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

  if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE _SCL_SECURE_NO_WARNINGS)

    set (MSVC_WARNINGS "/W3 /wd4127 /wd4250 /wd4413 /wd4800")
    set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /FIconfig.h ${MSVC_WARNINGS}")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /FIconfig.h ${MSVC_WARNINGS}")

    set (CMAKE_DEBUG_POSTFIX "d")

    add_compile_definitions(
      WIN32
      _WINDOWS
      _WIN32_WINNT=0x0601
      WINVER=0x0601
      NOMINMAX
      )
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
  find_library(QTKIT_LIBRARY QTKit REQUIRED)
  include_directories(/usr/local/include)
  include_directories(${CMAKE_SYSTEM_INCLUDE_PATH})
endif()

#----------------------------------------------------------------------------------------------------
# platform - Unix

if (UNIX AND NOT APPLE)
  set (PLATFORM_OS_UNIX ON)
  set (PLATFORM_OS_NAME "Unix")

  set (HAVE_MONITORS "mutter,screensaver,record,x11events")

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

find_package(spdlog QUIET)
if(NOT ${spdlog_FOUND})
  FetchContent_Declare(
      spdlog
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_SHALLOW TRUE
      GIT_TAG v1.10.0
    )
  set(SPDLOG_BUILD_SHARED ON)
  FetchContent_MakeAvailable(spdlog)
endif()
# TODO: remove workaround
add_compile_definitions(FMT_DEPRECATED_OSTREAM)

#----------------------------------------------------------------------------------------------------
# Crash reporting - Windows

if (WIN32 AND (NOT BUILD_ONLY_32_BIT_FOR_WIN64))
  set(HAVE_CRASH_REPORT OFF)
  set(HAVE_CRASHPAD OFF)
  if (WITH_CRASHPAD)

    FetchContent_Declare(
      crashpad
      GIT_REPOSITORY https://github.com/rcaelers/crashpad.git
      # GIT_SHALLOW TRUE
      GIT_TAG c4a59aef60b78cca2b841e89aadf6435c970b44d
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
      GIT_TAG fcd4bf0a3ecdb9a18480c4fae05fba2dce4a5169
    )
    FetchContent_MakeAvailable(dump_syms)

    set(DUMP_SYMS ${dump_syms_SOURCE_DIR}/target/release/dump_syms.exe)
    add_custom_target(dump_syms ALL DEPENDS ${DUMP_SYMS})
    add_custom_command(
      OUTPUT ${DUMP_SYMS}
      COMMAND cargo build --release
      WORKING_DIRECTORY ${dump_syms_SOURCE_DIR}
      )

    set(HAVE_CRASHPAD ON)
    set(HAVE_CRASH_REPORT ON)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-pdb=")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-pdb=")
      add_compile_options(-gdwarf -gcodeview)
    else()
      message(STATUS "Crashpad requires Clang")
    endif()
  endif()
endif()

#----------------------------------------------------------------------------------------------------
#  misc checks

check_function_exists(setlocale HAVE_SETLOCALE)

set (CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)

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

if (NOT BUILD_ONLY_32_BIT_FOR_WIN64)
  add_subdirectory(po)
  add_subdirectory(libs)
  add_subdirectory(ui)
else()
  add_subdirectory(libs/hooks)
  add_subdirectory(ui/app/toolkits/gtkmm/dist/windows/src)
endif()

if (WIN32 AND (NOT BUILD_ONLY_32_BIT_FOR_WIN64))
  if (CMAKE_CROSSCOMPILING)
    set(TOOLCHAIN_32BIT ${CMAKE_SOURCE_DIR}/cmake/toolchains/mingw32-gcc.cmake)
    set(CMAKE_COMMAND32 ${CMAKE_COMMAND})
  else()
    set(TOOLCHAIN_32BIT ${CMAKE_SOURCE_DIR}/cmake/toolchains/msys2.cmake)
    set(CMAKE_COMMAND32 ${MSYS_CMD} -here -mingw32 -no-start -defterm -c "cmake \$*" cmake)
  endif()

  ExternalProject_Add(Workrave32Bit
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    BINARY_DIR ${CMAKE_BINARY_DIR}/.32
    CMAKE_COMMAND ${CMAKE_COMMAND32}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${TOOLCHAIN_32BIT}
      -DCMAKE_INSTALL_PREFIX:FILEPATH=${CMAKE_INSTALL_PREFIX}
      -DWITH_UI:STRING=None
    )
endif()

#----------------------------------------------------------------------------------------------------
# Install

if (WIN32 AND (NOT BUILD_ONLY_32_BIT_FOR_WIN64))
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

feature_text( "Workrave" WORKRAVE_VERSION)
feature_text("Install prefix" CMAKE_INSTALL_PREFIX)
feature_text("OS" PLATFORM_OS_NAME)
feature_text("UI" WITH_UI)
message(STATUS "")
feature_bool("GSettings configuration" HAVE_GSETTINGS)
if (PLATFORM_OS_UNIX)
feature_text("ActivityMonitors" HAVE_MONITORS)
endif()
feature_bool("Gnome 3 Applet"  HAVE_GNOME3_PANEL_APPLET)
feature_bool("Indicator Applet" HAVE_INDICATOR)
feature_bool("XFCE Applet" HAVE_XFCE4)
feature_bool("MATE Applet" HAVE_MATE)
feature_bool("DBUS" HAVE_DBUS)
feature_bool("GStreamer" HAVE_GSTREAMER)
if(PLATFORM_OS_UNIX)
feature_bool("Pulseaudio" HAVE_PULSE)
endif()
if(PLATFORM_OS_WINDOWS)
feature_bool("Crash reporting" HAVE_CRASHPAD)
feature_bool("Auto update" HAVE_UNFOLD_AUTO_UPDATE)
endif()
feature_bool("Debug logs" TRACING)
feature_bool("Tests" HAVE_TESTS)

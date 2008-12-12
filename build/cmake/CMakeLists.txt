
set(ALL_LINGUAS nl de eo pl da es zh_TW ru fr pt_BR nb lt cs he eu ca sv ja bg hu tr el sk zh_CN it sl ro)
set(SOUND_THEMES subtle default)

# Set Workrave version
set(VERSION_MAJOR   "1")
set(VERSION_MINOR   "9")
set(VERSION_RELEASE "2")
set(VERSION_PATCH   "0")
if (VERSION_PATCH)
    set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}-${VERSION_PATCH}" )
else (VERSION_PATCH)
    set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}" )
endif (VERSION_PATCH)

# CMake
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

project(Workrave)

if(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  
  set(CMAKE_CXX_WARNING_LEVEL 2 CACHE TYPE STRING FORCE)
  
  SET(MSVC_W_ERROR   " /we4028 /we4013 /we4133 /we4047 /we4031 /we4002 /we4003 /we4114")
  SET(MSVC_W_DISABLE " /wd4127 /wd4090 /wd4101 /wd4244 /wd4250 /wd4413")
  
  SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /FIconfig.h /vd2 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /FIconfig.h /vd2 ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
endif(MSVC)

if(WIN32)
  set (CMAKE_DEBUG_POSTFIX "d")

  add_definitions( 
    -DWIN32
    -D_WINDOWS	   
  ) 

endif(WIN32)
  

if (WIN32)
  set (LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin)
else (WIN32)
  set (LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
endif (WIN32)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# for including config.h and for includes like <dir/foo.h>
include_directories(${CMAKE_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR} ${CMAKE_INCLUDE_PATH})

# linker search directories
link_directories(${LIBRARY_OUTPUT_PATH})
include_directories(${CMAKE_LIBRARY_PATH})

######################################################################
## Win32
######################################################################

if (WIN32)
  set(PLATFORM_OS_WIN32 1)
  if (MSVC)
    set(PLATFORM_OS_WIN32_NATIVE 1)
  endif (MSVC)
endif (WIN32)

######################################################################
## Options
######################################################################

option(DEVELOPMENT_MODE "Development" OFF)
if(DEVELOPMENT_MODE)
  set(DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../)
endif(DEVELOPMENT_MODE)

######################################################################
## Find packages
######################################################################

include(FindGlib)
include(FindGtkmm)

include(FindGettext)
FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
FIND_PROGRAM(INTLTOOL_UPDATE_EXECUTABLE intltool-update)
FIND_PROGRAM(INTLTOOL_MERGE_EXECUTABLE intltool-merge)
MARK_AS_ADVANCED(INTLTOOL_UPDATE)

if (NOT GETTEXT_FOUND)
  message("=== GetText 0.17 not found")
  message("=== Please install Getttext")
  message(FATAL_ERROR "Aborting")
endif (NOT GETTEXT_FOUND)

if (WIN32)
  include(FindPerl)
  if (NOT PERL_FOUND)
    message("=== Perl not found")
    message("=== Please install Perl")
    message(FATAL_ERROR "Aborting")
  else (NOT PERL_FOUND)
    set (INTLTOOL_MERGE_EXECUTABLE ${PERL_EXECUTABLE} ${INTLTOOL_MERGE_EXECUTABLE})
  endif (NOT PERL_FOUND)
endif (WIN32)

######################################################################
## Create config.h
######################################################################

if (MINGW)
  set (HAVE_GNUC_VARARGS 1)
endif(MINGW)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )


# compiler definitions 
add_definitions(-DHAVE_CONFIG_H=1)

######################################################################
## Subdirectories
######################################################################

add_subdirectory(harpoon)
add_subdirectory(common)
add_subdirectory(backend)
add_subdirectory(frontend)


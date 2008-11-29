
# Set Workrave version
set (VERSION_MAJOR   "1")
set (VERSION_MINOR   "9")
set (VERSION_RELEASE "2")
set (VERSION_PATCH   "0")
if (VERSION_PATCH)
    set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}-${VERSION_PATCH}" )
else (VERSION_PATCH)
    set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}" )
endif (VERSION_PATCH)

# CMake
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

project(Workrave)


if(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  
  # Use the highest warning level
  if (WALL)
    set(WALL 1 CACHE TYPE STRING FORCE)
    set(CMAKE_CXX_WARNING_LEVEL 4 CACHE TYPE STRING FORCE)
    
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
      STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    else(CMAKE_C_FLAGS MATCHES "/W[0-4]")
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    endif(CMAKE_C_FLAGS MATCHES "/W[0-4]")
  else (WALL)
    set(CMAKE_CXX_WARNING_LEVEL 3 CACHE TYPE STRING FORCE)
  endif (WALL)
  
  SET(MSVC_W_ERROR   " /we4028 /we4013 /we4133 /we4047 /we4031 /we4002 /we4003 /we4114")
  SET(MSVC_W_DISABLE " /wd4127 /wd4090 /wd4101 /wd4244")
  
  SET(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /FIconfig.h ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
  SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /FIconfig.h ${MSVC_W_ERROR} ${MSVC_W_DISABLE}")
endif(MSVC)

if(WIN32)
  set (CMAKE_DEBUG_POSTFIX "d")
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

add_subdirectory(common)
add_subdirectory(backend)


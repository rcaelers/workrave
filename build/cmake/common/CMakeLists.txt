project(Workrave)

SET(COMMON_DIR ${CMAKE_SOURCE_DIR}/../../common)

set(COMMON_SOURCES 
  ${COMMON_DIR}/src/debug.cc
  ${COMMON_DIR}/src/GlibThread.cc
  ${COMMON_DIR}/src/Locale.cc
  ${COMMON_DIR}/src/StringUtil.cc
  ${COMMON_DIR}/src/Util.cc
  ${COMMON_DIR}/src/WRID.cc
  ${COMMON_DIR}/include/Exception.hh
  ${COMMON_DIR}/include/GlibMutex.hh
  ${COMMON_DIR}/include/GlibThread.hh
  ${COMMON_DIR}/include/Locale.hh
  ${COMMON_DIR}/include/Mutex.hh
  ${COMMON_DIR}/include/QtMutex.hh
  ${COMMON_DIR}/include/Runnable.hh
  ${COMMON_DIR}/include/StringUtil.hh
  ${COMMON_DIR}/include/Thread.hh
  ${COMMON_DIR}/include/Util.hh
  ${COMMON_DIR}/include/debug.hh
  ${COMMON_DIR}/include/nls.h
  ${COMMON_DIR}/include/timeutil.h
)

if (HAVE_DBUS)
  set (COMMON_SOURCES ${COMMON_SOURCES}
    ${COMMON_DIR}/include/DBus.hh
    ${COMMON_DIR}/include/DBusBinding.hh
    ${COMMON_DIR}/include/DBusException.hh
    ${COMMON_DIR}/src/DBus.cc
    ${COMMON_DIR}/src/DBusBinding.cc
  )
  include_directories(${DBUS_INCLUDES})
endif (HAVE_DBUS)

if (WIN32)
  set (COMMON_SOURCES ${COMMON_SOURCES}
    ${COMMON_DIR}/src/win32/crashlog.cc
    ${COMMON_DIR}/include/win32/Sound.hh
    ${COMMON_DIR}/include/win32/crashlog.h
    ${COMMON_DIR}/include/win32/w32debug.hh
  )
endif (WIN32)

if (UNIX)
  set (COMMON_SOURCES ${COMMON_SOURCES}
    ${COMMON_DIR}/include/unix/Sound.hh
  )
endif (UNIX)

add_definitions(-D_XOPEN_SOURCE=600)

include_directories( 
  "${COMMON_DIR}/include"
  "${COMMON_DIR}/include/win32"
  "${COMMON_DIR}/win32/harpoon/include"
)

if (HAVE_DBUS)
  include_directories(${DBUS_INCLUDES})
endif(HAVE_DBUS)
include_directories(${GLIB_INCLUDES})

add_library(workrave-common STATIC ${COMMON_SOURCES})

project(Workrave)

message (${CMAKE_SOURCE_DIR}) 
SET(COMMON_DIR ${CMAKE_SOURCE_DIR}/../../common)

set(COMMON_SOURCES 
  ${COMMON_DIR}/src/debug.cc
  ${COMMON_DIR}/src/GlibThread.cc
  ${COMMON_DIR}/src/Locale.cc
  ${COMMON_DIR}/src/StringUtil.cc
  ${COMMON_DIR}/src/Util.cc
)

if (HAVE_DBUS)
  set (COMMON_SOURCES ${COMMON_SOURCES}
    ${COMMON_DIR}/src/DBus.cc
    ${COMMON_DIR}/src/DBusBinding.cc
  )
endif (HAVE_DBUS)

if (WIN32)
  set (COMMON_SOURCES ${COMMON_SOURCES}
    ${COMMON_DIR}/src/win32/crashlog.cc
  )
endif (WIN32)

add_definitions(-D_XOPEN_SOURCE=600)

include_directories( 
  "${COMMON_DIR}/include"
  "${COMMON_DIR}/include/win32"
  "${COMMON_DIR}/win32/harpoon/include"
)

add_library( workrave-common STATIC ${COMMON_SOURCES} )

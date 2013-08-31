set(SRC
  src/Locale.cc
  src/TimeSource.cc
  src/Util.cc
  src/debug.cc
  src/Platform.cc
  )

#  src/Timer.cc

if (HAVE_GLIB)
  set(SRC ${SRC} src/GlibThread.cc)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} src/win32/crashlog.cc src/win32/W32ActiveSetup.cc W32CriticalSection.cc)
endif()

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

add_definitions(
  -DWORKRAVE_PKGDATADIR="${PKGDATADIR}"
  -DWORKRAVE_DATADIR="${DATADIR}"
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-utils STATIC ${SRC})


# TODO: Generate locale.inc
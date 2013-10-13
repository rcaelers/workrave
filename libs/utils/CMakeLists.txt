set(SRC
  src/Locale.cc
  src/TimeSource.cc
  src/AssetPath.cc
  src/debug.cc
  src/Platform.cc
  )

if (HAVE_GLIB)
  set(SRC ${SRC} src/GlibThread.cc)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} src/Platform-win32.cc rc/win32/crashlog.cc src/win32/W32ActiveSetup.cc src/win32/W32CriticalSection.cc )
endif()

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC} src/Platform-unix.cc)
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
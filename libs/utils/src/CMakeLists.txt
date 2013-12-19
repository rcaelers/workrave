set(SRC
  Locale.cc
  TimeSource.cc
  AssetPath.cc
  debug.cc
  )

if (TRANSLATIONS_1)
  set(SRC ${SRC} debug.cc)
endif()

if (HAVE_GLIB)
  set(SRC ${SRC} GlibThread.cc)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} Platform-win32.cc rc/win32/crashlog.cc win32/W32ActiveSetup.cc win32/W32CriticalSection.cc )
endif()

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC} Platform-unix.cc)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC} Platform-unix.cc)
endif()

if (NOT PLATFORM_OS_UNIX)
  set(SRC ${SRC} Platform.cc)
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
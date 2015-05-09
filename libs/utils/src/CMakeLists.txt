set(SRC
  Locale.cc
  TimeSource.cc
  AssetPath.cc
  debug.cc
  )

if (TRANSLATIONS_1)
  set(SRC ${SRC} debug.cc)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} Platform-win32.cc win32/crashlog.cc win32/W32ActiveSetup.cc win32/W32CriticalSection.cc win32/dll_hell.c)

  include_directories(
    ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include
    )
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

if (HAVE_DBUS_GIO)
  set(SRC ${SRC} DBusProxy-gio.cc)
endif()
if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-utils STATIC ${SRC})
add_cppcheck(workrave-libs-utils)


# TODO: Generate locale.inc
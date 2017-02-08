set(SRC
  TimeSource.cc
  AssetPath.cc
  debug.cc
  )

if (TRANSLATIONS_1)
  set(SRC ${SRC} debug.cc)
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} Platform-win32.cc win32/W32ActiveSetup.cc win32/W32CriticalSection.cc)

  if (NOT CMAKE_CL_64)
    set(SRC ${SRC} win32/crashlog.cc)
  endif()
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

add_library(workrave-libs-utils STATIC ${SRC})
add_cppcheck(workrave-libs-utils)

if (HAVE_GTK)
  target_include_directories(workrave-libs-utils PRIVATE ${GTK_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_WIN32)
  target_include_directories(workrave-libs-utils PRIVATE ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

target_include_directories(
  workrave-libs-utils
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )


# TODO: Generate locale.inc

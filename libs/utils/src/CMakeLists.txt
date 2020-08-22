set(SRC
  TimeSource.cc
  AssetPath.cc
  debug.cc
  )

if (PLATFORM_OS_WINDOWS)
  set(SRC ${SRC} Platform-win32.cc win32/W32ActiveSetup.cc win32/W32CriticalSection.cc)

  if (PLATFORM_OS_WINDOWS_32)
    set(SRC ${SRC} win32/crashlog.c)
  endif()
endif()

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC} Platform-unix.cc)
endif()

if (PLATFORM_OS_MACOS)
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

if (HAVE_QT5)
  target_include_directories(workrave-libs-utils PRIVATE ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_WINDOWS)
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

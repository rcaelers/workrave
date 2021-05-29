add_library(workrave-libs-utils STATIC
  Diagnostics.cc
  TimeSource.cc
  AssetPath.cc
  Util.cc
  StringUtil.cc
  WRID.cc
  debug.cc)

target_code_coverage(workrave-libs-utils)

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-utils PRIVATE Platform-win32.cc win32/W32ActiveSetup.cc win32/W32CriticalSection.cc)
  target_include_directories(workrave-libs-utils PRIVATE ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

if (PLATFORM_OS_UNIX)
  target_sources(workrave-libs-utils PRIVATE Platform-unix.cc)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-libs-utils PRIVATE Platform-unix.cc)
endif()

if (NOT PLATFORM_OS_UNIX)
  target_sources(workrave-libs-utils PRIVATE Platform.cc)
endif()

if (HAVE_DBUS_GIO)
  target_sources(workrave-libs-utils PRIVATE DBusProxy-gio.cc)
endif()

if (HAVE_GTK)
  target_include_directories(workrave-libs-utils PRIVATE ${GTK_INCLUDE_DIRS})
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-libs-utils PRIVATE ${GLIB_INCLUDE_DIRS})
  target_link_libraries(workrave-libs-utils PRIVATE ${GLIB_LIBS})
endif()

if (HAVE_QT)
  target_link_libraries(workrave-libs-utils PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)
endif()

if (HAVE_QT5)
  target_include_directories(workrave-libs-utils PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()


target_include_directories(
  workrave-libs-utils
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/utils/include)

# TODO: Generate locale.inc

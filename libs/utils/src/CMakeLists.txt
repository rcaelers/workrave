add_library(workrave-libs-utils STATIC)

target_sources(workrave-libs-utils PRIVATE
  Logging.cc
  Diagnostics.cc
  TimeSource.cc
  AssetPath.cc
  Paths.cc
  debug.cc)

target_code_coverage(workrave-libs-utils)

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-utils PRIVATE Platform-windows.cc windows/W32ActiveSetup.cc windows/W32CriticalSection.cc windows/StringUtils.cc)
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
  target_link_libraries(workrave-libs-utils PUBLIC ${GLIB_LIBRARIES})
endif()

if (HAVE_QT)
  target_link_libraries(workrave-libs-utils PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)
	target_link_libraries(workrave-libs-utils PRIVATE Qt::GuiPrivate)
endif()

target_link_libraries(workrave-libs-utils PUBLIC fmt::fmt spdlog::spdlog PRIVATE Boost::boost)

target_include_directories(
  workrave-libs-utils
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/utils/include)

# TODO: Generate locale.inc

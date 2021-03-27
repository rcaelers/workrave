add_library(workrave-libs-session STATIC System.cc)

if (PLATFORM_OS_UNIX)
  target_sources(workrave-libs-session PRIVATE ScreenLockCommandline.cc)

  if (HAVE_DBUS_GIO)
  target_sources(workrave-libs-session PRIVATE 
      ScreenLockDBus.cc
      SystemStateChangeConsolekit.cc
      SystemStateChangeLogind.cc
      SystemStateChangeUPower.cc)
   endif()
endif()

if (WIN32)
  target_sources(workrave-libs-session PRIVATE 
    win32/W32LockScreen.cc
    win32/W32Shutdown.cc)
  target_include_directories(workrave-libs-session PRIVATE 
    ${CMAKE_SOURCE_DIR}/libs/session/src/win32/ 
    ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

if (HAVE_GLIB)
  target_include_directories(workrave-libs-session PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-session workrave-libs-utils)
target_include_directories(workrave-libs-session PUBLIC ${CMAKE_SOURCE_DIR}/libs/session/include)

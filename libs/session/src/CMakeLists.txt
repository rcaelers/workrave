set(SRC
  System.cc
 )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    ScreenLockCommandline.cc
    )

  if (HAVE_DBUS_GIO)
    set(SRC ${SRC}
      ScreenLockDBus.cc
      SystemStateChangeConsolekit.cc
      SystemStateChangeLogind.cc
      SystemStateChangeUPower.cc
      )
   endif()
endif()

if (WIN32)
  set(SRC ${SRC}
    win32/W32LockScreen.cc
    win32/W32Shutdown.cc
    )
  include_directories(${CMAKE_SOURCE_DIR}/libs/session/src/win32/ ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

add_library(workrave-libs-session STATIC ${SRC})
add_cppcheck(workrave-libs-session)

if (HAVE_GLIB)
  target_include_directories(workrave-libs-session PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-session workrave-libs-utils)

target_include_directories(workrave-libs-session PUBLIC ${CMAKE_SOURCE_DIR}/libs/session/include)

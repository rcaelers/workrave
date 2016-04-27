set(SRC
  System.cc
 )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    ScreenLockCommandline.cc
    )

  set(SRC ${SRC}
    ScreenLockDBus.cc
    SystemStateChangeConsolekit.cc
    SystemStateChangeLogind.cc
    SystemStateChangeUPower.cc
    )
endif()

if (WIN32)
  set(SRC ${SRC}
    win32/W32LockScreen.cc
    win32/W32Shutdown.cc
    )
  include_directories(${CMAKE_SOURCE_DIR}/libs/session/src/win32/ ${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/session/include/session
  ${CMAKE_SOURCE_DIR}/libs/session/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-session STATIC ${SRC})
add_cppcheck(workrave-libs-session)

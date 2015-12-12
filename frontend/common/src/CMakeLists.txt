set(SRC
  Backend.cc
  Exercise.cc
  GUIConfig.cc
  Session.cc
  SoundTheme.cc
  System.cc
  Text.cc
  TimerBoxControl.cc
  Ui.cc
 )

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    ScreenLockCommandline.cc
    )
endif()

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
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
  include_directories(${CMAKE_SOURCE_DIR}/frontend/common/src/win32/ ${CMAKE_SOURCE_DIR}/src/libs/hooks/harpoon/include)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/apps/common/include
  ${CMAKE_SOURCE_DIR}/frontend/common/include
  ${CMAKE_SOURCE_DIR}/src/libs/core/include
  ${CMAKE_SOURCE_DIR}/src/libs/utils/include
  ${CMAKE_SOURCE_DIR}/src/libs/config/include
  ${CMAKE_SOURCE_DIR}/src/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/src/libs/audio/include
  )

#if (HAVE_DBUS AND NOT HAVE_GTK)
#  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
#  set(SRC ${SRC}
#    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
#  )
#endif()

add_library(workrave-frontend-common STATIC ${SRC})
add_cppcheck(workrave-frontend-common)

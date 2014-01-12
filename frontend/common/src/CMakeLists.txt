set(SRC
  Exercise.cc
  CoreFactory.cc
  GUIConfig.cc
  Session.cc
  System.cc
  Text.cc
  SoundTheme.cc
  Ui.cc
 )

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

if (WIN32)
  include_directories(${CMAKE_SOURCE_DIR}/libs/hooks/harpoon/include)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/apps/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/
  ${CMAKE_SOURCE_DIR}/backend/include/
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  )

if (HAVE_DBUS AND NOT HAVE_GTK)
  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
  )
endif()

add_library(workrave-frontend-common STATIC ${SRC})

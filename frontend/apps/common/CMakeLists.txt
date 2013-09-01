set(SRC
  src/Application.cc
  src/MenuModelItem.cc
  src/MenuModel.cc
 )

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/apps/common/include/
  ${CMAKE_SOURCE_DIR}/frontend/common/include/
  ${CMAKE_SOURCE_DIR}/backend/include/
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  )

if (HAVE_DBUS)
  dbus_generate(DBusGUI ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-gui.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc)
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusGUI.cc
  )
endif (HAVE_DBUS)

add_library(workrave-frontend-apps-common STATIC ${SRC})

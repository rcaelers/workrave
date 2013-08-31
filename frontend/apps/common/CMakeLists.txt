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

add_library(workrave-frontend-apps-common STATIC ${SRC})

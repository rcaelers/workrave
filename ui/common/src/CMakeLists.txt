set(SRC
  Backend.cc
  Exercise.cc
  GUIConfig.cc
  SoundTheme.cc
  Session.cc
  TimerBoxControl.cc
 )

include_directories(
  ${CMAKE_SOURCE_DIR}/ui/common/include/commonui
  ${CMAKE_SOURCE_DIR}/ui/common/include
  ${CMAKE_SOURCE_DIR}/libs/core/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  )

add_library(workrave-ui-common STATIC ${SRC})
add_cppcheck(workrave-ui-common)

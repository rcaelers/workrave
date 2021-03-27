set(SRC
  # Backend.cc
  Exercise.cc
  GUIConfig.cc
  #SoundTheme.cc
  Session.cc
  Text.cc
  TimerBoxControl.cc
)

add_library(workrave-ui-common STATIC ${SRC})

target_link_libraries(workrave-ui-common
  workrave-libs-utils
  workrave-libs-audio
  workrave-libs-core)

target_include_directories(
  workrave-ui-common
  PUBLIC
  ${CMAKE_SOURCE_DIR}/ui/common/include
  PRIVATE
  ${CMAKE_SOURCE_DIR}/ui/common/include/commonui
  )

if (HAVE_GLIB)
  target_include_directories(workrave-ui-common PRIVATE ${GLIB_INCLUDE_DIRS})
endif()

set(SRC
  Backend.cc
  Exercise.cc
  GUIConfig.cc
  SoundTheme.cc
  Session.cc
  TimerBoxControl.cc
)

add_library(workrave-ui-common STATIC ${SRC})

target_link_libraries(workrave-ui-common
  workrave-libs-utils
  workrave-libs-audio)

if (HAVE_CORE_NEXT)
  target_link_libraries(workrave-ui-common workrave-libs-core-next)
else()
  target_link_libraries(workrave-ui-common workrave-libs-core)
endif()

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

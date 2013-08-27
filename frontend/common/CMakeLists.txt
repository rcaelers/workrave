
set(SRC
  src/Exercise.cc
  src/CoreFactory.cc
  src/Makefile.am
  src/System.cc
  src/Text.cc
  src/TimerBoxControl.cc
  src/SoundTheme.cc
 )

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/common/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  ${CMAKE_SOURCE_DIR}/backend/include
  )

add_library(workrave-frontend-common STATIC ${SRC})

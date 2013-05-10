
set(SRC
  src/CoreFactory.cc
  src/Makefile.am
  src/SoundPlayer.cc
  src/System.cc
  src/Text.cc
  src/TimerBoxControl.cc
 )

if (HAVE_GSTREAMER)
  set(SRC ${SRC}
    src/GstSoundPlayer.cc
    )
  include_directories(${GSTREAMER_INCLUDE_DIRS})
endif()

if (HAVE_PULSE)
  set(SRC ${SRC}
    src/PulseMixer.cc
    )
  include_directories(${PULSE_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    src/win32/Sound.cc
    src/win32/W32DirectSoundPlayer.hh
    src/win32/W32Mixer.cc
    src/win32/W32SoundPlayer.cc
    )
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    src/osx/OSXSoundPlayer.cc
    src/osx/Sound.cc)
endif()

if (PLATFORM_OS_UNIX)
  set(SRC ${SRC}
    src/x11/Sound.cc)
endif()

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/frontend/common/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  ${CMAKE_SOURCE_DIR}/backend/include
  )

add_library(workrave-frontend-common STATIC ${SRC})

if (HAVE_GSTREAMER)
  link_directories(${GSTREAMER_LIBPATH})
  target_link_libraries(workrave-frontend-common ${GSTREAMER_LIBRARIES})
endif()

if (HAVE_PULSE)
  link_directories(${PULSE_LIBPATH})
  target_link_libraries(workrave-frontend-common ${PULSE_LIBRARIES})
endif()

#----------------------------------------------------------------------------------------------------
# Install

foreach(size 16x16 24x24 32x32 48x48 64x64 96x96 128x128 scalable)
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/share/images/${size}/
    DESTINATION share/icons/hicolor/${size}/apps/
    FILES_MATCHING
    PATTERN "*.png"
    PATTERN "*.svg"
    )
endforeach()


FILE(GLOB images
  ${CMAKE_CURRENT_SOURCE_DIR}/share/images/*.png
  ${CMAKE_CURRENT_SOURCE_DIR}/share/images/*.svg
  ${CMAKE_CURRENT_SOURCE_DIR}/share/images/*.ico)
  
install(FILES ${images} DESTINATION share/workrave/images)

# TODO: install win32/osx specific files.

set (SOUND_THEMES default subtle)

foreach(soundtheme ${SOUND_THEMES})
  file(GLOB SND_FILES  ${CMAKE_CURRENT_SOURCE_DIR}/share/sounds/${soundtheme}/*.wav
	  	       ${CMAKE_CURRENT_SOURCE_DIR}/share/sounds/${soundtheme}/soundtheme)

  install(FILES ${SND_FILES} DESTINATION share/sounds/workrave/${soundtheme}
  )
endforeach()


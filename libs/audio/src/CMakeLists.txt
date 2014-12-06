set(SRC
  SoundPlayer.cc
 )

if (HAVE_GSTREAMER)
  set(SRC ${SRC}
    GstSoundPlayer.cc
    )
  include_directories(${GSTREAMER_INCLUDE_DIRS})
endif()

if (HAVE_PULSE)
  set(SRC ${SRC}
    PulseMixer.cc
    )
  include_directories(${PULSE_INCLUDE_DIRS})
endif()

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC}
    win32/W32DirectSoundPlayer.cc
    win32/W32Mixer.cc
    win32/W32SoundPlayer.cc
    )
  include_directories(${CMAKE_SOURCE_DIR}/libs/audio/src/win32)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC}
    osx/OSXSoundPlayer.cc)
  include_directories(${CMAKE_SOURCE_DIR}/libs/audio/src/osx)
  set_source_files_properties(osx/OSXSoundPlayer.cc PROPERTIES COMPILE_FLAGS "-x objective-c++")  
endif()

if (HAVE_GTK)
  include_directories(${GTK_INCLUDE_DIRS})
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/audio/src
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  )

add_library(workrave-libs-audio STATIC ${SRC})

if (HAVE_GSTREAMER)
  link_directories(${GSTREAMER_LIBPATH})
  target_link_libraries(workrave-libs-audio ${GSTREAMER_LIBRARIES})
endif()

if (HAVE_PULSE)
  link_directories(${PULSE_LIBPATH})
  target_link_libraries(workrave-libs-audio ${PULSE_LIBRARIES})
endif()

if (PLATFORM_OS_OSX)
  target_link_libraries(workrave-libs-audio ${QTKIT_LIBRARY})
endif()

add_library(workrave-libs-audio STATIC SoundPlayer.cc)

if (HAVE_GSTREAMER)
  target_sources(workrave-libs-audio PRIVATE GstSoundPlayer.cc)
  target_include_directories(workrave-libs-audio PRIVATE ${GSTREAMER_INCLUDE_DIRS})
  target_link_libraries(workrave-libs-audio ${GSTREAMER_LIBPATH})
  target_link_libraries(workrave-libs-audio ${GSTREAMER_LIBRARIES})
endif()

if (HAVE_PULSE)
  target_sources(workrave-libs-audio PRIVATE PulseMixer.cc)
  target_include_directories(workrave-libs-audio PRIVATE ${PULSE_INCLUDE_DIRS})
  target_link_libraries(workrave-libs-audio ${PULSE_LIBPATH})
  target_link_libraries(workrave-libs-audio ${PULSE_LIBRARIES})
endif()

if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-libs-audio PRIVATE
    windows/W32DirectSoundPlayer.cc
    windows/W32Mixer.cc
    windows/W32SoundPlayer.cc)
    target_include_directories(workrave-libs-audio PRIVATE ${CMAKE_SOURCE_DIR}/libs/audio/src/windows)
endif()

if (PLATFORM_OS_MACOS)
  target_sources(workrave-libs-audio PRIVATE macos/MACOSSoundPlayer.cc)
  target_include_directories(workrave-libs-audio PRIVATE ${CMAKE_SOURCE_DIR}/libs/audio/src/macos)
  set_source_files_properties(macos/MacOSSoundPlayer.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

if (HAVE_GTK)
  target_include_directories(workrave-libs-audio PRIVATE ${GTK_INCLUDE_DIRS})
endif()

target_link_libraries(workrave-libs-audio workrave-libs-utils)

target_include_directories(
  workrave-libs-audio
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/audio/include
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/audio/src
  )

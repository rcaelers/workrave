set(SRC
  Updater.cc
  )

if (PLATFORM_OS_WIN32)
endif()

if (PLATFORM_OS_OSX)
  set(SRC ${SRC} SparkleUpdater.mm)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/updater/src
  ${CMAKE_SOURCE_DIR}/libs/updater/include
  ${CMAKE_SOURCE_DIR}/libs/updater/include/updater
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  "${CMAKE_CURRENT_SOURCe_DIR}/../ext/sparkle"
  )

add_library(workrave-libs-updater STATIC ${SRC})

if (PLATFORM_OS_OSX)
  target_link_libraries(workrave-libs-updater ${COCOA_LIBRARY} "${CMAKE_CURRENT_BINARY_DIR}/../ext/sparkle/Release/Sparkle.framework")
  add_dependencies(workrave-libs-updater Sparkle)
endif()


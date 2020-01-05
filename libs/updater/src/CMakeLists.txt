set(SRC
  Updater.cc
  )

if (PLATFORM_OS_WINDOWS)
endif()

if (PLATFORM_OS_OSX)
  # TODO: set(SRC ${SRC} SparkleUpdater.mm)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/updater/src
  ${CMAKE_SOURCE_DIR}/libs/updater/include
  ${CMAKE_SOURCE_DIR}/libs/updater/include/updater
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

# TODO:

#add_library(workrave-libs-updater STATIC ${SRC})
#add_cppcheck(workrave-libs-updater)

#if (PLATFORM_OS_OSX)
#  set_target_properties(workrave-libs-updater PROPERTIES LINK_FLAGS "-Wl,-rpath/${CMAKE_CURRENT_BINARY_DIR}/../ext/sparkle/Release/")
#  target_link_libraries(workrave-libs-updater "-F/${CMAKE_CURRENT_BINARY_DIR}/../ext/sparkle/Release/ -framework Sparkle")
#  add_dependencies(workrave-libs-updater Sparkle)
#endif()


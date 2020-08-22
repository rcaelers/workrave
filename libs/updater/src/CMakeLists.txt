set(SRC
  Updater.cc
  )

add_library(workrave-libs-updater STATIC ${SRC})
add_cppcheck(workrave-libs-updater)

target_include_directories(
  workrave-libs-updater
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/updater/src
  ${CMAKE_SOURCE_DIR}/libs/updater/include/updater
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/updater/include
  )

target_link_libraries(workrave-libs-updater workrave-libs-utils)

#if (PLATFORM_OS_MACOS)
#  set_target_properties(workrave-libs-updater PROPERTIES LINK_FLAGS "-Wl,-rpath/${CMAKE_CURRENT_BINARY_DIR}/../ext/sparkle/Release/")
#  target_link_libraries(workrave-libs-updater "-F/${CMAKE_CURRENT_BINARY_DIR}/../ext/sparkle/Release/ -framework Sparkle")
#  add_dependencies(workrave-libs-updater Sparkle)
#endif()


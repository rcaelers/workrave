add_library(workrave-libs-updater STATIC
  Config.cc
)

target_code_coverage(workrave-libs-updater)

target_link_libraries(workrave-libs-updater
  PUBLIC
  workrave-libs-utils
  workrave-libs-config
  Unfold::unfold)

target_include_directories(
  workrave-libs-updater
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/updater/include)

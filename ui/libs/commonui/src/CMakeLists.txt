add_library(workrave-libs-commonui STATIC)

target_sources(workrave-libs-commonui PRIVATE
  Exercise.cc
  Locale.cc
  MenuHelper.cc
  MenuModel.cc
  Text.cc
)

target_link_libraries(workrave-libs-commonui
  PUBLIC
  workrave-libs-core
  workrave-libs-utils)

target_include_directories(
    workrave-libs-commonui
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include/commonui
)

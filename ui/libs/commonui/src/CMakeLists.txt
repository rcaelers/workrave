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
  workrave-libs-utils)

if (HAVE_CORE_NEXT)
  target_link_libraries(workrave-libs-commonui PRIVATE workrave-libs-core-next)
else()
  target_link_libraries(workrave-libs-commonui PRIVATE workrave-libs-core)
endif()

target_include_directories(
    workrave-libs-commonui
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include
    PRIVATE
    ${CMAKE_SOURCE_DIR}/ui/libs/commonui/include/commonui
    ${GLIB_INCLUDE_DIRS}
)

if (PLATFORM_OS_MACOS)
  set_source_files_properties(Locale.cc PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
endif()

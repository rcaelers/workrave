add_library(workrave-libs-core-shadow STATIC
  CoreShadow.cc
  CoreShadowClient.cc
  CoreShadowProxy.cc
  CoreShadowTypes.cc)

target_include_directories(workrave-libs-core-shadow
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/core-shadow/include)

target_link_libraries(workrave-libs-core-shadow
  PUBLIC
  workrave-libs-core-next
  PRIVATE
  workrave-libs-config
  workrave-libs-utils
  fmt::fmt)

if (PLATFORM_OS_UNIX OR PLATFORM_OS_MACOS)
  target_sources(workrave-libs-core-shadow PRIVATE CoreShadowProcess-posix.cc)
endif()

add_executable(workrave-core-shadow-helper
  CoreShadowHelper.cc
  CoreShadowTypes.cc)

target_include_directories(workrave-core-shadow-helper
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/libs/core-shadow/include)

target_link_libraries(workrave-core-shadow-helper
  PRIVATE
  workrave-libs-core
  workrave-libs-config
  workrave-libs-utils
  fmt::fmt)

if (NOT PLATFORM_OS_MACOS)
  install(TARGETS workrave-core-shadow-helper RUNTIME DESTINATION "${BINDIR}")
endif()

# The vocabulary both cores speak: break ids, operation and usage modes. The
# two cores are separate implementations of one API, so these types are shared
# rather than duplicated. The @rpc annotations are read by each core's own
# binding generator, through its include path.
add_library(workrave-libs-core-api INTERFACE)

target_link_libraries(workrave-libs-core-api INTERFACE workrave-libs-utils)

target_include_directories(workrave-libs-core-api
  INTERFACE
  ${CMAKE_SOURCE_DIR}/libs/core-api/include)

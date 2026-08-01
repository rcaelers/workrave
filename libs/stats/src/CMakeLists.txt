add_library(workrave-libs-stats STATIC
  FileStatisticsStore.cc
  LocalTime.cc
  SqliteStatisticsStore.cc
  Statistics.cc
  StatisticsStoreFactory.cc)

target_code_coverage(workrave-libs-stats AUTO)

target_link_libraries(workrave-libs-stats PUBLIC workrave-libs-core-api)

# Statistics counts mouse and keyboard activity itself. Test binaries that set
# USE_STUBS get the stub monitor, exactly as the cores do.
target_link_libraries(workrave-libs-stats PUBLIC
  $<$<NOT:$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>>:workrave-libs-input-monitor>
  $<$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>:workrave-libs-input-monitor-stub>
)
target_link_libraries(workrave-libs-stats PRIVATE workrave-libs-utils ${SQLITE3_TARGET})

target_include_directories(workrave-libs-stats
  PRIVATE
  ${CMAKE_SOURCE_DIR}/libs/stats/src
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/stats/include)

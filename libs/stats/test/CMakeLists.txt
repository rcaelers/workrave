if (HAVE_TESTS)
  add_executable(workrave-stats-test FileStatisticsStoreTests.cc SqliteStatisticsStoreTests.cc)
  target_code_coverage(workrave-stats-test AUTO)

  target_link_libraries(workrave-stats-test PRIVATE workrave-libs-stats workrave-libs-utils GTest::gtest_main)
  target_link_libraries(workrave-stats-test PRIVATE ${SQLITE3_TARGET} ${EXTRA_LIBRARIES})
  target_include_directories(workrave-stats-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/stats/src)

  workrave_add_test(workrave-stats-test)
endif()

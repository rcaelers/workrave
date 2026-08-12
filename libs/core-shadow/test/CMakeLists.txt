if (HAVE_TESTS)
  add_executable(workrave-core-shadow-comparator-test CoreShadowComparatorTests.cc)
  target_code_coverage(workrave-core-shadow-comparator-test AUTO)
  set_target_properties(workrave-core-shadow-comparator-test PROPERTIES USE_STUBS ON)
  target_include_directories(workrave-core-shadow-comparator-test PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/core-shadow/src)
  target_link_libraries(workrave-core-shadow-comparator-test PRIVATE
    workrave-libs-core-shadow
    GTest::gtest_main
    ${EXTRA_LIBRARIES})
  workrave_add_test(workrave-core-shadow-comparator-test)
endif()

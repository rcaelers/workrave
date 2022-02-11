if (HAVE_TESTS)
  add_executable(workrave-libs-utils-enum-test EnumTest.cc)
  target_code_coverage(workrave-libs-utils-enum-test AUTO)

  target_link_libraries(workrave-libs-utils-enum-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-libs-utils-enum-test PRIVATE Boost::test_exec_monitor)
  target_link_libraries(workrave-libs-utils-enum-test PRIVATE ${EXTRA_LIBRARIES})

  if (PLATFORM_OS_WINDOWS)
    target_link_libraries(workrave-libs-utils-enum-test PRIVATE libssp)
  endif()

  add_test(NAME workrave-libs-utils-enum-test COMMAND workrave-libs-utils-enum-test)
endif()

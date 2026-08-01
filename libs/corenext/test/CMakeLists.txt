if (HAVE_TESTS)
  add_executable(workrave-core-next-timer-test
    SimulatedTime.cc
    TimerTests.cc)
  target_code_coverage(workrave-core-next-timer-test AUTO)

  set_target_properties(workrave-core-next-timer-test PROPERTIES USE_STUBS ON)

  target_link_libraries(workrave-core-next-timer-test PRIVATE workrave-libs-core-next)
  target_link_libraries(workrave-core-next-timer-test PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-core-next-timer-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-timer-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)

  add_executable(workrave-core-next-integration-test
    ActivityMonitorStub.cc
    IntegrationTests.cc
    SimulatedTime.cc
    )
  target_code_coverage(workrave-core-next-integration-test AUTO EXCLUDE LocalActivityMonitor.cc XmlConfigurator.* GSettingsConfigurator.* usr/include/*)

  set_target_properties(workrave-core-next-integration-test PROPERTIES USE_STUBS ON)

  if (HAVE_APP_GTK)
    target_link_directories(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-core-next)
  if (HAVE_GRPC AND HAVE_CORE_NEXT)
    # This test constructs a real Core (see IntegrationTests.cc), which pulls
    # in Core.o and therefore Core::init_rpc()'s reference to RpcCoreServer —
    # see libs/corenext/src/RpcCoreServer.hh for why that symbol lives in a
    # separate library that must be linked explicitly.
    target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-core-next-rpc)
    target_include_directories(workrave-core-next-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/rpc/include)
  endif()
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-config)
  target_link_libraries(workrave-core-next-integration-test PRIVATE workrave-libs-utils)
  target_link_libraries(workrave-core-next-integration-test PRIVATE Boost::unit_test_framework)
  target_link_libraries(workrave-core-next-integration-test PRIVATE ${EXTRA_LIBRARIES})

  target_include_directories(workrave-core-next-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/corenext/src)
  target_include_directories(workrave-core-next-integration-test PRIVATE ${CMAKE_SOURCE_DIR}/libs/stats/src)

  if (HAVE_APP_QT)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt6DBus_LIBRARIES})
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${Qt6Widgets_LIBRARIES})
  elseif (HAVE_APP_GTK)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARIES})
    target_link_directories(workrave-core-next-integration-test PRIVATE ${GLIB_LIBRARY_DIRS})
  endif()

  if (PLATFORM_OS_UNIX)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${X11_X11_LIB} ${X11_XTest_LIB} ${X11_Xscreensaver_LIB})
  endif()

  if (SSP_LIBRARY)
    target_link_libraries(workrave-core-next-integration-test PRIVATE ${SSP_LIBRARY})
    target_link_libraries(workrave-core-next-timer-test PRIVATE ${SSP_LIBRARY})
  endif()

  workrave_add_test(workrave-core-next-integration-test)
  workrave_add_test(workrave-core-next-timer-test)

  if (HAVE_GRPC AND HAVE_CORE_NEXT)
    # Including all three production-generated headers in one translation
    # unit also proves their service-prefixed protobuf types do not collide.
    add_executable(workrave-core-next-rpc-service-names-test RpcServiceNamesTest.cc)
    set_target_properties(workrave-core-next-rpc-service-names-test PROPERTIES USE_STUBS ON)
    target_include_directories(workrave-core-next-rpc-service-names-test PRIVATE
      ${CMAKE_BINARY_DIR}/libs/corenext/src
      ${CMAKE_BINARY_DIR}/libs/config/src)
    target_link_libraries(workrave-core-next-rpc-service-names-test PRIVATE
      GTest::gtest_main
      workrave-libs-core-next
      workrave-libs-config-rpc
      workrave-libs-core-next-rpc
      gRPC::grpc++
      protobuf::libprotobuf
      ${EXTRA_LIBRARIES})
    add_dependencies(workrave-core-next-rpc-service-names-test
      RpcCore_rpc_source_target
      RpcBreak_rpc_source_target
      RpcConfig_rpc_source_target)
    workrave_add_test(workrave-core-next-rpc-service-names-test)
  endif()

endif()

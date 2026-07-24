if (HAVE_RPC AND HAVE_TESTS)

  rpc_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/RpcTestServer.hh ${CMAKE_CURRENT_BINARY_DIR} RpcTest)
  rpc_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/RpcKeyedServer.hh ${CMAKE_CURRENT_BINARY_DIR} RpcKeyed)

  add_executable(workrave-libs-rpc-test
    RpcTestServer.cc
    RpcKeyedServer.cc
    Test.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcTestServiceImpl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcTest.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcTest.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcKeyedServiceImpl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcKeyed.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcKeyed.grpc.pb.cc
    )

  target_include_directories(workrave-libs-rpc-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  target_link_libraries(workrave-libs-rpc-test PRIVATE
    workrave-libs-rpc
    gRPC::grpc++
    protobuf::libprotobuf
    GTest::gtest_main
    ${EXTRA_LIBRARIES}
    )

  workrave_add_test(workrave-libs-rpc-test)

  if (HAVE_DBUS)
    # TEMPORARY: compares clang-rpc-gen's DBus output against the existing,
    # untouched libs/dbus pipeline's introspection XML — see
    # DBusComparisonTest.cc's top comment for why this reads the generated
    # .cc sources as plain text (get_interface_introspect() is a compile-time
    # constant, and compiling/linking both generators' glue into one binary
    # is a guaranteed duplicate-symbol error). No Qt/DBus/gRPC link
    # dependency at all — just GTest plus a couple of file paths.
    add_executable(workrave-libs-dbus-comparison-test DBusComparisonTest.cc)
    target_link_libraries(workrave-libs-dbus-comparison-test PRIVATE
      GTest::gtest_main
      ${EXTRA_LIBRARIES}
      )
    # These custom targets (defined by dbus_generate_source()/
    # rpc_generate_source() in libs/corenext/src/CMakeLists.txt and
    # libs/config/src/CMakeLists.txt) are the ones that actually produce the
    # .cc files named below — depend on them directly so the paths are
    # guaranteed to exist and be current, without needing to link the
    # libraries built from them.
    add_dependencies(workrave-libs-dbus-comparison-test
      DBusWorkraveNext_dbus_source_target
      RpcCore_rpc_source_target
      RpcBreak_rpc_source_target
      RpcConfig_rpc_source_target
      )
    target_compile_definitions(workrave-libs-dbus-comparison-test PRIVATE
      OLD_DBUS_WORKRAVE_NEXT_CC="${CMAKE_BINARY_DIR}/libs/corenext/src/DBusWorkraveNext.cc"
      NEW_CORE_DBUS_CC="${CMAKE_BINARY_DIR}/libs/corenext/src/RpcCoreDBus.cc"
      NEW_BREAK_DBUS_CC="${CMAKE_BINARY_DIR}/libs/corenext/src/RpcBreakDBus.cc"
      NEW_CONFIG_DBUS_CC="${CMAKE_BINARY_DIR}/libs/config/src/RpcConfigDBus.cc"
      )

    workrave_add_test(workrave-libs-dbus-comparison-test)
  endif()

endif()

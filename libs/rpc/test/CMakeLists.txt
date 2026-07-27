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

endif()

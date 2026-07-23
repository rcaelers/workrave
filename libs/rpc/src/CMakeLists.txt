add_library(workrave-libs-rpc STATIC
  RpcServer.cc
  Duration.cc
)

target_link_libraries(workrave-libs-rpc PUBLIC gRPC::grpc++ gRPC::grpc++_reflection)
target_include_directories(workrave-libs-rpc PUBLIC ${CMAKE_SOURCE_DIR}/libs/rpc/include)

# Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

include_guard(GLOBAL)

set(WORKRAVE_GRPC_VERSION "1.83.0")
set(WORKRAVE_GRPC_SOURCE_DIR "" CACHE PATH "Use an existing gRPC source tree instead of downloading it")
mark_as_advanced(WORKRAVE_GRPC_SOURCE_DIR)

function(workrave_configure_grpc)
  if(NOT WIN32 OR NOT MINGW)
    find_package(gRPC CONFIG REQUIRED)
    return()
  endif()

  include(FetchContent)

  set(_grpc_patch_command
    ${CMAKE_COMMAND}
    -DGRPC_SOURCE_DIR=<SOURCE_DIR>
    -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../patches/GrpcMinGWUnixSocket.cmake)

  if(WORKRAVE_GRPC_SOURCE_DIR)
    FetchContent_Declare(workrave_grpc
      SOURCE_DIR "${WORKRAVE_GRPC_SOURCE_DIR}"
      PATCH_COMMAND ${_grpc_patch_command})
  else()
    # Pin the official upstream release digest so configuration is
    # reproducible and fails closed if the downloaded source changes.
    FetchContent_Declare(workrave_grpc
      URL "https://github.com/grpc/grpc/archive/refs/tags/v1.83.0.tar.gz"
      URL_HASH "SHA256=90d453393a9d41215df546103b10b33b9566df79cdf6f49dc67f6c4d044d090d"
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
      PATCH_COMMAND ${_grpc_patch_command})
  endif()

  # Build only gRPC itself. Its dependencies continue to come from MSYS2, so
  # there is one Abseil/Protobuf ABI in the process and no duplicated bundled
  # dependency stack. Static gRPC also avoids MinGW's cross-DLL TLS and export
  # issues, which are the reason the MSYS2 shared package carries extra patches.
  set(BUILD_SHARED_LIBS OFF)
  set(gRPC_INSTALL OFF)
  set(gRPC_BUILD_TESTS OFF)
  set(gRPC_BUILD_CODEGEN ON)
  set(gRPC_DOWNLOAD_ARCHIVES OFF)
  set(gRPC_BUILD_GRPCPP_OTEL_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_CPP_PLUGIN ON)
  set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF)
  set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF)
  set(gRPC_ABSL_PROVIDER package)
  set(gRPC_CARES_PROVIDER package)
  set(gRPC_PROTOBUF_PROVIDER package)
  set(gRPC_RE2_PROVIDER package)
  set(gRPC_SSL_PROVIDER package)
  set(gRPC_ZLIB_PROVIDER package)

  # MSYS2 applies this definition while compiling gRPC because declarations in
  # strsafe.h otherwise conflict with libc++ headers. Keep it scoped to the
  # private dependency subtree.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSTRSAFE_NO_DEPRECATE")

  message(STATUS
    "gRPC: building private static ${WORKRAVE_GRPC_VERSION} with MinGW AF_UNIX support; "
    "using MSYS2 package dependencies")
  FetchContent_MakeAvailable(workrave_grpc)

  # gRPC's build-tree targets are intentionally unnamespaced. Workrave also
  # supports installed gRPC packages, whose exported targets are namespaced, so
  # provide the same interface in the private-build case.
  if(NOT TARGET gRPC::grpc++)
    add_library(gRPC::grpc++ ALIAS grpc++)
  endif()
  if(NOT TARGET gRPC::grpc++_reflection)
    add_library(gRPC::grpc++_reflection ALIAS grpc++_reflection)
  endif()
  if(NOT TARGET gRPC::grpc_cpp_plugin)
    add_executable(gRPC::grpc_cpp_plugin ALIAS grpc_cpp_plugin)
  endif()
endfunction()

# Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

include_guard(GLOBAL)

function(workrave_configure_grpc)
  # The Workrave MinGW package imports the normal upstream gRPC targets and
  # advertises its AF_UNIX patch. Other platforms use their regular gRPC
  # package directly.
  if(WIN32 AND MINGW)
    find_package(WorkraveGrpc CONFIG QUIET)
  endif()
  if(NOT TARGET gRPC::grpc++)
    find_package(gRPC CONFIG REQUIRED)
  endif()

  if(NOT WIN32 OR NOT MINGW)
    set(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS ON PARENT_SCOPE)
    return()
  endif()

  # GRPC_HAVE_UNIX_SOCKET is private to gRPC and is not installed as part of
  # its public API. During a native build, verify the linked library itself by
  # starting a temporary Unix-domain-socket server. This catches both missing
  # Windows AF_UNIX support and an unpatched gRPC binary.
  if(NOT CMAKE_CROSSCOMPILING)
    include(CheckCXXSourceRuns)
    set(_workrave_saved_required_libraries "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_LIBRARIES gRPC::grpc++)
    unset(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS CACHE)
    check_cxx_source_runs([=[
      #include <chrono>
      #include <filesystem>
      #include <string>

      #include <grpcpp/generic/async_generic_service.h>
      #include <grpcpp/grpcpp.h>

      int main()
      {
        const auto unique_suffix = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto socket_path = std::filesystem::temp_directory_path()
                                 / ("workrave-grpc-probe-" + std::to_string(unique_suffix) + ".sock");
        std::error_code ignored;
        std::filesystem::remove(socket_path, ignored);

        grpc::AsyncGenericService service;
        grpc::ServerBuilder builder;
        builder.RegisterAsyncGenericService(&service);
        auto completion_queue = builder.AddCompletionQueue();
        builder.AddListeningPort("unix:" + socket_path.string(), grpc::InsecureServerCredentials());
        auto server = builder.BuildAndStart();
        if (!server)
          {
            std::filesystem::remove(socket_path, ignored);
            return 1;
          }

        server->Shutdown();
        completion_queue->Shutdown();
        void *tag = nullptr;
        bool ok = false;
        while (completion_queue->Next(&tag, &ok))
          {
          }
        server.reset();
        std::filesystem::remove(socket_path, ignored);
        return 0;
      }
    ]=] WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS)
    set(CMAKE_REQUIRED_LIBRARIES "${_workrave_saved_required_libraries}")
    unset(_workrave_saved_required_libraries)

    if(NOT WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS)
      message(WARNING
        "The installed gRPC library cannot listen on Windows Unix domain sockets; "
        "Workrave will support TCP/IP gRPC only")
      set(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS OFF PARENT_SCOPE)
    else()
      message(STATUS "gRPC: verified Windows Unix domain socket support")
      set(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS ON PARENT_SCOPE)
    endif()
    return()
  endif()

  # A cross-compiled executable cannot be run during configuration. Trust the
  # capability marker when available, otherwise retain TCP/IP support only.
  if(NOT WorkraveGrpc_FOUND OR NOT WORKRAVE_GRPC_MINGW_AF_UNIX)
    message(WARNING
      "Cannot verify Windows Unix domain socket support while cross-compiling; "
      "Workrave will support TCP/IP gRPC only")
    set(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS OFF PARENT_SCOPE)
  else()
    message(STATUS "gRPC: package declares Windows Unix domain socket support")
    set(WORKRAVE_GRPC_SUPPORTS_UNIX_SOCKETS ON PARENT_SCOPE)
  endif()
endfunction()

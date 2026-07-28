# Enables the Windows AF_UNIX implementation already present in gRPC when the
# library is compiled with MinGW. Upstream currently disables the implementation
# solely through this preprocessor guard even though current MinGW-w64 provides
# the required afunix.h API.

if(NOT DEFINED GRPC_SOURCE_DIR)
  message(FATAL_ERROR "GRPC_SOURCE_DIR was not provided")
endif()

set(_grpc_port_header "${GRPC_SOURCE_DIR}/src/core/lib/iomgr/port.h")
if(NOT EXISTS "${_grpc_port_header}")
  message(FATAL_ERROR "gRPC platform header not found: ${_grpc_port_header}")
endif()

file(READ "${_grpc_port_header}" _grpc_port_contents)
set(_grpc_mingw_guard
  "#define GRPC_WINSOCK_SOCKET 1\n#ifndef __MINGW32__\n#define GRPC_HAVE_UNIX_SOCKET 1\n#endif  // __MINGW32__")
set(_grpc_unix_socket_enabled
  "#define GRPC_WINSOCK_SOCKET 1\n#define GRPC_HAVE_UNIX_SOCKET 1")

string(FIND "${_grpc_port_contents}" "${_grpc_mingw_guard}" _grpc_guard_position)
if(NOT _grpc_guard_position EQUAL -1)
  string(REPLACE
    "${_grpc_mingw_guard}"
    "${_grpc_unix_socket_enabled}"
    _grpc_port_contents
    "${_grpc_port_contents}")
  file(WRITE "${_grpc_port_header}" "${_grpc_port_contents}")
  message(STATUS "Enabled gRPC Windows AF_UNIX support for MinGW")
else()
  string(FIND "${_grpc_port_contents}" "${_grpc_unix_socket_enabled}" _grpc_enabled_position)
  if(_grpc_enabled_position EQUAL -1)
    message(FATAL_ERROR
      "The gRPC MinGW AF_UNIX guard no longer matches the audited source. "
      "Review ${_grpc_port_header} before updating the private dependency.")
  endif()
  message(STATUS "gRPC Windows AF_UNIX support was already enabled")
endif()

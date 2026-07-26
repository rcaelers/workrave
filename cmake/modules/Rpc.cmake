set(RPC_TOOL_DIR ${CMAKE_SOURCE_DIR}/libs/rpc/tool)
set(RPC_TOOL_BIN ${RPC_TOOL_DIR}/target/release/clang-rpc-gen${CMAKE_EXECUTABLE_SUFFIX})

find_program(CARGO "cargo")
if (NOT CARGO)
  message(FATAL_ERROR "Could not find cargo. Please install rust and cargo (see https://rustup.rs) to build clang-rpc-gen.")
endif()

file(GLOB_RECURSE RPC_TOOL_SOURCES CONFIGURE_DEPENDS
  ${RPC_TOOL_DIR}/src/*.rs
  ${RPC_TOOL_DIR}/src/*.jinja
  )

add_custom_command(
  OUTPUT ${RPC_TOOL_BIN}
  COMMAND ${CARGO} build --release --locked
  WORKING_DIRECTORY ${RPC_TOOL_DIR}
  DEPENDS ${RPC_TOOL_SOURCES} ${RPC_TOOL_DIR}/Cargo.toml
  COMMENT "Building clang-rpc-gen (Rust)"
  )

add_custom_target(clang_rpc_gen_tool ALL DEPENDS ${RPC_TOOL_BIN})

# rpc_generate_source(HEADER DIRECTORY NAME
#                      [ANCHOR_SOURCE <path>] [PROTO_PACKAGE <pkg>] [HEADER_INCLUDE <literal>]
#                      [ADAPTER_NAMESPACE <cxx-ns>] [ANNOTATIONS <path>] [DBUS])
#
# Runs clang-rpc-gen against an @rpc-annotated HEADER to produce
# DIRECTORY/NAME.proto + DIRECTORY/NAME ServiceImpl.hh/.cc, then runs the
# real protoc + grpc_cpp_plugin against that .proto to produce the actual
# message/service C++ classes (DIRECTORY/NAME.pb.h/.cc,
# DIRECTORY/NAME.grpc.pb.h/.cc).
#
# ANCHOR_SOURCE is a .cc that #includes HEADER, used to resolve real compiler
# flags via the top-level compile_commands.json (headers are never keys in a
# compile-commands database themselves). Defaults to HEADER's own directory
# with the same base name (Foo.hh -> Foo.cc); pass ANCHOR_SOURCE explicitly
# for headers with no same-directory .cc of their own (e.g. a pure-virtual
# interface header — point it at any .cc in the same library that includes
# the header, transitively or directly).
#
# HEADER_INCLUDE is the literal text used in the generated adapter's
# #include "..." of HEADER. Defaults to HEADER's bare file name, which only
# resolves for targets that happen to have HEADER's own directory on their
# include path. Pass e.g. HEADER_INCLUDE "config/IConfigurator.hh" for a
# header that's only reachable via a library's PUBLIC include root (so other
# targets linking against that library, not just the library itself, can
# compile the generated adapter too).
#
# ADAPTER_NAMESPACE wraps the generated ServiceImpl class in the specified
# C++ namespace (for example "workrave::core::rpc"). It deliberately does
# not affect the protobuf package, wire endpoint, or protoc-generated C++
# message/service namespaces; those continue to follow PROTO_PACKAGE.
#
# ANNOTATIONS points at a file supplying `@rpc` tags by fully-qualified name
# (see libs/rpc/tool/src/external_annotations.rs for the format) — for a
# HEADER that can't carry annotation comments of its own, e.g. third-party or
# generated code. Merges with (doesn't replace) whatever real comments HEADER
# already has.
#
# DBUS additionally requests a DBus binding (DIRECTORY/NAME DBus.hh/.cc, see
# libs/rpc/tool/src/dbus_gen.rs) alongside the gRPC output — HEADER's
# interface must carry @rpc.dbus(interface="..."). No-op if HAVE_DBUS isn't
# set, since the generated code #includes libs/dbus's runtime headers.
# Deliberately just generates the files: it's the caller's job to decide
# what (if anything) links them — see libs/corenext/src/CMakeLists.txt for
# why this exists as an unlinked-by-default, comparison-only opt-in rather
# than something wired into the real app.
macro(rpc_generate_source HEADER DIRECTORY NAME)
  if (HAVE_RPC)
    cmake_parse_arguments(_rpc "DBUS" "ANCHOR_SOURCE;PROTO_PACKAGE;HEADER_INCLUDE;ADAPTER_NAMESPACE;ANNOTATIONS" "" ${ARGN})

    if (_rpc_PROTO_PACKAGE)
      set(_rpc_proto_package ${_rpc_PROTO_PACKAGE})
    else()
      set(_rpc_proto_package "workrave")
    endif()

    if (_rpc_ANCHOR_SOURCE)
      set(_rpc_anchor_source ${_rpc_ANCHOR_SOURCE})
    else()
      get_filename_component(_rpc_header_dir ${HEADER} DIRECTORY)
      get_filename_component(_rpc_header_name_we ${HEADER} NAME_WE)
      set(_rpc_anchor_source ${_rpc_header_dir}/${_rpc_header_name_we}.cc)
    endif()

    set(_rpc_proto_output ${DIRECTORY}/${NAME}.proto)
    set(_rpc_adapter_hh ${DIRECTORY}/${NAME}ServiceImpl.hh)
    set(_rpc_adapter_cc ${DIRECTORY}/${NAME}ServiceImpl.cc)

    set(_rpc_header_include_args "")
    if (_rpc_HEADER_INCLUDE)
      set(_rpc_header_include_args --header-include ${_rpc_HEADER_INCLUDE})
    endif()

    set(_rpc_adapter_namespace_args "")
    if (_rpc_ADAPTER_NAMESPACE)
      set(_rpc_adapter_namespace_args --adapter-namespace ${_rpc_ADAPTER_NAMESPACE})
    endif()

    set(_rpc_annotations_args "")
    set(_rpc_annotations_depends "")
    if (_rpc_ANNOTATIONS)
      set(_rpc_annotations_args --annotations ${_rpc_ANNOTATIONS})
      set(_rpc_annotations_depends ${_rpc_ANNOTATIONS})
    endif()

    set(_rpc_dbus_args "")
    set(_rpc_dbus_outputs "")
    if (_rpc_DBUS AND HAVE_DBUS)
      set(_rpc_dbus_hh ${DIRECTORY}/${NAME}DBus.hh)
      set(_rpc_dbus_cc ${DIRECTORY}/${NAME}DBus.cc)
      set(_rpc_dbus_args --out-dbus-hh ${_rpc_dbus_hh} --out-dbus-cc ${_rpc_dbus_cc})
      set(_rpc_dbus_outputs ${_rpc_dbus_hh} ${_rpc_dbus_cc})
    endif()

    add_custom_command(
      OUTPUT ${_rpc_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_dbus_outputs}
      COMMAND ${RPC_TOOL_BIN}
              --header ${HEADER}
              --anchor-source ${_rpc_anchor_source}
              --compile-commands ${CMAKE_BINARY_DIR}/compile_commands.json
              --out-proto ${_rpc_proto_output}
              --out-adapter-hh ${_rpc_adapter_hh}
              --out-adapter-cc ${_rpc_adapter_cc}
              --proto-package ${_rpc_proto_package}
              ${_rpc_header_include_args}
              ${_rpc_adapter_namespace_args}
              ${_rpc_annotations_args}
              ${_rpc_dbus_args}
      DEPENDS ${HEADER} ${_rpc_anchor_source} ${RPC_TOOL_BIN} ${_rpc_annotations_depends}
      COMMENT "Generating gRPC bindings for ${HEADER}"
      )

    set(_rpc_pb_hh ${DIRECTORY}/${NAME}.pb.h)
    set(_rpc_pb_cc ${DIRECTORY}/${NAME}.pb.cc)
    set(_rpc_grpc_pb_hh ${DIRECTORY}/${NAME}.grpc.pb.h)
    set(_rpc_grpc_pb_cc ${DIRECTORY}/${NAME}.grpc.pb.cc)

    add_custom_command(
      OUTPUT ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc}
      COMMAND protobuf::protoc
              --cpp_out=${DIRECTORY}
              --grpc_out=${DIRECTORY}
              --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
              -I ${DIRECTORY}
              ${_rpc_proto_output}
      DEPENDS ${_rpc_proto_output} protobuf::protoc gRPC::grpc_cpp_plugin
      COMMENT "Running protoc/grpc_cpp_plugin for ${NAME}.proto"
      )

    add_custom_target(
      ${NAME}_rpc_source_target ALL
      DEPENDS ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc} ${_rpc_dbus_outputs}
      )

    set_source_files_properties(
      ${_rpc_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc} ${_rpc_dbus_outputs}
      PROPERTIES GENERATED TRUE
      )
  endif()
endmacro()

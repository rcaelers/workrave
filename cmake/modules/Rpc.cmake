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

function(rpc_generate_parse_context TARGET_NAME DIRECTORY NAME OUTPUT_VAR)
  if (NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "RPC generation target '${TARGET_NAME}' does not exist")
  endif()

  set(_rpc_context ${DIRECTORY}/${NAME}-$<CONFIG>.rpc-parse-context)
  set(_rpc_standard
    "$<IF:$<BOOL:$<TARGET_PROPERTY:${TARGET_NAME},CXX_STANDARD>>,$<TARGET_PROPERTY:${TARGET_NAME},CXX_STANDARD>,${CMAKE_CXX_STANDARD}>")
  set(_rpc_dialect
    "$<IF:$<STREQUAL:$<TARGET_PROPERTY:${TARGET_NAME},CXX_EXTENSIONS>,OFF>,c++,gnu++>")
  set(_rpc_includes
    "$<REMOVE_DUPLICATES:$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>>")
  set(_rpc_definitions
    "$<REMOVE_DUPLICATES:$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>>")

  string(CONCAT _rpc_context_content
    "standard=${_rpc_dialect}${_rpc_standard}\n"
    "$<$<BOOL:${_rpc_includes}>:include=$<JOIN:${_rpc_includes},\ninclude=>>\n"
    "$<$<BOOL:${_rpc_definitions}>:define=$<JOIN:${_rpc_definitions},\ndefine=>>\n")

  if (CMAKE_SYSROOT)
    string(APPEND _rpc_context_content "sysroot=${CMAKE_SYSROOT}\n")
  endif()
  if (CMAKE_CXX_COMPILER_TARGET)
    string(APPEND _rpc_context_content "target=${CMAKE_CXX_COMPILER_TARGET}\n")
  endif()

  # file(GENERATE) evaluates the target properties after all direct and
  # transitive usage requirements have been collected. The generator thus
  # receives semantic parse data, never an arbitrary compiler command.
  file(GENERATE
    OUTPUT ${_rpc_context}
    CONTENT "${_rpc_context_content}"
    TARGET ${TARGET_NAME})

  set(${OUTPUT_VAR} ${_rpc_context} PARENT_SCOPE)
endfunction()

# rpc_generate_source(HEADER DIRECTORY NAME
#                      TARGET <cmake-target>
#                      [PROTO_PACKAGE <pkg>] [HEADER_INCLUDE <literal>]
#                      [PROTO_TYPES_PACKAGE <pkg>] [ADAPTER_NAMESPACE <cxx-ns>]
#                      [GRPC_SERVICES_NAMESPACE <cxx-name>]
#                      [ANNOTATIONS <path>] [DBUS])
#
# Runs clang-rpc-gen against an @rpc-annotated HEADER to produce
# DIRECTORY/NAME.proto + DIRECTORY/NAME ServiceImpl.hh/.cc, then runs the
# real protoc + grpc_cpp_plugin against that .proto to produce the actual
# message/service C++ classes (DIRECTORY/NAME.pb.h/.cc,
# DIRECTORY/NAME.grpc.pb.h/.cc).
#
# TARGET is the CMake target whose semantic compilation context should be used
# to parse HEADER. CMake writes the target's effective C++ standard, include
# directories, compile definitions, target, and sysroot to a typed context
# file. Build-only compiler flags are deliberately not passed to libclang.
#
# HEADER_INCLUDE is the literal text used in the generated adapter's
# #include "..." of HEADER. Defaults to HEADER's bare file name, which only
# resolves for targets that happen to have HEADER's own directory on their
# include path. Pass e.g. HEADER_INCLUDE "config/IConfigurator.hh" for a
# header that's only reachable via a library's PUBLIC include root (so other
# targets linking against that library, not just the library itself, can
# compile the generated adapter too).
#
# ADAPTER_NAMESPACE wraps the generated ServiceImpl and, when requested, DBus
# binding classes in the specified C++ namespace (for example
# "workrave::core::rpc"). It deliberately does not affect protobuf packages,
# DBus interface names, or other wire-visible names.
#
# PROTO_TYPES_PACKAGE moves all payload enums/messages into a second imported
# ${NAME}Types.proto schema. The service remains in PROTO_PACKAGE (and keeps
# that wire name), while the payload package controls their generated C++
# namespace and prevents independently generated services from colliding.
#
# GRPC_SERVICES_NAMESPACE is an unqualified C++ namespace appended to
# PROTO_PACKAGE for grpc_cpp_plugin's generated service/stub classes. It does
# not change the protobuf package or wire service name. For example,
# PROTO_PACKAGE workrave plus GRPC_SERVICES_NAMESPACE rpc produces the C++
# class workrave::rpc::<Service> while retaining workrave.<Service> on wire.
#
# ANNOTATIONS points at a file supplying `@rpc` tags by fully-qualified name
# (see libs/rpc/tool/src/external_annotations.rs for the format) — for a
# HEADER that can't carry annotation comments of its own, e.g. third-party or
# generated code. Merges with (doesn't replace) whatever real comments HEADER
# already has.
#
# DBUS additionally requests a DBus binding (DIRECTORY/NAME DBus.hh/.cc, see
# libs/rpc/tool/src/dbus_gen.rs) alongside the gRPC output — HEADER's
# interface must carry @rpc.dbus(interface="..."). No-op unless either DBus
# implementation is enabled, since the generated code includes libs/dbus's
# runtime headers.
# Deliberately just generates the files: the caller decides which target owns
# and links them. Use rpc_generate_dbus_source() when no gRPC output is needed.
macro(rpc_generate_source HEADER DIRECTORY NAME)
  if (HAVE_RPC)
    cmake_parse_arguments(_rpc "DBUS" "TARGET;PROTO_PACKAGE;PROTO_TYPES_PACKAGE;GRPC_SERVICES_NAMESPACE;HEADER_INCLUDE;ADAPTER_NAMESPACE;ANNOTATIONS" "" ${ARGN})

    if (NOT _rpc_TARGET)
      message(FATAL_ERROR "rpc_generate_source(${NAME}) requires TARGET <cmake-target>")
    endif()
    rpc_generate_parse_context(${_rpc_TARGET} ${DIRECTORY} ${NAME} _rpc_parse_context)

    if (_rpc_PROTO_PACKAGE)
      set(_rpc_proto_package ${_rpc_PROTO_PACKAGE})
    else()
      set(_rpc_proto_package "workrave")
    endif()

    set(_rpc_proto_output ${DIRECTORY}/${NAME}.proto)
    set(_rpc_types_proto_output "")
    set(_rpc_types_proto_args "")
    if (_rpc_PROTO_TYPES_PACKAGE)
      set(_rpc_types_proto_output ${DIRECTORY}/${NAME}Types.proto)
      set(_rpc_types_proto_args
        --out-types-proto ${_rpc_types_proto_output}
        --proto-types-package ${_rpc_PROTO_TYPES_PACKAGE})
    endif()
    set(_rpc_adapter_hh ${DIRECTORY}/${NAME}ServiceImpl.hh)
    set(_rpc_adapter_cc ${DIRECTORY}/${NAME}ServiceImpl.cc)

    set(_rpc_grpc_services_namespace_args "")
    set(_rpc_grpc_out ${DIRECTORY})
    if (_rpc_GRPC_SERVICES_NAMESPACE)
      set(_rpc_grpc_services_namespace_args
        --grpc-services-namespace ${_rpc_GRPC_SERVICES_NAMESPACE})
      set(_rpc_grpc_out "services_namespace=${_rpc_GRPC_SERVICES_NAMESPACE}:${DIRECTORY}")
    endif()

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
    if (_rpc_DBUS AND (HAVE_DBUS OR HAVE_RPC_DBUS))
      set(_rpc_dbus_hh ${DIRECTORY}/${NAME}DBus.hh)
      set(_rpc_dbus_cc ${DIRECTORY}/${NAME}DBus.cc)
      set(_rpc_dbus_args --out-dbus-hh ${_rpc_dbus_hh} --out-dbus-cc ${_rpc_dbus_cc})
      set(_rpc_dbus_outputs ${_rpc_dbus_hh} ${_rpc_dbus_cc})
    endif()

    add_custom_command(
      OUTPUT ${_rpc_proto_output} ${_rpc_types_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_dbus_outputs}
      COMMAND ${RPC_TOOL_BIN}
              --header ${HEADER}
              --parse-context ${_rpc_parse_context}
              --out-proto ${_rpc_proto_output}
              --out-adapter-hh ${_rpc_adapter_hh}
              --out-adapter-cc ${_rpc_adapter_cc}
              --proto-package ${_rpc_proto_package}
              ${_rpc_types_proto_args}
              ${_rpc_grpc_services_namespace_args}
              ${_rpc_header_include_args}
              ${_rpc_adapter_namespace_args}
              ${_rpc_annotations_args}
              ${_rpc_dbus_args}
      DEPENDS ${HEADER} ${_rpc_parse_context} ${RPC_TOOL_BIN} ${_rpc_annotations_depends}
      COMMENT "Generating gRPC bindings for ${HEADER}"
      )

    set(_rpc_pb_hh ${DIRECTORY}/${NAME}.pb.h)
    set(_rpc_pb_cc ${DIRECTORY}/${NAME}.pb.cc)
    set(_rpc_grpc_pb_hh ${DIRECTORY}/${NAME}.grpc.pb.h)
    set(_rpc_grpc_pb_cc ${DIRECTORY}/${NAME}.grpc.pb.cc)
    set(_rpc_types_pb_hh "")
    set(_rpc_types_pb_cc "")

    if (_rpc_types_proto_output)
      set(_rpc_types_pb_hh ${DIRECTORY}/${NAME}Types.pb.h)
      set(_rpc_types_pb_cc ${DIRECTORY}/${NAME}Types.pb.cc)
      add_custom_command(
        OUTPUT ${_rpc_types_pb_hh} ${_rpc_types_pb_cc}
        COMMAND protobuf::protoc
                --cpp_out=${DIRECTORY}
                -I ${DIRECTORY}
                ${_rpc_types_proto_output}
        DEPENDS ${_rpc_types_proto_output} protobuf::protoc
        COMMENT "Running protoc for ${NAME}Types.proto"
        )
    endif()

    add_custom_command(
      OUTPUT ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc}
      COMMAND protobuf::protoc
              --cpp_out=${DIRECTORY}
              --grpc_out=${_rpc_grpc_out}
              --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
              -I ${DIRECTORY}
              ${_rpc_proto_output}
      DEPENDS ${_rpc_proto_output} ${_rpc_types_proto_output} ${_rpc_types_pb_hh} protobuf::protoc gRPC::grpc_cpp_plugin
      COMMENT "Running protoc/grpc_cpp_plugin for ${NAME}.proto"
      )

    add_custom_target(
      ${NAME}_rpc_source_target ALL
      DEPENDS ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc} ${_rpc_types_pb_hh} ${_rpc_types_pb_cc} ${_rpc_dbus_outputs}
      )

    set_source_files_properties(
      ${_rpc_proto_output} ${_rpc_types_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_pb_hh} ${_rpc_pb_cc} ${_rpc_grpc_pb_hh} ${_rpc_grpc_pb_cc} ${_rpc_types_pb_hh} ${_rpc_types_pb_cc} ${_rpc_dbus_outputs}
      PROPERTIES GENERATED TRUE
      )
  endif()
endmacro()

# rpc_generate_dbus_source(HEADER DIRECTORY NAME TARGET <cmake-target>
#                          [HEADER_INCLUDE <literal>]
#                          [ADAPTER_NAMESPACE <cxx-ns>] [ANNOTATIONS <path>])
#
# DBus-only entry point used when the clang-generated implementation is
# selected. clang-rpc-gen currently constructs its shared gRPC+DBus semantic
# model in one invocation, so the three gRPC text artifacts are generated as
# unused implementation details.
macro(rpc_generate_dbus_source HEADER DIRECTORY NAME)
  if (HAVE_RPC_DBUS)
    cmake_parse_arguments(_rpc_dbus "" "TARGET;HEADER_INCLUDE;ADAPTER_NAMESPACE;ANNOTATIONS" "" ${ARGN})

    if (NOT _rpc_dbus_TARGET)
      message(FATAL_ERROR "rpc_generate_dbus_source(${NAME}) requires TARGET <cmake-target>")
    endif()
    rpc_generate_parse_context(${_rpc_dbus_TARGET} ${DIRECTORY} ${NAME} _rpc_dbus_parse_context)

    set(_rpc_dbus_proto ${DIRECTORY}/${NAME}.unused.proto)
    set(_rpc_dbus_adapter_hh ${DIRECTORY}/${NAME}ServiceImpl.unused.hh)
    set(_rpc_dbus_adapter_cc ${DIRECTORY}/${NAME}ServiceImpl.unused.cc)
    set(_rpc_dbus_hh ${DIRECTORY}/${NAME}DBus.hh)
    set(_rpc_dbus_cc ${DIRECTORY}/${NAME}DBus.cc)

    set(_rpc_dbus_header_include_args "")
    if (_rpc_dbus_HEADER_INCLUDE)
      set(_rpc_dbus_header_include_args --header-include ${_rpc_dbus_HEADER_INCLUDE})
    endif()

    set(_rpc_dbus_namespace_args "")
    if (_rpc_dbus_ADAPTER_NAMESPACE)
      set(_rpc_dbus_namespace_args --adapter-namespace ${_rpc_dbus_ADAPTER_NAMESPACE})
    endif()

    set(_rpc_dbus_annotations_args "")
    set(_rpc_dbus_annotations_depends "")
    if (_rpc_dbus_ANNOTATIONS)
      set(_rpc_dbus_annotations_args --annotations ${_rpc_dbus_ANNOTATIONS})
      set(_rpc_dbus_annotations_depends ${_rpc_dbus_ANNOTATIONS})
    endif()

    add_custom_command(
      OUTPUT
        ${_rpc_dbus_proto}
        ${_rpc_dbus_adapter_hh}
        ${_rpc_dbus_adapter_cc}
        ${_rpc_dbus_hh}
        ${_rpc_dbus_cc}
      COMMAND ${RPC_TOOL_BIN}
              --header ${HEADER}
              --parse-context ${_rpc_dbus_parse_context}
              --out-proto ${_rpc_dbus_proto}
              --out-adapter-hh ${_rpc_dbus_adapter_hh}
              --out-adapter-cc ${_rpc_dbus_adapter_cc}
              --proto-package workrave.dbus.compat
              ${_rpc_dbus_header_include_args}
              ${_rpc_dbus_namespace_args}
              ${_rpc_dbus_annotations_args}
              --out-dbus-hh ${_rpc_dbus_hh}
              --out-dbus-cc ${_rpc_dbus_cc}
      DEPENDS ${HEADER} ${_rpc_dbus_parse_context} ${RPC_TOOL_BIN} ${_rpc_dbus_annotations_depends}
      COMMENT "Generating DBus binding for ${HEADER}"
      )

    add_custom_target(
      ${NAME}_rpc_dbus_source_target ALL
      DEPENDS ${_rpc_dbus_hh} ${_rpc_dbus_cc}
      )

    set_source_files_properties(
      ${_rpc_dbus_proto}
      ${_rpc_dbus_adapter_hh}
      ${_rpc_dbus_adapter_cc}
      ${_rpc_dbus_hh}
      ${_rpc_dbus_cc}
      PROPERTIES GENERATED TRUE
      )
  endif()
endmacro()

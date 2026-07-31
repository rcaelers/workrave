set(RPC_TOOL_DIR ${CMAKE_SOURCE_DIR}/libs/rpc/tool)
set(RPC_TOOL_BIN ${RPC_TOOL_DIR}/target/release/clang-rpc-gen${CMAKE_EXECUTABLE_SUFFIX})

set(RPC_CODEGEN "AUTO" CACHE STRING "Build RPC bindings: AUTO, ON, or OFF (use checked-in gen/ files)")
set_property(CACHE RPC_CODEGEN PROPERTY STRINGS AUTO ON OFF)
find_program(CARGO "cargo")

# Locate libclang. Most Linux distros split LLVM into per-version packages;
# the unversioned "libclang.so" symlink that find_library() below can match
# only ships in the much less commonly installed libclang-<ver>-dev package.
# The runtime package most people actually have (e.g. libclang1-21) installs
# only a versioned soname such as /usr/lib/aarch64-linux-gnu/libclang-21.so.21
# in a multiarch directory, which find_library() cannot match by name. Fall
# back to globbing for those versioned libraries, picking the newest, the
# same way the Rust clang-sys crate (used by clang-rpc-gen) locates libclang
# at its own build time.
find_library(RPC_LIBCLANG_LIBRARY
  NAMES clang libclang
  HINTS "$ENV{LIBCLANG_PATH}" /opt/homebrew/opt/llvm/lib)
if (NOT RPC_LIBCLANG_LIBRARY)
  file(GLOB _rpc_libclang_candidates
    "$ENV{LIBCLANG_PATH}/libclang*.so*"
    "$ENV{LIBCLANG_PATH}/libclang*.dylib"
    /usr/lib/llvm-*/lib/libclang-*.so*
    /usr/lib/*/libclang-*.so*
    /usr/lib/libclang-*.so*
    /usr/lib64/libclang-*.so*
    /usr/local/lib/libclang-*.so*)
  # libclang-cpp.so* is Clang's separate C++ interface library; clang-sys
  # (and this glob's intent) need the C API library, plain libclang-<ver>.so*.
  list(FILTER _rpc_libclang_candidates EXCLUDE REGEX "libclang-cpp")
  if (_rpc_libclang_candidates)
    list(SORT _rpc_libclang_candidates COMPARE NATURAL ORDER DESCENDING)
    list(GET _rpc_libclang_candidates 0 RPC_LIBCLANG_LIBRARY)
  endif()
endif()
if (RPC_LIBCLANG_LIBRARY)
  get_filename_component(RPC_LIBCLANG_DIR "${RPC_LIBCLANG_LIBRARY}" DIRECTORY)
  message(STATUS "RPC: found libclang: ${RPC_LIBCLANG_LIBRARY}")
endif()

if (RPC_CODEGEN STREQUAL "AUTO")
  if (CARGO AND RPC_LIBCLANG_LIBRARY)
    set(RPC_CODEGEN_ENABLED ON)
  else()
    set(RPC_CODEGEN_ENABLED OFF)
  endif()
elseif (RPC_CODEGEN STREQUAL "ON")
  if (NOT CARGO)
    message(FATAL_ERROR "RPC_CODEGEN=ON requires Rust/Cargo (https://rustup.rs); use RPC_CODEGEN=OFF for checked-in bindings")
  endif()
  if (NOT RPC_LIBCLANG_LIBRARY)
    message(FATAL_ERROR "RPC_CODEGEN=ON requires libclang; set LIBCLANG_PATH or use RPC_CODEGEN=OFF")
  endif()
  set(RPC_CODEGEN_ENABLED ON)
elseif (RPC_CODEGEN STREQUAL "OFF")
  set(RPC_CODEGEN_ENABLED OFF)
else()
  message(FATAL_ERROR "RPC_CODEGEN must be AUTO, ON, or OFF")
endif()

if (RPC_CODEGEN_ENABLED)
  file(GLOB_RECURSE RPC_TOOL_SOURCES CONFIGURE_DEPENDS
    ${RPC_TOOL_DIR}/src/*.rs
    ${RPC_TOOL_DIR}/src/*.jinja)

  set(_rpc_cargo_env_command "")
  if (RPC_LIBCLANG_DIR AND NOT DEFINED ENV{LIBCLANG_PATH})
    set(_rpc_cargo_env_command ${CMAKE_COMMAND} -E env "LIBCLANG_PATH=${RPC_LIBCLANG_DIR}")
  endif()

  add_custom_command(
    OUTPUT ${RPC_TOOL_BIN}
    COMMAND ${_rpc_cargo_env_command} ${CARGO} build --release --locked
    WORKING_DIRECTORY ${RPC_TOOL_DIR}
    DEPENDS ${RPC_TOOL_SOURCES} ${RPC_TOOL_DIR}/Cargo.toml
    COMMENT "Building clang-rpc-gen (Rust)")

  add_custom_target(clang_rpc_gen_tool ALL DEPENDS ${RPC_TOOL_BIN})
  add_custom_target(rpc_refresh_pregenerated)
  add_dependencies(rpc_refresh_pregenerated clang_rpc_gen_tool)
  message(STATUS "RPC bindings: generating with Rust/libclang (RPC_CODEGEN=${RPC_CODEGEN})")
else()
  message(STATUS "RPC bindings: using checked-in gen/ artifacts (RPC_CODEGEN=${RPC_CODEGEN})")
endif()

function(rpc_verify_pregenerated HEADER PREGENERATED_DIR NAME)
  set(_hash_file "${PREGENERATED_DIR}/${NAME}.sha256")
  if (NOT EXISTS "${_hash_file}")
    message(FATAL_ERROR "Missing pre-generated RPC hash: ${_hash_file}")
  endif()
  file(READ "${_hash_file}" _expected_hash)
  string(STRIP "${_expected_hash}" _expected_hash)
  file(SHA256 "${HEADER}" _actual_hash)
  if (NOT _actual_hash STREQUAL _expected_hash)
    message(FATAL_ERROR
      "Pre-generated RPC binding ${NAME} is stale for ${HEADER}. "
      "Configure with RPC_CODEGEN=ON and build target rpc_refresh_pregenerated.")
  endif()
endfunction()

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

  # Target include properties do not contain the compiler's implicit search
  # directories. libclang does not necessarily use the same resource directory
  # as CMAKE_CXX_COMPILER (notably with MSYS2 Clang), so reproduce CMake's
  # detected implicit directories explicitly. This includes Clang's builtin
  # headers such as mm_malloc.h.
  foreach(_rpc_system_include IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
    string(APPEND _rpc_context_content "system-include=${_rpc_system_include}\n")
  endforeach()

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
#                      [HEADER_INCLUDE <literal>] [PROTO_TYPES_PACKAGE <pkg>]
#                      [ADAPTER_NAMESPACE <cxx-ns>]
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
# file, together with CMake's detected implicit compiler include directories.
# Build-only compiler flags are deliberately not passed to libclang.
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
# The public service wire name (both protobuf package and service identifier)
# comes from the header's fully-qualified `@rpc(service="package.Service")`
# annotation. PROTO_TYPES_PACKAGE moves all payload enums/messages into a
# second imported ${NAME}Types.proto schema. It controls both their wire
# package and generated C++ namespace.
#
# GRPC_SERVICES_NAMESPACE is an unqualified C++ namespace appended to the
# annotation's protobuf package for grpc_cpp_plugin's generated service/stub
# classes. It does not change the protobuf package or wire service name. For
# example, @rpc(service="workrave.CoreService") plus
# GRPC_SERVICES_NAMESPACE rpc produces workrave::rpc::CoreService while
# retaining workrave.CoreService on wire.
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
# implementation is enabled. The UI toolkit selects the standalone QtDBus or
# GDBus runtime in libs/rpc.
# Deliberately just generates the files: the caller decides which target owns
# and links them. Use rpc_generate_dbus_source() when no gRPC output is needed.
macro(rpc_generate_source HEADER DIRECTORY NAME)
  if (HAVE_GRPC)
    cmake_parse_arguments(_rpc "DBUS" "TARGET;PROTO_TYPES_PACKAGE;GRPC_SERVICES_NAMESPACE;HEADER_INCLUDE;ADAPTER_NAMESPACE;ANNOTATIONS;PREGENERATED_DIR" "" ${ARGN})

    if (NOT _rpc_TARGET)
      message(FATAL_ERROR "rpc_generate_source(${NAME}) requires TARGET <cmake-target>")
    endif()
    if (RPC_CODEGEN_ENABLED)
      add_dependencies(${_rpc_TARGET} clang_rpc_gen_tool)
      rpc_generate_parse_context(${_rpc_TARGET} ${DIRECTORY} ${NAME} _rpc_parse_context)
    elseif (NOT _rpc_PREGENERATED_DIR)
      message(FATAL_ERROR "rpc_generate_source(${NAME}) requires PREGENERATED_DIR when RPC_CODEGEN=OFF")
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
    if (_rpc_DBUS AND HAVE_DBUS)
      set(_rpc_dbus_hh ${DIRECTORY}/${NAME}DBus.hh)
      set(_rpc_dbus_cc ${DIRECTORY}/${NAME}DBus.cc)
      set(_rpc_dbus_args
        --dbus-backend ${RPC_DBUS_BACKEND}
        --out-dbus-hh ${_rpc_dbus_hh}
        --out-dbus-cc ${_rpc_dbus_cc})
      set(_rpc_dbus_outputs ${_rpc_dbus_hh} ${_rpc_dbus_cc})
    endif()

    if (RPC_CODEGEN_ENABLED)
      add_custom_command(
        OUTPUT ${_rpc_proto_output} ${_rpc_types_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc} ${_rpc_dbus_outputs}
        COMMAND ${RPC_TOOL_BIN}
              --header ${HEADER}
              --parse-context ${_rpc_parse_context}
              --out-proto ${_rpc_proto_output}
              --out-adapter-hh ${_rpc_adapter_hh}
              --out-adapter-cc ${_rpc_adapter_cc}
              ${_rpc_types_proto_args}
              ${_rpc_grpc_services_namespace_args}
              ${_rpc_header_include_args}
              ${_rpc_adapter_namespace_args}
              ${_rpc_annotations_args}
              ${_rpc_dbus_args}
      DEPENDS ${HEADER} ${_rpc_parse_context} ${RPC_TOOL_BIN} ${_rpc_annotations_depends}
        COMMENT "Generating gRPC bindings for ${HEADER}")

      if (_rpc_PREGENERATED_DIR)
        set(_rpc_pregen_types_args "")
        if (_rpc_PROTO_TYPES_PACKAGE)
          set(_rpc_pregen_types_args
            --out-types-proto ${_rpc_PREGENERATED_DIR}/${NAME}Types.proto
            --proto-types-package ${_rpc_PROTO_TYPES_PACKAGE})
        endif()
        add_custom_target(${NAME}_rpc_refresh_pregenerated
          COMMAND ${CMAKE_COMMAND} -E make_directory ${_rpc_PREGENERATED_DIR}
          COMMAND ${RPC_TOOL_BIN}
                  --header ${HEADER}
                  --parse-context ${_rpc_parse_context}
                  --out-proto ${_rpc_PREGENERATED_DIR}/${NAME}.proto
                  --out-adapter-hh ${_rpc_PREGENERATED_DIR}/${NAME}ServiceImpl.hh
                  --out-adapter-cc ${_rpc_PREGENERATED_DIR}/${NAME}ServiceImpl.cc
                  ${_rpc_pregen_types_args}
                  ${_rpc_grpc_services_namespace_args}
                  ${_rpc_header_include_args}
                  ${_rpc_adapter_namespace_args}
                  ${_rpc_annotations_args}
          COMMAND ${CMAKE_COMMAND} -DINPUT=${HEADER} -DOUTPUT=${_rpc_PREGENERATED_DIR}/${NAME}.sha256
                  -P ${CMAKE_SOURCE_DIR}/cmake/modules/WriteFileHash.cmake
          DEPENDS ${HEADER} ${_rpc_parse_context} ${RPC_TOOL_BIN} ${_rpc_annotations_depends}
          COMMENT "Refreshing checked-in RPC bindings for ${HEADER}")
        add_dependencies(rpc_refresh_pregenerated ${NAME}_rpc_refresh_pregenerated)
      endif()
    else()
      rpc_verify_pregenerated(${HEADER} ${_rpc_PREGENERATED_DIR} ${NAME})
      set(_rpc_pregen_files
        ${_rpc_PREGENERATED_DIR}/${NAME}.proto
        ${_rpc_PREGENERATED_DIR}/${NAME}ServiceImpl.hh
        ${_rpc_PREGENERATED_DIR}/${NAME}ServiceImpl.cc)
      if (_rpc_PROTO_TYPES_PACKAGE)
        list(APPEND _rpc_pregen_files ${_rpc_PREGENERATED_DIR}/${NAME}Types.proto)
      endif()
      foreach(_rpc_pregen_file IN LISTS _rpc_pregen_files)
        if (NOT EXISTS ${_rpc_pregen_file})
          message(FATAL_ERROR "Missing pre-generated RPC artifact: ${_rpc_pregen_file}")
        endif()
      endforeach()
      add_custom_command(
        OUTPUT ${_rpc_proto_output} ${_rpc_types_proto_output} ${_rpc_adapter_hh} ${_rpc_adapter_cc}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_rpc_pregen_files} ${DIRECTORY}
        DEPENDS ${_rpc_pregen_files}
        COMMENT "Using checked-in RPC bindings for ${HEADER}")
    endif()

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
  if (HAVE_DBUS)
    cmake_parse_arguments(_rpc_dbus "" "TARGET;HEADER_INCLUDE;ADAPTER_NAMESPACE;ANNOTATIONS;PREGENERATED_DIR" "" ${ARGN})

    if (NOT _rpc_dbus_TARGET)
      message(FATAL_ERROR "rpc_generate_dbus_source(${NAME}) requires TARGET <cmake-target>")
    endif()
    if (RPC_CODEGEN_ENABLED)
      add_dependencies(${_rpc_dbus_TARGET} clang_rpc_gen_tool)
      rpc_generate_parse_context(${_rpc_dbus_TARGET} ${DIRECTORY} ${NAME} _rpc_dbus_parse_context)
    elseif (NOT _rpc_dbus_PREGENERATED_DIR)
      message(FATAL_ERROR "rpc_generate_dbus_source(${NAME}) requires PREGENERATED_DIR when RPC_CODEGEN=OFF")
    endif()

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

    if (RPC_CODEGEN_ENABLED)
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
              ${_rpc_dbus_header_include_args}
              ${_rpc_dbus_namespace_args}
              ${_rpc_dbus_annotations_args}
              --dbus-backend ${RPC_DBUS_BACKEND}
              --out-dbus-hh ${_rpc_dbus_hh}
              --out-dbus-cc ${_rpc_dbus_cc}
      DEPENDS ${HEADER} ${_rpc_dbus_parse_context} ${RPC_TOOL_BIN} ${_rpc_dbus_annotations_depends}
        COMMENT "Generating DBus binding for ${HEADER}")

      if (_rpc_dbus_PREGENERATED_DIR)
        add_custom_target(${NAME}_rpc_dbus_refresh_pregenerated
          COMMAND ${CMAKE_COMMAND} -E make_directory ${_rpc_dbus_PREGENERATED_DIR}
          COMMAND ${RPC_TOOL_BIN}
                  --header ${HEADER}
                  --parse-context ${_rpc_dbus_parse_context}
                  --out-proto ${_rpc_dbus_proto}
                  --out-adapter-hh ${_rpc_dbus_adapter_hh}
                  --out-adapter-cc ${_rpc_dbus_adapter_cc}
                  ${_rpc_dbus_header_include_args}
                  ${_rpc_dbus_namespace_args}
                  ${_rpc_dbus_annotations_args}
                  --dbus-backend qt
                  --dbus-header-include ${NAME}DBus.hh
                  --out-dbus-hh ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBusQt.hh
                  --out-dbus-cc ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBusQt.cc
          COMMAND ${RPC_TOOL_BIN}
                  --header ${HEADER}
                  --parse-context ${_rpc_dbus_parse_context}
                  --out-proto ${_rpc_dbus_proto}
                  --out-adapter-hh ${_rpc_dbus_adapter_hh}
                  --out-adapter-cc ${_rpc_dbus_adapter_cc}
                  ${_rpc_dbus_header_include_args}
                  ${_rpc_dbus_namespace_args}
                  ${_rpc_dbus_annotations_args}
                  --dbus-backend gio
                  --dbus-header-include ${NAME}DBus.hh
                  --out-dbus-hh ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBusGio.hh
                  --out-dbus-cc ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBusGio.cc
          COMMAND ${CMAKE_COMMAND} -DINPUT=${HEADER} -DOUTPUT=${_rpc_dbus_PREGENERATED_DIR}/${NAME}.sha256
                  -P ${CMAKE_SOURCE_DIR}/cmake/modules/WriteFileHash.cmake
          DEPENDS ${HEADER} ${_rpc_dbus_parse_context} ${RPC_TOOL_BIN} ${_rpc_dbus_annotations_depends}
          COMMENT "Refreshing checked-in DBus binding for ${HEADER}")
        add_dependencies(rpc_refresh_pregenerated ${NAME}_rpc_dbus_refresh_pregenerated)
      endif()
    else()
      rpc_verify_pregenerated(${HEADER} ${_rpc_dbus_PREGENERATED_DIR} ${NAME})
      if (RPC_DBUS_BACKEND STREQUAL "qt")
        set(_rpc_dbus_pregen_suffix Qt)
      elseif (RPC_DBUS_BACKEND STREQUAL "gio")
        set(_rpc_dbus_pregen_suffix Gio)
      else()
        message(FATAL_ERROR "Unknown internal RPC DBus backend: ${RPC_DBUS_BACKEND}")
      endif()
      set(_rpc_dbus_pregen_hh ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBus${_rpc_dbus_pregen_suffix}.hh)
      set(_rpc_dbus_pregen_cc ${_rpc_dbus_PREGENERATED_DIR}/${NAME}DBus${_rpc_dbus_pregen_suffix}.cc)
      if (NOT EXISTS ${_rpc_dbus_pregen_hh} OR NOT EXISTS ${_rpc_dbus_pregen_cc})
        message(FATAL_ERROR "Missing pre-generated DBus artifacts for ${NAME} in ${_rpc_dbus_PREGENERATED_DIR}")
      endif()
      add_custom_command(
        OUTPUT ${_rpc_dbus_hh} ${_rpc_dbus_cc}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_rpc_dbus_pregen_hh} ${_rpc_dbus_hh}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_rpc_dbus_pregen_cc} ${_rpc_dbus_cc}
        DEPENDS ${_rpc_dbus_pregen_hh} ${_rpc_dbus_pregen_cc}
        COMMENT "Using checked-in DBus binding for ${HEADER}")
    endif()

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

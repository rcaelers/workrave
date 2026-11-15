# protobuf_generate defaults to the target's Windows protoc. Allow a native
# generator of the same version while retaining the Windows protobuf library.
set(source "${SOURCE_DIR}/src/CMakeLists.txt")
file(READ "${source}" original)
set(needle "protobuf_generate(\n")
set(replacement "protobuf_generate(\n    PROTOC_EXE \"\${WORKRAVE_HOST_PROTOC}\"\n")
if(NOT original MATCHES "PROTOC_EXE")
  if(NOT original MATCHES "protobuf_generate\\(")
    message(FATAL_ERROR "Sigstore protobuf generation rules have changed")
  endif()
  string(REPLACE "${needle}" "${replacement}" patched "${original}")
  file(WRITE "${source}" "${patched}")
endif()

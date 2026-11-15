# MinGW headers use lower-case names on case-sensitive cross-build hosts.
set(source "${SOURCE_DIR}/third_party/mini_chromium/mini_chromium/base/rand_util.cc")
file(READ "${source}" original)
string(REPLACE "#include <NTSecAPI.h>" "#include <ntsecapi.h>" patched "${original}")
if(NOT patched STREQUAL original)
  file(WRITE "${source}" "${patched}")
endif()

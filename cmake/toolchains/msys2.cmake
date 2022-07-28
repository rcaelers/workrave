set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_PREFIX_PATH
    ${MINGW_PREFIX}
)

if (DEFINED ENV{MINGW_PREFIX})
    set(SYS_ROOT               "$ENV{MINGW_PREFIX}")
else()
    set(SYS_ROOT               "$ENV{MINGW_ROOT}")
endif()

set(CMAKE_ASM_MASM_COMPILER    "uasm")
set(CMAKE_C_COMPILER           "clang")
set(CMAKE_CXX_COMPILER         "clang++")
set(CMAKE_RC_COMPILER          "llvm-rc")

set(CMAKE_C_FLAGS              "-fuse-ld=lld")
set(CMAKE_CXX_FLAGS            ${CMAKE_C_FLAGS})

set(CMAKE_C_FLAGS_DEBUG        "-O0 -g -fuse-ld=lld")
set(CMAKE_CXX_FLAGS_DEBUG      ${CMAKE_C_FLAGS_DEBUG})

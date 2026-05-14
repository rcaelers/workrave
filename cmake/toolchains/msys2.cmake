set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

list(APPEND CMAKE_PREFIX_PATH
    ${MINGW_PREFIX}
)
# Detect mingw root and convert MSYS2 paths to Windows paths
if (DEFINED ENV{MINGW_PREFIX})
    set(MSYS2_PATH "$ENV{MINGW_PREFIX}")
elseif (DEFINED ENV{MSYSTEM_PREFIX})
    set(MSYS2_PATH "$ENV{MSYSTEM_PREFIX}")
elseif (DEFINED ENV{MINGW_ROOT})
    set(SYS_ROOT "$ENV{MINGW_ROOT}")
else()
    message(WARNING "Could not determine SYS_ROOT. Please set MINGW_PREFIX, MSYSTEM_PREFIX, or MINGW_ROOT environment variable.")
endif()

# Convert MSYS2 path to Windows path using cygpath if needed
if (DEFINED MSYS2_PATH)
    execute_process(
        COMMAND cygpath -m "${MSYS2_PATH}"
        OUTPUT_VARIABLE SYS_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (NOT SYS_ROOT)
        set(SYS_ROOT "${MSYS2_PATH}")
    endif()
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|AMD64")
    set(CMAKE_ASM_MASM_COMPILER    "uasm")
endif()

set(CMAKE_C_COMPILER           "clang")
set(CMAKE_CXX_COMPILER         "clang++")
set(CMAKE_RC_COMPILER          "llvm-rc")

set(CMAKE_C_FLAGS              "-fuse-ld=lld")
set(CMAKE_CXX_FLAGS            ${CMAKE_C_FLAGS})

set(CMAKE_C_FLAGS_DEBUG        "-O0 -g -fuse-ld=lld")
set(CMAKE_CXX_FLAGS_DEBUG      ${CMAKE_C_FLAGS_DEBUG})

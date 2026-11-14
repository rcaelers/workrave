set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Retain the toolchain root across automatic CMake reconfiguration, including
# reconfiguration launched by an IDE outside the MSYS2 shell.
if (DEFINED ENV{MINGW_PREFIX})
    set(MSYS2_PATH "$ENV{MINGW_PREFIX}")
elseif (DEFINED ENV{MSYSTEM_PREFIX})
    set(MSYS2_PATH "$ENV{MSYSTEM_PREFIX}")
elseif (DEFINED ENV{MINGW_ROOT})
    set(SYS_ROOT "$ENV{MINGW_ROOT}")
elseif (NOT SYS_ROOT)
    if (IS_ABSOLUTE "${CMAKE_CXX_COMPILER}")
        set(MSYS2_COMPILER "${CMAKE_CXX_COMPILER}")
    else()
        find_program(MSYS2_COMPILER NAMES clang++ NO_CACHE)
    endif()
    if (MSYS2_COMPILER)
        get_filename_component(MSYS2_BIN_DIR "${MSYS2_COMPILER}" DIRECTORY)
        get_filename_component(MSYS2_PREFIX "${MSYS2_BIN_DIR}" DIRECTORY)
        if (EXISTS "${MSYS2_PREFIX}/lib/libkernel32.a")
            set(SYS_ROOT "${MSYS2_PREFIX}")
        endif()
    endif()
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

# Native Windows callers can supply backslashes in MINGW_ROOT. Keep paths in
# CMake form so generated install scripts do not interpret them as escapes.
if (SYS_ROOT)
    file(TO_CMAKE_PATH "${SYS_ROOT}" SYS_ROOT)
    set(SYS_ROOT "${SYS_ROOT}" CACHE PATH "MSYS2 target environment root" FORCE)
    list(APPEND CMAKE_PREFIX_PATH "${SYS_ROOT}")
else()
    message(FATAL_ERROR "Could not determine the MSYS2 root. Set SYS_ROOT, MINGW_PREFIX, MSYSTEM_PREFIX, or MINGW_ROOT.")
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

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
    if (CROSS_COMPILE_SYSROOT)
        set(MSYS2_PATH "${CROSS_COMPILE_SYSROOT}")
    else()
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

# Optional: run a native (fast) host compiler that cross-emits code for a
# different MSYS2 environment's target, e.g. a native ARM64 clang.exe
# (clangarm64) emitting x86_64-w64-windows-gnu code/libs sourced from the
# clang64 sysroot. This avoids running the compiler itself under Windows'
# x86-on-ARM64 emulation, which is otherwise the dominant build-time cost
# on an ARM64 host. Off by default; set CROSS_COMPILE_SYSROOT to enable.
if(DEFINED CROSS_COMPILE_SYSROOT AND NOT "${CROSS_COMPILE_SYSROOT}" STREQUAL "")
    execute_process(
        COMMAND cygpath -m "${CROSS_COMPILE_SYSROOT}"
        OUTPUT_VARIABLE CROSS_SYSROOT_WIN
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT CROSS_SYSROOT_WIN)
        set(CROSS_SYSROOT_WIN "${CROSS_COMPILE_SYSROOT}")
    endif()

    file(GLOB _cross_clang_resdirs "${CROSS_SYSROOT_WIN}/lib/clang/*")
    list(LENGTH _cross_clang_resdirs _cross_clang_resdirs_len)
    if(NOT _cross_clang_resdirs_len EQUAL 1)
        message(FATAL_ERROR "CROSS_COMPILE_SYSROOT: expected exactly one lib/clang/<ver> dir under ${CROSS_SYSROOT_WIN}, found: ${_cross_clang_resdirs}")
    endif()
    set(_cross_resource_dir "${_cross_clang_resdirs}")

    set(CMAKE_SYSTEM_NAME       Windows)
    set(CMAKE_SYSTEM_PROCESSOR  AMD64)

    set(_cross_target "x86_64-w64-windows-gnu")
    set(CMAKE_C_COMPILER_TARGET   "${_cross_target}")
    set(CMAKE_CXX_COMPILER_TARGET "${_cross_target}")
    set(CMAKE_SYSROOT "${CROSS_SYSROOT_WIN}")

    set(_cross_flags "-target ${_cross_target} --sysroot=${CROSS_SYSROOT_WIN} -resource-dir=${_cross_resource_dir}")
    set(CMAKE_C_FLAGS         "${_cross_flags} -fuse-ld=lld")
    set(CMAKE_CXX_FLAGS       "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG   "-O0 -g ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

    # Qt/Crashpad/DBus etc. are the x86_64 clang64 build; host tools like
    # moc/uic/rcc are x86_64 executables and will run under emulation, but
    # that cost is negligible next to the compile fleet.
    list(APPEND CMAKE_PREFIX_PATH "${CROSS_SYSROOT_WIN}")

    # CMAKE_SYSTEM_NAME activates strict root-path search, so find_program()
    # (Gettext, git, ...) only sees CMAKE_FIND_ROOT_PATH entries. Tools like
    # msgfmt/msgmerge live under the MSYS2 root's usr/bin, one level above
    # the clang64 prefix, not inside it — include that root too.
    get_filename_component(_cross_msys_root "${CROSS_SYSROOT_WIN}" DIRECTORY)
    list(APPEND CMAKE_FIND_ROOT_PATH "${CROSS_SYSROOT_WIN}" "${_cross_msys_root}/usr" "${_cross_msys_root}")
endif()

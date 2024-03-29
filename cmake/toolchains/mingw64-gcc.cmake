set(SYS_ROOT_PREFIX "")

set(TOOLCHAIN_ROOT "/usr/x86_64-w64-mingw32")
set(SYS_ROOT "${SYS_ROOT_PREFIX}/usr/x86_64-w64-mingw32/sys-root/mingw/")

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

set (WINE wine)

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_ROOT} ${SYS_ROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories(${TOOLCHAIN_ROOT}/include)
include_directories(${SYS_ROOT}/include)

set(ENV{PKG_CONFIG_LIBDIR} "${SYS_ROOT}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${SYS_ROOT_PREFIX}")

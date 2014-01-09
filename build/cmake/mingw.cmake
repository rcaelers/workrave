SET(MINGW_TARGET /home/robc/src/runtime/usr/i686-w64-mingw32/sys-root/mingw/)
SET(MINGW_ENV /usr/i686-w64-mingw32)

SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER  i686-w64-mingw32-windres)

SET(CMAKE_FIND_ROOT_PATH ${MINGW_ENV} ${MINGW_TARGET})

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories(${MINGW_ENV}/include)
include_directories(${MINGW_TARGET}/include)

link_directories("/home/robc/src/tinderbox/prebuilt/Release")

set(pkgconfigLibDir)
set(pkgconfigLibDir )
set(ENV{PKG_CONFIG_LIBDIR} "${MINGW_TARGET}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_SYSROOT_DIR, "/home/robc/src/runtime/")
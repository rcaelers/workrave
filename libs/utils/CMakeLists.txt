set(SRC
  src/GlibThread.cc
  src/Locale.cc
  src/StringUtil.cc
  src/TimeSource.cc
  src/Timer.cc
  src/Util.cc
  src/debug.cc
  )

if (PLATFORM_OS_WIN32)
  set(SRC ${SRC} src/win32/crashlog.cc)
endif()

add_definitions(
  -DWORKRAVE_PKGDATADIR="${PKGDATADIR}"
  -DWORKRAVE_DATADIR="${DATADIR}"
)

include_directories(
  ${CMAKE_SOURCE_DIR}/libs/utils/include/utils
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  )

add_library(workrave-libs-utils STATIC ${SRC})


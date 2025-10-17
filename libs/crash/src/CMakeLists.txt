if (HAVE_CRASHPAD)
  add_library(workrave-libs-crash STATIC CrashReporter.cc)

  target_link_libraries(workrave-libs-crash workrave-libs-utils workrave-libs-input-monitor crashpad::client)
  target_include_directories(workrave-libs-crash PUBLIC ${CMAKE_SOURCE_DIR}/libs/crash/include)
  target_compile_definitions(workrave-libs-crash PRIVATE WORKRAVE_BUILD_DATETIME="${WORKRAVE_BUILD_DATETIME}")
endif()

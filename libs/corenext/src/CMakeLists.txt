add_library(workrave-libs-core-next STATIC
  Break.cc
  BreakDBus.cc
  BreakStateModel.cc
  BreakStatistics.cc
  BreakConfig.cc
  BreaksControl.cc
  Core.cc
  CoreDBus.cc
  CoreModes.cc
  CoreConfig.cc
  CoreHooks.cc
  DayTimePred.cc
  LocalActivityMonitor.cc
  ReadingActivityMonitor.cc
  Statistics.cc
  Timer.cc
  TimerActivityMonitor.cc)

target_code_coverage(workrave-libs-core-next)

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR} DBusWorkraveNext)
  target_sources(workrave-libs-core-next PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkraveNext.cc)
endif()

if (WITH_CORE_NEXT)
  if (HAVE_GSETTINGS)
    gsettings_add_schemas(${CMAKE_CURRENT_SOURCE_DIR})
  endif()
endif()

target_link_libraries(workrave-libs-core-next
  PUBLIC
  workrave-libs-utils
  workrave-libs-config
  workrave-libs-dbus)

target_link_libraries(workrave-libs-core-next PUBLIC
  $<$<NOT:$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>>:workrave-libs-input-monitor>
  $<$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>:workrave-libs-input-monitor-stub>
)

target_include_directories(
  workrave-libs-core-next
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/corenext/include)

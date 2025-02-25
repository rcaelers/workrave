add_library(workrave-libs-core STATIC
  Break.cc
  #BreakDBus.cc
  #BreakStateModel.cc
  #BreakStatistics.cc
  #BreakConfig.cc
  #BreaksControl.cc
  Core.cc
  #CoreDBus.cc
  #CoreModes.cc
  CoreConfig.cc
  CoreHooks.cc
  DayTimePred.cc
  LocalActivityMonitor.cc
  ReadingActivityMonitor.cc
  Statistics.cc
  Test.cc
  Timer.cc
  #TimerActivityMonitor.cc
  BreakControl.cc
  TimePredFactory.cc
  )

target_code_coverage(workrave-libs-core)

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR} DBusWorkrave)
  target_sources(workrave-libs-core PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc)
endif()

if (HAVE_GSETTINGS)
  gsettings_add_schemas(${CMAKE_CURRENT_SOURCE_DIR})
endif()

target_link_libraries(workrave-libs-core
  PUBLIC
  workrave-libs-utils
  workrave-libs-config
  workrave-libs-dbus)

target_link_libraries(workrave-libs-core PUBLIC
  $<$<NOT:$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>>:workrave-libs-input-monitor>
  $<$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>:workrave-libs-input-monitor-stub>
)

target_include_directories(
  workrave-libs-core
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/core/include)


set(SRC
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
  TimerActivityMonitor.cc
  )

if (HAVE_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR} DBusWorkrave)

  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc
  )
endif (HAVE_DBUS)

if (HAVE_GSETTINGS)
  # TODO: compile glib enum?
  gsettings_add_schemas(workrave-libs-core ${CMAKE_SOURCE_DIR}/libs/core/src)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/src
  ${CMAKE_SOURCE_DIR}/libs/core/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  )

add_library(workrave-libs-core STATIC ${SRC})

add_cppcheck(workrave-libs-core)

target_link_libraries(workrave-libs-core ${CMAKE_THREAD_LIBS_INIT})

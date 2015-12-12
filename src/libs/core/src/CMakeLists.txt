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
  dbus_generate(DBusWorkrave ${CMAKE_CURRENT_SOURCE_DIR}/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc)
  set(SRC ${SRC}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc
  )
endif (HAVE_DBUS)

if (HAVE_GSETTINGS)
  # TODO: compile glib enum?
  gsettings_add_schemas(workrave-backend ${CMAKE_SOURCE_DIR}/backend/src)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/src
  ${CMAKE_SOURCE_DIR}/backend/include
  ${CMAKE_SOURCE_DIR}/libs/utils/include
  ${CMAKE_SOURCE_DIR}/libs/config/include
  ${CMAKE_SOURCE_DIR}/libs/input-monitor/include
  ${CMAKE_SOURCE_DIR}/libs/dbus/include
  )

add_library(workrave-backend STATIC ${SRC})

add_cppcheck(workrave-backend)

target_link_libraries(workrave-backend ${CMAKE_THREAD_LIBS_INIT})

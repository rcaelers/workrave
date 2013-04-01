set(SRC
  src/ActivityMonitor.cc
  src/Break.cc
  src/BreaksControl.cc
  src/Core.cc
  src/CoreConfig.cc
  src/CoreHooks.cc
  src/DayTimePred.cc
  src/LocalActivityMonitor.cc
  src/ReadingActivityMonitor.cc
  src/Statistics.cc
  src/Timer.cc
  src/TimerActivityMonitor.cc
  )

if (HAVE_DBUS)
  dbus_generate(DBusWorkrave ${CMAKE_CURRENT_SOURCE_DIR}/src/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc)
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

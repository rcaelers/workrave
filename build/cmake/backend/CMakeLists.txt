project(Workrave)

SET(COMMON_DIR ${CMAKE_SOURCE_DIR}/../../common)
SET(BACKEND_DIR ${CMAKE_SOURCE_DIR}/../../backend)

set(BACKEND_SOURCES 
  ${BACKEND_DIR}/include/CoreConfig.hh
  ${BACKEND_DIR}/include/CoreFactory.hh
  ${BACKEND_DIR}/include/IApp.hh
  ${BACKEND_DIR}/include/IBreak.hh
  ${BACKEND_DIR}/include/IBreakResponse.hh
  ${BACKEND_DIR}/include/IConfigurator.hh
  ${BACKEND_DIR}/include/IConfiguratorListener.hh
  ${BACKEND_DIR}/include/ICore.hh
  ${BACKEND_DIR}/include/ICoreEventListener.hh
  ${BACKEND_DIR}/include/IStatistics.hh
  ${BACKEND_DIR}/src/ActivityMonitor.cc
  ${BACKEND_DIR}/src/ActivityMonitor.hh
  ${BACKEND_DIR}/src/ActivityMonitorListener.hh
  ${BACKEND_DIR}/src/Break.cc
  ${BACKEND_DIR}/src/Break.hh
  ${BACKEND_DIR}/src/BreakControl.cc
  ${BACKEND_DIR}/src/BreakControl.hh
  ${BACKEND_DIR}/src/ConfigBackendAdapter.hh
  ${BACKEND_DIR}/src/Configurator.cc
  ${BACKEND_DIR}/src/Configurator.hh
  ${BACKEND_DIR}/src/ConfiguratorFactory.cc
  ${BACKEND_DIR}/src/ConfiguratorFactory.hh
  ${BACKEND_DIR}/src/Core.cc
  ${BACKEND_DIR}/src/Core.hh
  ${BACKEND_DIR}/src/CoreConfig.cc
  ${BACKEND_DIR}/src/CoreFactory.cc
  ${BACKEND_DIR}/src/DayTimePred.cc
  ${BACKEND_DIR}/src/DayTimePred.hh
  ${BACKEND_DIR}/src/GlibIniConfigurator.cc
  ${BACKEND_DIR}/src/GlibIniConfigurator.hh
  ${BACKEND_DIR}/src/IActivityMonitor.hh
  ${BACKEND_DIR}/src/IConfigBackend.hh
  ${BACKEND_DIR}/src/IDistributionClientMessage.hh
  ${BACKEND_DIR}/src/IInputMonitor.hh
  ${BACKEND_DIR}/src/IInputMonitorFactory.hh
  ${BACKEND_DIR}/src/IInputMonitorListener.hh
  ${BACKEND_DIR}/src/IdleLogManager.cc
  ${BACKEND_DIR}/src/IdleLogManager.hh
  ${BACKEND_DIR}/src/InputMonitor.cc
  ${BACKEND_DIR}/src/InputMonitor.hh
  ${BACKEND_DIR}/src/InputMonitor.icc
  ${BACKEND_DIR}/src/InputMonitorFactory.cc
  ${BACKEND_DIR}/src/InputMonitorFactory.hh
  ${BACKEND_DIR}/src/InputMonitorFactoryInterface.hh
  ${BACKEND_DIR}/src/PacketBuffer.cc
  ${BACKEND_DIR}/src/PacketBuffer.hh
  ${BACKEND_DIR}/src/Statistics.cc
  ${BACKEND_DIR}/src/Statistics.hh
  ${BACKEND_DIR}/src/TimePred.hh
  ${BACKEND_DIR}/src/TimePredFactory.cc
  ${BACKEND_DIR}/src/TimePredFactory.hh
  ${BACKEND_DIR}/src/TimeSource.hh
  ${BACKEND_DIR}/src/Timer.cc
  ${BACKEND_DIR}/src/Timer.hh
  ${BACKEND_DIR}/src/Timer.icc
  ${BACKEND_DIR}/src/TimerActivityMonitor.hh
  ${BACKEND_DIR}/src/Variant.hh
  )

if (APPLE)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${BACKEND_DIR}/src/osx/OSXConfigurator.cc
    ${BACKEND_DIR}/src/osx/OSXConfigurator.hh
    ${BACKEND_DIR}/src/osx/OSXInputMonitor.cc
    ${BACKEND_DIR}/src/osx/OSXInputMonitor.hh
    ${BACKEND_DIR}/src/osx/OSXInputMonitorFactory.cc
    ${BACKEND_DIR}/src/osx/OSXInputMonitorFactory.hh
  )
endif (APPLE)
  
if (UNIX)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${BACKEND_DIR}/src/unix/GConfConfigurator.cc
    ${BACKEND_DIR}/src/unix/GConfConfigurator.hh
    ${BACKEND_DIR}/src/unix/UnixInputMonitorFactory.cc
    ${BACKEND_DIR}/src/unix/UnixInputMonitorFactory.hh
    ${BACKEND_DIR}/src/unix/X11InputMonitor.cc
    ${BACKEND_DIR}/src/unix/X11InputMonitor.hh
    ${BACKEND_DIR}/src/unix/dummy.c
  )
endif (UNIX)

if (WIN32)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${BACKEND_DIR}/src/win32/Harpoon.cc
    ${BACKEND_DIR}/include/Harpoon.hh
    ${BACKEND_DIR}/src/win32/W32ActiveSetup.cc
    ${BACKEND_DIR}/src/win32/W32ActiveSetup.hh
    ${BACKEND_DIR}/src/win32/W32AlternateMonitor.cc
    ${BACKEND_DIR}/src/win32/W32AlternateMonitor.hh
    ${BACKEND_DIR}/src/win32/W32Configurator.cc
    ${BACKEND_DIR}/src/win32/W32Configurator.hh
    ${BACKEND_DIR}/src/win32/W32InputMonitor.cc
    ${BACKEND_DIR}/src/win32/W32InputMonitor.hh
    ${BACKEND_DIR}/src/win32/W32InputMonitorFactory.cc
    ${BACKEND_DIR}/src/win32/W32InputMonitorFactory.hh
    ${BACKEND_DIR}/src/win32/W32LowLevelMonitor.cc
    ${BACKEND_DIR}/src/win32/W32LowLevelMonitor.hh
    )

  include_directories( 
    "${COMMON_DIR}/include/win32"
    "${COMMON_DIR}/win32/harpoon/include"
    "${COMMON_DIR}/win32/harpoonHelper/include"
    "${BACKEND_DIR}/src/win32"
  )

endif (WIN32)

if (HAVE_GDOME)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${BACKEND_DIR}/src/XMLConfigurator.cc
    ${BACKEND_DIR}/src/XMLConfigurator.hh
  )
endif (HAVE_GDOME)

if (HAVE_DISTRIBUTION)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${BACKEND_DIR}/include/DistributionLogListener.hh
    ${BACKEND_DIR}/include/IDistributionManager.hh
    ${BACKEND_DIR}/src/DistributionLink.hh
    ${BACKEND_DIR}/src/DistributionListener.hh
    ${BACKEND_DIR}/src/DistributionManager.cc
    ${BACKEND_DIR}/src/DistributionManager.hh
    ${BACKEND_DIR}/src/DistributionSocketLink.cc
    ${BACKEND_DIR}/src/DistributionSocketLink.hh
    ${BACKEND_DIR}/src/FakeActivityMonitor.hh
    ${BACKEND_DIR}/src/GNetSocketDriver.cc
    ${BACKEND_DIR}/src/GNetSocketDriver.hh
    ${BACKEND_DIR}/src/GIOSocketDriver.cc
    ${BACKEND_DIR}/src/GIOSocketDriver.hh
    ${BACKEND_DIR}/src/SocketDriver.hh
    ${BACKEND_DIR}/src/SocketDriver.icc
    ${BACKEND_DIR}/src/SocketDriver.cc
    ${BACKEND_DIR}/src/Test.cc
    ${BACKEND_DIR}/src/Test.hh
  )
endif(HAVE_DISTRIBUTION)

if (HAVE_DBUS)
  add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/../../common/bin/dbusgen.py -l C++ -s ${BACKEND_DIR}/src/workrave-service.xml DBusWorkrave
	DEPENDS ${BACKEND_DIR}/src/workrave-service.xml
  )
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkrave.cc
  )
  include_directories(${DBUS_INCLUDES})
endif (HAVE_DBUS)
    

include_directories( 
  "${COMMON_DIR}/include"
  "${BACKEND_DIR}/include"
  "${BACKEND_DIR}/src"
)


include_directories(${GLIB_INCLUDES})
if (HAVE_DBUS)
  include_directories(${DBUS_INCLUDES})
endif(HAVE_DBUS)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(workrave-backend STATIC ${BACKEND_SOURCES})

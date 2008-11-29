project(Workrave)

SET(BACKEND_DIR ${CMAKE_SOURCE_DIR}/../../backend)

set(BACKEND_SOURCES 
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
  ${BACKEND_DIR}/src/DistributionLink.hh
  ${BACKEND_DIR}/src/DistributionListener.hh
  ${BACKEND_DIR}/src/DistributionManager.cc
  ${BACKEND_DIR}/src/DistributionManager.hh
  ${BACKEND_DIR}/src/DistributionSocketLink.cc
  ${BACKEND_DIR}/src/DistributionSocketLink.hh
  ${BACKEND_DIR}/src/FakeActivityMonitor.hh
  ${BACKEND_DIR}/src/GlibIniConfigurator.cc
  ${BACKEND_DIR}/src/GlibIniConfigurator.hh
  ${BACKEND_DIR}/src/GNetSocketDriver.cc
  ${BACKEND_DIR}/src/GNetSocketDriver.hh
  ${BACKEND_DIR}/src/win32/Harpoon.cc
  ${BACKEND_DIR}/src/win32/Harpoon.hh
  ${BACKEND_DIR}/src/IActivityMonitor.hh
  ${BACKEND_DIR}/src/IConfigBackend.hh
  ${BACKEND_DIR}/src/IDistributionClientMessage.hh
  ${BACKEND_DIR}/src/IdleLogManager.cc
  ${BACKEND_DIR}/src/IdleLogManager.hh
  ${BACKEND_DIR}/src/IInputMonitor.hh
  ${BACKEND_DIR}/src/IInputMonitorFactory.hh
  ${BACKEND_DIR}/src/IInputMonitorListener.hh
  ${BACKEND_DIR}/src/InputMonitor.cc
  ${BACKEND_DIR}/src/InputMonitor.hh
  ${BACKEND_DIR}/src/InputMonitorFactory.cc
  ${BACKEND_DIR}/src/InputMonitorFactory.hh
  ${BACKEND_DIR}/src/InputMonitorFactoryInterface.hh
  ${BACKEND_DIR}/src/PacketBuffer.cc
  ${BACKEND_DIR}/src/PacketBuffer.hh
  ${BACKEND_DIR}/src/SocketDriver.hh
  ${BACKEND_DIR}/src/Statistics.cc
  ${BACKEND_DIR}/src/Statistics.hh
  ${BACKEND_DIR}/src/TimePred.hh
  ${BACKEND_DIR}/src/TimePredFactory.cc
  ${BACKEND_DIR}/src/TimePredFactory.hh
  ${BACKEND_DIR}/src/Timer.cc
  ${BACKEND_DIR}/src/Timer.hh
  ${BACKEND_DIR}/src/TimerActivityMonitor.hh
  ${BACKEND_DIR}/src/TimeSource.hh
  ${BACKEND_DIR}/src/Variant.hh
  )

if (WIN32)
  set(BACKEND_SOURCES ${BACKEND_SOURCES}
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
endif (WIN32)

include_directories( 
  "${BACKEND_DIR}/include"
  "${BACKEND_DIR}/src"
  "${BACKEND_DIR}/src/win32"
)

add_definitions( 
  -DWIN32
  -D_DEBUG
  -D_WINDOWS
)

add_library(workrave-backend STATIC ${BACKEND_SOURCES})

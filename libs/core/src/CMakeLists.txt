add_library(workrave-libs-core STATIC
  Break.cc
  #BreakStateModel.cc
  #BreakStatistics.cc
  #BreakConfig.cc
  #BreaksControl.cc
  Core.cc
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

if (HAVE_CORE_DBUS)
  rpc_generate_dbus_source(${CMAKE_CURRENT_SOURCE_DIR}/Core.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusLegacyCoreBinding
    TARGET workrave-libs-core
    PREGENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen
    ADAPTER_NAMESPACE workrave::core::legacy_rpc)
  rpc_generate_dbus_source(${CMAKE_CURRENT_SOURCE_DIR}/Break.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusLegacyBreakBinding
    TARGET workrave-libs-core
    PREGENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen
    ADAPTER_NAMESPACE workrave::core::legacy_rpc)
  rpc_generate_dbus_source(${CMAKE_SOURCE_DIR}/libs/config/include/config/IConfigurator.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusLegacyConfigBinding
    TARGET workrave-libs-core
    PREGENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen
    HEADER_INCLUDE "config/IConfigurator.hh"
    ADAPTER_NAMESPACE workrave::core::legacy_rpc)

  target_sources(workrave-libs-core PRIVATE
    LegacyRpcDBusServer.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusLegacyCoreBindingDBus.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusLegacyBreakBindingDBus.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusLegacyConfigBindingDBus.cc)
  target_include_directories(workrave-libs-core PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/libs/rpc/include)

  if (RPC_DBUS_BACKEND STREQUAL "qt")
    target_link_libraries(workrave-libs-core PUBLIC workrave-libs-rpc-dbus-qt)
  elseif (RPC_DBUS_BACKEND STREQUAL "gio")
    target_link_libraries(workrave-libs-core PUBLIC workrave-libs-rpc-dbus-gio)
  endif()
endif()

if (HAVE_GSETTINGS AND NOT HAVE_CORE_SHADOW)
  gsettings_add_schemas(${CMAKE_CURRENT_SOURCE_DIR})
endif()

target_link_libraries(workrave-libs-core
  PUBLIC
  workrave-libs-utils
  workrave-libs-config)

target_link_libraries(workrave-libs-core PUBLIC
  $<$<NOT:$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>>:workrave-libs-input-monitor>
  $<$<BOOL:$<TARGET_PROPERTY:USE_STUBS>>:workrave-libs-input-monitor-stub>
)

target_include_directories(
  workrave-libs-core
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/core/include)

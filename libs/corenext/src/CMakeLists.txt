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

if (HAVE_DBUS AND NOT HAVE_RPC_DBUS)
  dbus_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/workrave-service.xml ${CMAKE_CURRENT_BINARY_DIR} DBusWorkraveNext)
  target_sources(workrave-libs-core-next PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DBusWorkraveNext.cc)
endif()

if (HAVE_RPC_DBUS)
  rpc_generate_dbus_source(${CMAKE_CURRENT_SOURCE_DIR}/Core.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusCoreBinding
    TARGET workrave-libs-core-next
    ADAPTER_NAMESPACE workrave::core::rpc)
  rpc_generate_dbus_source(${CMAKE_CURRENT_SOURCE_DIR}/Break.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusBreakBinding
    TARGET workrave-libs-core-next
    ADAPTER_NAMESPACE workrave::core::rpc)
  rpc_generate_dbus_source(${CMAKE_SOURCE_DIR}/libs/config/include/config/IConfigurator.hh ${CMAKE_CURRENT_BINARY_DIR} RpcDBusConfigBinding
    TARGET workrave-libs-core-next
    HEADER_INCLUDE "config/IConfigurator.hh"
    ADAPTER_NAMESPACE workrave::core::rpc)

  target_sources(workrave-libs-core-next PRIVATE
    RpcDBusServer.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusCoreBindingDBus.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusBreakBindingDBus.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcDBusConfigBindingDBus.cc)
  target_include_directories(workrave-libs-core-next PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (HAVE_RPC_DBUS)
  # The direct bindings include rpc/Duration.hh for annotated chrono methods.
  target_include_directories(workrave-libs-core-next PRIVATE ${CMAKE_SOURCE_DIR}/libs/rpc/include)
endif()

if (HAVE_RPC)
  # BreaksControl.hh uses rpc::InstanceRegistry<> (see get_break_registry())
  # to route BreakService calls to the right one of the three live Break
  # instances — the gRPC analog of BreakDBus's per-object-path registration.
  # InstanceRegistry.hh/RpcException.hh are header-only with no link
  # dependency of their own, so this only needs an include path, NOT linking
  # workrave-libs-rpc (which would pull gRPC::grpc++ back onto every
  # consumer's link line — exactly what workrave-libs-core-next-rpc below
  # exists to avoid).
  target_include_directories(workrave-libs-core-next PRIVATE ${CMAKE_SOURCE_DIR}/libs/rpc/include)

  # Public wire names are declared beside the interfaces as
  # @rpc(service="workrave.<Service>"). Payload types live in separate
  # imported packages so their generated C++ names are isolated in
  # workrave::rpc::core and workrave::rpc::breaks.
  rpc_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/Core.hh ${CMAKE_CURRENT_BINARY_DIR} RpcCore
    TARGET workrave-libs-core-next
    PROTO_TYPES_PACKAGE workrave.rpc.core
    GRPC_SERVICES_NAMESPACE rpc
    ADAPTER_NAMESPACE workrave::core::rpc)
  rpc_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/Break.hh ${CMAKE_CURRENT_BINARY_DIR} RpcBreak
    TARGET workrave-libs-core-next
    PROTO_TYPES_PACKAGE workrave.rpc.breaks
    GRPC_SERVICES_NAMESPACE rpc
    ADAPTER_NAMESPACE workrave::core::rpc)

  # Deliberately a SEPARATE, opt-in static library rather than sources added
  # to workrave-libs-core-next itself: for a static library, CMake can't hide
  # its link dependencies from whoever finally links it — even PRIVATE deps
  # still land on the final executable's link line, because the .a doesn't
  # resolve its own symbols. Folding grpc++/protobuf/absl (a huge dylib
  # graph) into workrave-libs-core-next broke fmt symbol resolution for
  # *every* consumer, including ones that never touch RPC (e.g.
  # workrave-core-next-integration-test). Only a target that actually wants
  # the generated CoreService/BreakService should pay that cost, by linking
  # workrave-libs-core-next-rpc explicitly.
  add_library(workrave-libs-core-next-rpc STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/RpcCoreServiceImpl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcCore.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcCore.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcCoreTypes.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcBreakServiceImpl.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcBreak.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcBreak.grpc.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/RpcBreakTypes.pb.cc
    # Wires the three generated ServiceImpls above (plus ConfigServiceImpl,
    # from workrave-libs-config-rpc) into a single running rpc::RpcServer for
    # the real Core/BreaksControl/Configurator. Core::init_rpc() constructs
    # this — see RpcCoreServer.hh for why this lives here and not in
    # workrave-libs-core-next itself.
    RpcCoreServer.cc)
  target_link_libraries(workrave-libs-core-next-rpc
    PUBLIC workrave-libs-core-next
    PRIVATE workrave-libs-rpc workrave-libs-config-rpc gRPC::grpc++ protobuf::libprotobuf)
  target_include_directories(workrave-libs-core-next-rpc PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    # RpcCoreServer.cc constructs a
    # workrave::config::rpc::ConfigServiceServiceImpl, generated by the same
    # rpc_generate_source() call as libs/config/src/CMakeLists.txt's
    # workrave-libs-config-rpc.
    ${CMAKE_BINARY_DIR}/libs/config/src)

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
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/libs/corenext/include)

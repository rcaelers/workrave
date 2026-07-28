add_library(workrave-libs-rpc-common STATIC Duration.cc)
target_include_directories(workrave-libs-rpc-common PUBLIC ${CMAKE_SOURCE_DIR}/libs/rpc/include)

if (HAVE_RPC)
  add_library(workrave-libs-rpc STATIC RpcServer.cc)
  target_link_libraries(workrave-libs-rpc
    PUBLIC workrave-libs-rpc-common gRPC::grpc++ gRPC::grpc++_reflection)
  target_include_directories(workrave-libs-rpc PUBLIC ${CMAKE_SOURCE_DIR}/libs/rpc/include)
endif()

if (HAVE_RPC_DBUS)
  add_library(workrave-libs-rpc-dbus-common INTERFACE)
  target_include_directories(workrave-libs-rpc-dbus-common
    INTERFACE ${CMAKE_SOURCE_DIR}/libs/rpc/include)

  if (RPC_DBUS_BACKEND STREQUAL "qt")
    add_library(workrave-libs-rpc-dbus-qt STATIC dbus/QtServer.cc)
    target_link_libraries(workrave-libs-rpc-dbus-qt
      PUBLIC workrave-libs-rpc-common workrave-libs-rpc-dbus-common Qt${QT_VERSION_MAJOR}::DBus)
    target_include_directories(workrave-libs-rpc-dbus-qt
      PUBLIC ${CMAKE_SOURCE_DIR}/libs/rpc/include)
    target_compile_definitions(workrave-libs-rpc-dbus-qt PUBLIC WORKRAVE_RPC_DBUS_QT)
  elseif (RPC_DBUS_BACKEND STREQUAL "gio")
    add_library(workrave-libs-rpc-dbus-gio STATIC dbus/GioServer.cc)
    target_link_libraries(workrave-libs-rpc-dbus-gio
      PUBLIC workrave-libs-rpc-common workrave-libs-rpc-dbus-common ${GLIB_LIBRARIES})
    target_include_directories(workrave-libs-rpc-dbus-gio
      PUBLIC ${CMAKE_SOURCE_DIR}/libs/rpc/include ${GLIB_INCLUDE_DIRS})
    target_link_directories(workrave-libs-rpc-dbus-gio PUBLIC ${GLIB_LIBRARY_DIRS})
    target_compile_options(workrave-libs-rpc-dbus-gio PUBLIC ${GLIB_CFLAGS_OTHER})
    target_compile_definitions(workrave-libs-rpc-dbus-gio PUBLIC WORKRAVE_RPC_DBUS_GIO)
  else()
    message(FATAL_ERROR "Unknown internal RPC DBus backend: ${RPC_DBUS_BACKEND}")
  endif()
endif()

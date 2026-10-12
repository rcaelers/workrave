# The screenshot driver is a separate, uninstalled development application.
option(WITH_WINDOW_PREVIEW "Build the development window screenshot driver" OFF)
if (NOT WITH_WINDOW_PREVIEW)
  return()
endif()
if (NOT HAVE_GRPC)
  message(FATAL_ERROR "WITH_WINDOW_PREVIEW requires WITH_GRPC")
endif()

if (HAVE_APP_QT)
  qt_add_executable(workrave-window-preview WindowPreview.cc)
  set(_preview_toolkit workrave-toolkit-qt)
  set(_preview_toolkit_dir qt)
else()
  add_executable(workrave-window-preview WindowPreview.cc)
  set(_preview_toolkit workrave-toolkit-gtkmm)
  set(_preview_toolkit_dir gtkmm)
endif()

target_include_directories(workrave-window-preview PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../toolkits/${_preview_toolkit_dir})

# Reuse the production UI and its platform dependencies; no production target
# references this executable or its RPC service.
get_target_property(_preview_toolkit_libraries ${_preview_toolkit} LINK_LIBRARIES)
target_link_libraries(workrave-window-preview PRIVATE ${_preview_toolkit_libraries})
get_target_property(_preview_toolkit_includes ${_preview_toolkit} INCLUDE_DIRECTORIES)
target_include_directories(workrave-window-preview PRIVATE ${_preview_toolkit_includes})
get_target_property(_preview_app_libraries workrave LINK_LIBRARIES)
target_link_libraries(workrave-window-preview PRIVATE
  ${_preview_app_libraries} ${_preview_toolkit}
  workrave-libs-rpc workrave-libs-session gRPC::grpc++ protobuf::libprotobuf)

rpc_generate_source(${CMAKE_CURRENT_SOURCE_DIR}/WindowPreview.hh ${CMAKE_CURRENT_BINARY_DIR} RpcWindowPreview
  TARGET workrave-window-preview
  PREGENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gen
  PROTO_TYPES_PACKAGE workrave.preview
  GRPC_SERVICES_NAMESPACE rpc
  ADAPTER_NAMESPACE workrave::ui::rpc)
target_sources(workrave-window-preview PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/RpcWindowPreviewServiceImpl.cc
  ${CMAKE_CURRENT_BINARY_DIR}/RpcWindowPreview.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/RpcWindowPreview.grpc.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/RpcWindowPreviewTypes.pb.cc)

if (APPLE)
  get_target_property(_preview_info_plist_template workrave-window-preview MACOSX_BUNDLE_INFO_PLIST)
  if (NOT _preview_info_plist_template)
    set(_preview_info_plist_template "${CMAKE_ROOT}/Modules/MacOSXBundleInfo.plist.in")
  endif()
  file(READ "${_preview_info_plist_template}" _preview_info_plist)
  string(REPLACE "</dict>" "\t<key>LSUIElement</key>\n\t<true/>\n</dict>" _preview_info_plist "${_preview_info_plist}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/WindowPreviewInfo.plist.in" "${_preview_info_plist}")
  set_target_properties(workrave-window-preview PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER org.workrave.WindowPreview
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/WindowPreviewInfo.plist.in")
endif()
if (PLATFORM_OS_WINDOWS)
  target_sources(workrave-window-preview PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../resource.rc)
endif()

# Deliberately no install rule: release packages contain only Workrave.

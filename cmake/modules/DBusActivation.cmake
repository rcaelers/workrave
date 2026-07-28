# D-Bus activation-file support for the generated RPC bindings.

macro(dbus_add_activation_service SOURCE BINDIR)
  if (HAVE_DBUS)
    get_filename_component(_service_name ${SOURCE} NAME)
    string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_service_name})
    set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})

    get_filename_component(_service_file ${SOURCE} ABSOLUTE)
    set(workravebindir ${CMAKE_INSTALL_PREFIX}/${BINDIR})
    configure_file(${_service_file} ${_target})
    install(FILES ${_target} DESTINATION ${DATADIR}/dbus-1/services)
    unset(workravebindir)
  endif()
endmacro()

macro(dbus_generate NAME XML OUTFILE)
  add_custom_command(
    OUTPUT ${OUTFILE}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/libs/dbus/bin/dbusgen.py -s --backend=${DBUS_BACKEND} -l C++ ${XML} ${NAME}
        DEPENDS ${XML}
  )
endmacro()

macro(dbus_generate_with_backend NAME XML OUTFILE BACKEND)
  add_custom_command(
    OUTPUT ${OUTFILE}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/libs/dbus/bin/dbusgen.py -s --backend=${BACKEND} -l C++ ${XML} ${NAME}
    DEPENDS ${XML}
    )
  set_source_files_properties(${OUTFILE} PROPERTIES GENERATED TRUE)
endmacro()

macro(dbus_add_activation_service SOURCE)
  get_filename_component(_service_name ${SOURCE} NAME)
  string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_service_name})
  set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})

  get_filename_component(_service_file ${SOURCE} ABSOLUTE)
  set(workravebindir ${CMAKE_INSTALL_PREFIX}/${BINDIR})
  configure_file(${_service_file} ${_target})
  install(FILES ${_target} DESTINATION share/dbus-1/services)
  unset(workravebindir)
endmacro()

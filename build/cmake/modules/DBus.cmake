set (DBUSGEN ${CMAKE_SOURCE_DIR}/libs/dbus/bin/dbusgen.py)
set (TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/libs/dbus/data)

macro(dbus_generate_source XML DIRECTORY NAME)
  set (opt_args ${ARGN})

  list(LENGTH opt_args num_opt_args)
  if (${num_opt_args} GREATER 0)
    list(GET opt_args 0 backend)
  else()
    set(backend ${DBUS_BACKEND})
  endif ()

  set(cc_template "${TEMPLATE_DIR}/${backend}-cc.jinja")
  set(cc_output   "${DIRECTORY}/${NAME}.cc")

  set(hh_template "${TEMPLATE_DIR}/${backend}-hh.jinja")
  set(hh_output   "${DIRECTORY}/${NAME}.hh")

  add_custom_command(
    OUTPUT ${hh_output}
    COMMAND ${PYTHON_EXECUTABLE} ${DBUSGEN} ${XML} ${hh_template} ${hh_output}
    DEPENDS ${XML} ${hh_template}
    )

  add_custom_command(
    OUTPUT ${cc_output}
    COMMAND ${PYTHON_EXECUTABLE} ${DBUSGEN} ${XML} ${cc_template} ${cc_output}
    DEPENDS ${XML} ${cc_template} ${hh_output}
    )

  add_custom_target(
    ${NAME}_dbus_source_target ALL
    DEPENDS ${OUTFILE}
    )

  set_source_files_properties(${cc_output} PROPERTIES GENERATED TRUE)
  set_source_files_properties(${hh_output} PROPERTIES GENERATED TRUE)
endmacro()

macro(dbus_generate_xml XML DIRECTORY NAME)
  set(template "${TEMPLATE_DIR}/Xml-xml.jinja")
  set(output   "${DIRECTORY}/${NAME}.xml")

  add_custom_command(
    OUTPUT ${output}
    COMMAND ${PYTHON_EXECUTABLE} ${DBUSGEN} ${XML} ${template} ${output}
    DEPENDS ${XML} ${template}
    )

  add_custom_target(
    ${NAME}_dbus_xml_target ALL
    DEPENDS ${OUTFILE}
    )

  set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
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

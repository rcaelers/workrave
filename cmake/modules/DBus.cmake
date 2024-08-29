set(DBUSGEN ${CMAKE_SOURCE_DIR}/libs/dbus/bin/dbusgen.py)
set(TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/libs/dbus/data)
set(VENV_DIR ${CMAKE_BINARY_DIR}/venv-dbus)
set(REQUIREMENTS_FILE ${CMAKE_SOURCE_DIR}/libs/dbus/bin/requirements.txt)

if (NOT HAVE_JINJA2)
  add_custom_command(
    OUTPUT ${VENV_DIR}/bin/activate
    COMMAND ${CMAKE_COMMAND} -E remove_directory -rf ${VENV_DIR}
    COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}
    COMMAND ${VENV_DIR}/bin/pip install -r ${REQUIREMENTS_FILE}
    DEPENDS ${REQUIREMENTS_FILE}
  )
  add_custom_target(venv DEPENDS ${VENV_DIR}/bin/activate)
  set(PYTHON_EXECUTABLE ${VENV_DIR}/bin/python)
else()
  add_custom_target(venv)
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
endif()


macro(dbus_generate_source XML DIRECTORY NAME)
  if (HAVE_DBUS)
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
      DEPENDS ${XML} ${hh_template} venv
      )

    add_custom_command(
      OUTPUT ${cc_output}
      COMMAND ${PYTHON_EXECUTABLE} ${DBUSGEN} ${XML} ${cc_template} ${cc_output}
      DEPENDS ${XML} ${cc_template} ${hh_output} venv
      )

    add_custom_target(
      ${NAME}_dbus_source_target ALL
      DEPENDS ${cc_output} ${hh_output}
      )

    set_source_files_properties(${cc_output} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${hh_output} PROPERTIES GENERATED TRUE)
  endif()
endmacro()

macro(dbus_generate_xml XML DIRECTORY NAME)
  if (HAVE_DBUS)
    set(template "${TEMPLATE_DIR}/Xml-xml.jinja")
    set(output   "${DIRECTORY}/${NAME}.xml")

    add_custom_command(
      OUTPUT ${output}
      COMMAND ${PYTHON_EXECUTABLE} ${DBUSGEN} ${XML} ${template} ${output}
      DEPENDS ${XML} ${template} ${VENV_DIR}/bin/activate
      )

    add_custom_target(
      ${NAME}_dbus_xml_target ALL
      DEPENDS ${OUTFILE}
      )

    set_source_files_properties(${output} PROPERTIES GENERATED TRUE)
  endif()
endmacro()

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

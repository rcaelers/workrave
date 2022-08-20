find_program(GDBUS_CODEGEN NAMES gdbus-codegen DOC "gdbus-codegen executable")

macro(gdbus_codegen outfiles name prefix namespace service_xml)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h" "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND "${GDBUS_CODEGEN}"
        --interface-prefix "${prefix}"
	      --c-namespace ${namespace}
	      --c-generate-object-manager
        --generate-c-code "${name}"
        "${service_xml}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${ARGN} "${service_xml}"
  )
  list(APPEND ${outfiles} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
endmacro()

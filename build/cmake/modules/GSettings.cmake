# GSettings.cmake
# Originally based on CMake macros written for Marlin
# Updated by Yorba for newer versions of GLib.
# Updated for Workrave.

option(GSETTINGS_COMPILE "Compile GSettings schemas. Can be disabled for packaging reasons." ON)

# Find the GLib path for schema installation
execute_process(
  COMMAND ${PKG_CONFIG_EXECUTABLE} glib-2.0 --variable prefix
  OUTPUT_VARIABLE _glib_prefix
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# Fetch path for schema compiler from pkg-config
execute_process(
  COMMAND ${PKG_CONFIG_EXECUTABLE} gio-2.0 --variable glib_compile_schemas
  OUTPUT_VARIABLE _glib_compile_schemas
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

set(glib_schema_compiler ${_glib_compile_schemas} CACHE INTERNAL "")
  
if (LOCALINSTALL)
  set(GSETTINGS_DIR "share/glib-2.0/schemas/")
  set(GSETTINGS_ABS_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/")
else()
  set(GSETTINGS_DIR "${_glib_prefix}/share/glib-2.0/schemas/" CACHE INTERNAL "")
  set(GSETTINGS_ABS_DIR "${GSETTINGS}")
endif()

message(STATUS "GSettings schemas will be installed into ${GSETTINGS_DIR}")
if (GSETTINGS_COMPILE)
    message(STATUS "GSettings schemas will be compiled.")
endif ()
  
macro(gsettings_add_schemas GSETTINGS_TARGET SCHEMA_DIRECTORY)

  # Locate all schema files.
  file(GLOB all_schema_in_in_files
    "${SCHEMA_DIRECTORY}/*.gschema.xml.in.in"
    )

  foreach(schema_in_in_file ${all_schema_in_in_files})
    file(RELATIVE_PATH schema_in_file ${CMAKE_CURRENT_SOURCE_DIR} ${schema_in_in_file}) 
    set(schema_in_file "${CMAKE_CURRENT_BINARY_DIR}/${schema_in_file}")

    string(REGEX REPLACE ".in$" "" schema_in_file ${schema_in_file} )
    configure_file(${schema_in_in_file} ${schema_in_file})

    string(REGEX REPLACE ".in$" "" schema_file ${schema_in_file} )

    intltool_nomerge_xml(${schema_in_file} ${schema_file})

    get_filename_component(target ${schema_file} NAME)
    add_custom_target(generate_schema${target} ALL DEPENDS ${schema_file})

    install(
      FILES ${schema_file}
      DESTINATION ${GSETTINGS_DIR}
      OPTIONAL
    )
  endforeach()
  
  if (GSETTINGS_COMPILE)
    install(
      CODE "message (STATUS \"Compiling GSettings schemas\")"
      )
    
    install(
      CODE "execute_process (COMMAND ${glib_schema_compiler} ${GSETTINGS_ABS_DIR})"
      )
  endif()
endmacro()


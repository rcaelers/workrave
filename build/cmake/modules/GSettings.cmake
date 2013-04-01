# GSettings.cmake
# Originally based on CMake macros written for Marlin
# Updated by Yorba for newer versions of GLib.
#
# NOTE: This module does an in-place compilation of GSettings; the
#       resulting gschemas.compiled file will end up in the same
#       source folder as the original schema(s).

option(GSETTINGS_COMPILE "Compile GSettings schemas. Can be disabled for packaging reasons." ON)
option(GSETTINGS_COMPILE_IN_PLACE "Compile GSettings schemas in the build folder. This is used for running an appliction without installing the GSettings systemwide.  The application will need to set GSETTINGS_SCHEMA_DIR" ON)

option (GSETTINGS_LOCALINSTALL "Install GSettings Schemas locally instead of to the GLib prefix" ON)

if (GSETTINGS_COMPILE)
    message(STATUS "GSettings schemas will be compiled.")
endif ()

if (GSETTINGS_COMPILE_IN_PLACE)
    message(STATUS "GSettings schemas will be compiled in-place.")
endif ()

macro(gsettings_add_schemas GSETTINGS_TARGET SCHEMA_DIRECTORY)
    set(PKG_CONFIG_EXECUTABLE pkg-config)
    
    # Locate all schema files.
    file(GLOB all_schema_files
        "${SCHEMA_DIRECTORY}/*.gschema.xml"
    )
    
    # Find the GLib path for schema installation
    execute_process(
        COMMAND
            ${PKG_CONFIG_EXECUTABLE}
            glib-2.0
            --variable prefix
        OUTPUT_VARIABLE
            _glib_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if (GSETTINGS_LOCALINSTALL)
    set(GSETTINGS_DIR "share/glib-2.0/schemas/")
    set(GSETTINGS_ABS_DIR "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas/")
  else()
    set(GSETTINGS_DIR "${_glib_prefix}/share/glib-2.0/schemas/" CACHE INTERNAL "")
    set(GSETTINGS_ABS_DIR "${GSETTINGS}")
  endif()
    
    
    # Fetch path for schema compiler from pkg-config
    execute_process(
        COMMAND
            ${PKG_CONFIG_EXECUTABLE}
            gio-2.0
            --variable
            glib_compile_schemas
        OUTPUT_VARIABLE
            _glib_compile_schemas
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    set(glib_schema_compiler ${_glib_compile_schemas} CACHE INTERNAL "")
    
    if (GSETTINGS_COMPILE_IN_PLACE)
        set(COMPILE_IN_PLACE_DIR ${CMAKE_BINARY_DIR}/gsettings)
        add_custom_command(
            TARGET
                ${GSETTINGS_TARGET}
            COMMAND 
                ${CMAKE_COMMAND} -E make_directory "${COMPILE_IN_PLACE_DIR}"
        )
        
        # Copy all schemas to the build folder.
        foreach(schema_file ${all_schema_files})
            add_custom_command(
                TARGET
                    ${GSETTINGS_TARGET}
                COMMAND 
                    ${CMAKE_COMMAND} -E copy "${schema_file}" "${COMPILE_IN_PLACE_DIR}"
                COMMENT "Copying schema ${schema_file} to ${COMPILE_IN_PLACE_DIR}"
            )
        endforeach()
        
        # Compile schema in-place.
        add_custom_command(
            TARGET 
                ${GSETTINGS_TARGET}
            COMMAND
                ${glib_schema_compiler} ${COMPILE_IN_PLACE_DIR}
            COMMENT "Compiling schemas in folder: ${COMPILE_IN_PLACE_DIR}"
        )
    endif()
        
    # Install and recompile schemas
    message(STATUS "GSettings schemas will be installed into ${GSETTINGS_DIR}")
    
    install(
        FILES
            ${all_schema_files}
        DESTINATION
            ${GSETTINGS_DIR}
        OPTIONAL
    )
    
    if (GSETTINGS_COMPILE)
        install(
            CODE
                "message (STATUS \"Compiling GSettings schemas\")"
        )
        
        install(
            CODE
                "execute_process (COMMAND ${glib_schema_compiler} ${GSETTINGS_ABS_DIR})"
        )
    endif()
endmacro()


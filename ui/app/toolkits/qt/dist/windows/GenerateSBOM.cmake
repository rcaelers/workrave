if (NOT HAVE_SBOM)
  return()
endif()

message(STATUS "Generating SBOM...")

execute_process(
  COMMAND ${CMAKE_COMMAND} -E env
    BUILD_DIR=${BINARY_DIR}
    OUTPUT_DIR=${INSTALL_PATH}
    SOURCES_DIR=${SOURCE_DIR}
    ${BASH_CMD} ${SOURCE_DIR}/tools/local/sbom.sh
  RESULT_VARIABLE sbom_result
)

if (NOT sbom_result EQUAL 0)
  message(FATAL_ERROR "SBOM generation failed with exit code ${sbom_result}")
endif()

message(STATUS "SBOM generated: ${INSTALL_PATH}/sbom.spdx.json")

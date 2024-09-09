set(EXTERNAL_SBOM_FILE "${CMAKE_BINARY_DIR}/external-sbom.csv")

# "Package Name,Version,License,Description,URL"

function(FetchContent_Declare_Logged package_name package_description license)
  cmake_parse_arguments(
    FETCHCONTENT_ARG
    ""
    "GIT_REPOSITORY;GIT_TAG;URL;URL_HASH"
    ""
    ${ARGN}
  )

  FetchContent_Declare(${package_name} ${ARGN})

  if(FETCHCONTENT_ARG_URL MATCHES "https://www.nuget.org/api/v2/package/.*/(.*)$")
    set(version "${CMAKE_MATCH_1}")
    file(APPEND ${EXTERNAL_SBOM_FILE} "${package_name},${version},${license},${package_description},${FETCHCONTENT_ARG_URL}\n")
  elseif(FETCHCONTENT_ARG_GIT_REPOSITORY)
    file(APPEND ${EXTERNAL_SBOM_FILE} "${package_name},${FETCHCONTENT_ARG_GIT_TAG},${license},${package_description},${FETCHCONTENT_ARG_GIT_REPOSITORY}\n")
  elseif(FETCHCONTENT_ARG_URL)
    file(APPEND ${EXTERNAL_SBOM_FILE} "${package_name},Unknown,${license},${package_description},${FETCHCONTENT_ARG_URL}\n")
  else()
    message(WARNING "Could not determine package source for ${package_name}")
    file(APPEND ${EXTERNAL_SBOM_FILE} "${package_name},Unknown,${license},${package_description},Unknown\n")
  endif()

  message(STATUS "Added ${package_name} to SBOM with license: ${license}")
endfunction()


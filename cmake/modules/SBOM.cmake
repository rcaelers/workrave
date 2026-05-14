set(EXTERNAL_SBOM_FILE "${CMAKE_BINARY_DIR}/external-sbom.csv")

# "Package Name,Version,License,Description,URL"

# Track packages already added to SBOM by FetchContent_Declare_Logged
set_property(GLOBAL PROPERTY SBOM_DECLARED_PACKAGES "")

function(sbom_set_license name license)
  set_property(GLOBAL PROPERTY "SBOM_LICENSE_${name}" "${license}")
endfunction()

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

  set_property(GLOBAL APPEND PROPERTY SBOM_DECLARED_PACKAGES "${package_name}")
  sbom_set_license("${package_name}" "${license}")
  message(STATUS "SBOM: Added ${package_name} with license: ${license}")
endfunction()

# Resolve version from a git source directory via tag or commit hash
function(_sbom_resolve_git_version source_dir out_version)
  set(_version "Unknown")
  find_package(Git QUIET)
  if(Git_FOUND AND IS_DIRECTORY "${source_dir}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match
      WORKING_DIRECTORY "${source_dir}"
      OUTPUT_VARIABLE _tag
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _tag_result
    )
    if(_tag_result EQUAL 0 AND _tag)
      set(_version "${_tag}")
    else()
      execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${source_dir}"
        OUTPUT_VARIABLE _hash
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _hash_result
      )
      if(_hash_result EQUAL 0 AND _hash)
        set(_version "${_hash}")
      endif()
    endif()
  endif()
  set(${out_version} "${_version}" PARENT_SCOPE)
endfunction()

# Get the remote origin URL from a git source directory
function(_sbom_get_git_url source_dir out_url)
  set(_url "Unknown")
  find_package(Git QUIET)
  if(Git_FOUND AND IS_DIRECTORY "${source_dir}")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} remote get-url origin
      WORKING_DIRECTORY "${source_dir}"
      OUTPUT_VARIABLE _remote_url
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
      RESULT_VARIABLE _result
    )
    if(_result EQUAL 0 AND _remote_url)
      set(_url "${_remote_url}")
    endif()
  endif()
  set(${out_url} "${_url}" PARENT_SCOPE)
endfunction()

# Process git submodules in a source directory and add them to the SBOM.
# Uses per-submodule license from sbom_set_license, falling back to parent license.
function(_sbom_process_submodules source_dir)
  set(_gitmodules "${source_dir}/.gitmodules")
  if(NOT EXISTS "${_gitmodules}")
    return()
  endif()

  find_package(Git QUIET)
  if(NOT Git_FOUND)
    return()
  endif()

  # Determine parent package license for fallback
  get_filename_component(_parent_dir "${source_dir}" NAME)
  string(REGEX REPLACE "-src$" "" _parent_name "${_parent_dir}")
  get_property(_has_parent_license GLOBAL PROPERTY "SBOM_LICENSE_${_parent_name}" SET)
  if(_has_parent_license)
    get_property(_parent_license GLOBAL PROPERTY "SBOM_LICENSE_${_parent_name}")
  else()
    set(_parent_license "Unknown")
  endif()

  file(READ "${_gitmodules}" _content)
  string(REGEX MATCHALL "\\[submodule[^\n]*\n([^\[]*)" _blocks "${_content}")

  foreach(_block ${_blocks})
    set(_path "")
    set(_url "")

    if(_block MATCHES "path[ \t]*=[ \t]*([^\n]+)")
      string(STRIP "${CMAKE_MATCH_1}" _path)
    endif()
    if(_block MATCHES "url[ \t]*=[ \t]*([^\n]+)")
      string(STRIP "${CMAKE_MATCH_1}" _url)
    endif()

    if(NOT _path OR NOT _url)
      continue()
    endif()

    get_filename_component(_name "${_path}" NAME)
    set(_submodule_dir "${source_dir}/${_path}")

    get_property(_has_license GLOBAL PROPERTY "SBOM_LICENSE_${_name}" SET)
    if(_has_license)
      get_property(_license GLOBAL PROPERTY "SBOM_LICENSE_${_name}")
    else()
      set(_license "${_parent_license}")
      message(WARNING "SBOM: No license for submodule ${_name}, using parent (${_parent_name}) license: ${_license}")
    endif()

    _sbom_resolve_git_version("${_submodule_dir}" _version)

    file(APPEND ${EXTERNAL_SBOM_FILE} "${_name},${_version},${_license},${_name} (submodule),${_url}\n")
    message(STATUS "SBOM: Discovered submodule ${_name} (${_version}) with license: ${_license}")
  endforeach()
endfunction()

# Scan FETCHCONTENT_BASE_DIR for all fetched packages. Adds undeclared packages
# (sub-dependencies) that have a license defined via sbom_set_license().
# Also discovers git submodules in all fetched packages.
function(sbom_discover_packages)
  get_property(_declared GLOBAL PROPERTY SBOM_DECLARED_PACKAGES)

  file(GLOB _src_dirs "${FETCHCONTENT_BASE_DIR}/*-src")

  foreach(_src_dir ${_src_dirs})
    get_filename_component(_dir_name "${_src_dir}" NAME)
    string(REGEX REPLACE "-src$" "" _pkg_name "${_dir_name}")

    list(FIND _declared "${_pkg_name}" _idx)
    if(NOT _idx EQUAL -1)
      # Already declared via FetchContent_Declare_Logged, just check submodules
      _sbom_process_submodules("${_src_dir}")
      continue()
    endif()

    # Undeclared package: look up license
    get_property(_has_license GLOBAL PROPERTY "SBOM_LICENSE_${_pkg_name}" SET)
    if(NOT _has_license)
      message(STATUS "SBOM: Skipping ${_pkg_name} (no license defined)")
      continue()
    endif()
    get_property(_license GLOBAL PROPERTY "SBOM_LICENSE_${_pkg_name}")

    _sbom_resolve_git_version("${_src_dir}" _version)
    _sbom_get_git_url("${_src_dir}" _url)

    file(APPEND ${EXTERNAL_SBOM_FILE} "${_pkg_name},${_version},${_license},${_pkg_name},${_url}\n")
    message(STATUS "SBOM: Discovered ${_pkg_name} (${_version}) with license: ${_license}")

    _sbom_process_submodules("${_src_dir}")
  endforeach()
endfunction()


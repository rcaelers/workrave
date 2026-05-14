#
# Copyright (C) 2018-2025 by George Cave - gcave@stablecoder.ca
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

# USAGE: To enable any code coverage instrumentation/targets, the single CMake
# option of `CODE_COVERAGE` needs to be set to 'ON', either by GUI, ccmake, or
# on the command line.
#
# From this point, there are two primary methods for adding instrumentation to
# targets: 1 - A blanket instrumentation by calling `add_code_coverage()`, where
# all targets in that directory and all subdirectories are automatically
# instrumented. 2 - Per-target instrumentation by calling
# `target_code_coverage(<TARGET_NAME>)`, where the target is given and thus only
# that target is instrumented. This applies to both libraries and executables.
#
# To add coverage targets, such as calling `make ccov` to generate the actual
# coverage information for perusal or consumption, call
# `target_code_coverage(<TARGET_NAME>)` on an *executable* target.
#
# Example 1: All targets instrumented
#
# In this case, the coverage information reported will will be that of the
# `theLib` library target and `theExe` executable.
#
# 1a: Via global command
#
# ~~~
# add_code_coverage() # Adds instrumentation to all targets
#
# add_library(theLib lib.cpp)
#
# add_executable(theExe main.cpp)
# target_link_libraries(theExe PRIVATE theLib)
# target_code_coverage(theExe) # As an executable target, adds the 'ccov-theExe' target (instrumentation already added via global anyways) for generating code coverage reports.
# ~~~
#
# 1b: Via target commands
#
# ~~~
# add_library(theLib lib.cpp)
# target_code_coverage(theLib) # As a library target, adds coverage instrumentation but no targets.
#
# add_executable(theExe main.cpp)
# target_link_libraries(theExe PRIVATE theLib)
# target_code_coverage(theExe) # As an executable target, adds the 'ccov-theExe' target and instrumentation for generating code coverage reports.
# ~~~
#
# Example 2: Target instrumented, but with regex pattern of files to be excluded
# from report
#
# ~~~
# add_executable(theExe main.cpp non_covered.cpp)
# target_code_coverage(theExe
#     EXCLUDE non_covered.cpp
#     LCOV_EXCLUDE test/*
#     LLVM_EXCLUDE test/.*) # As an executable target, the reports will exclude the non-covered.cpp file, and any files in a test/ folder.
# ~~~
#
# Example 3: Target added to the 'ccov' and 'ccov-all' targets
#
# ~~~
# add_code_coverage_all_targets(
#     LCOV_EXCLUDE test/*
#     LLVM_EXCLUDE test/.*)# Adds the 'ccov-all' target set and sets it to exclude all files in test/ folders.
#
# add_executable(theExe main.cpp non_covered.cpp)
# target_code_coverage(theExe AUTO ALL EXCLUDE non_covered.cpp test/*) # As an executable target, adds to the 'ccov' and ccov-all' targets, and the reports will exclude the non-covered.cpp file, and any files in a test/ folder.
# ~~~
#
# Example 4: Hook all targets
#
# ~~~
# set(CCOV_TARGETS_HOOK ON) # enable 'add_executable' and 'add_library' hooks
# set(CCOV_TARGETS_HOOK_ARGS ALL AUTO) # set default arguments for coverage
#
# add_code_coverage() # Adds instrumentation to all targets
#
# add_library(theLib lib.cpp) # ccov-theLib target will be add
#
# add_executable(theExe main.cpp) # ccov-theExe target will be add
# target_link_libraries(theExe PRIVATE theLib)
# ~~~

# Options
option(
  CODE_COVERAGE
  "Builds targets with code coverage instrumentation. (Requires GCC or Clang)"
  OFF)

option(CCOV_TARGETS_HOOK "Autocapture all new targets." OFF)

option(CCOV_TARGETS_HOOK_ARGS "Default arguments for all hooked targets.")

# Programs
find_program(LLVM_COV_PATH llvm-cov)
find_program(LLVM_PROFDATA_PATH llvm-profdata)
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)
# Hide behind the 'advanced' mode flag for GUI/ccmake
mark_as_advanced(FORCE LLVM_COV_PATH LLVM_PROFDATA_PATH LCOV_PATH GENHTML_PATH)

# Variables
set(CMAKE_COVERAGE_DATA_DIRECTORY ${CMAKE_BINARY_DIR}/ccov-data)
set(CMAKE_COVERAGE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/ccov)

# Common initialization/checks
if(CODE_COVERAGE AND NOT CODE_COVERAGE_ADDED)
  set(CODE_COVERAGE_ADDED ON)

  # Common Targets
  file(MAKE_DIRECTORY ${CMAKE_COVERAGE_DATA_DIRECTORY})
  file(MAKE_DIRECTORY ${CMAKE_COVERAGE_OUTPUT_DIRECTORY})

  if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
     OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")

    # to create lists of linked objects to avoid multi-process file handle
    # issues
    file(MAKE_DIRECTORY ${CMAKE_COVERAGE_DATA_DIRECTORY}/objects)
    # to create lists of raw profile data while avoiding multi-process file
    # handle issues
    file(MAKE_DIRECTORY ${CMAKE_COVERAGE_DATA_DIRECTORY}/profraw)

    if(CMAKE_C_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID
                                                   MATCHES "AppleClang")
      # When on macOS and using the Apple-provided toolchain, use the
      # XCode-provided llvm toolchain via `xcrun`
      message(
        STATUS
          "Building with XCode-provided llvm code coverage tools (via `xcrun`)")
      set(LLVM_COV_PATH xcrun llvm-cov)
      set(LLVM_PROFDATA_PATH xcrun llvm-profdata)
    else()
      # Use the regular llvm toolchain
      message(STATUS "Building with llvm code coverage tools")
    endif()

    if(NOT LLVM_COV_PATH)
      message(FATAL_ERROR "llvm-cov not found! Aborting.")
    else()
      # Version number checking for 'EXCLUDE' compatibility
      execute_process(COMMAND ${LLVM_COV_PATH} --version
                      OUTPUT_VARIABLE LLVM_COV_VERSION_CALL_OUTPUT)
      string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" LLVM_COV_VERSION
                   ${LLVM_COV_VERSION_CALL_OUTPUT})

      if(LLVM_COV_VERSION VERSION_LESS "7.0.0")
        message(
          WARNING
            "target_code_coverage()/add_code_coverage_all_targets() 'EXCLUDE' option only available on llvm-cov >= 7.0.0"
        )
      endif()
    endif()

    # Targets
    add_custom_target(ccov-clean)

  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")
    # Messages
    message(STATUS "Building with lcov Code Coverage Tools")

    if(NOT LCOV_PATH)
      message(FATAL_ERROR "lcov not found! Aborting...")
    endif()
    if(NOT GENHTML_PATH)
      message(FATAL_ERROR "genhtml not found! Aborting...")
    endif()

    # Targets
    add_custom_target(ccov-clean COMMAND ${LCOV_PATH} --directory
                                         ${CMAKE_BINARY_DIR} --zerocounters)

  else()
    message(FATAL_ERROR "Code coverage requires Clang or GCC. Aborting.")
  endif()

  if(CCOV_TARGETS_HOOK)
    if(COMMAND _add_executable)
      message(
        FATAL_ERROR
          "add_executable was already redefined. Only one redefinitions is allowed."
      )
    endif()
    macro(add_executable)
      _add_executable(${ARGV})
      target_code_coverage(${ARGV0} ${CCOV_TARGETS_HOOK_ARGS})
    endmacro(add_executable)

    if(COMMAND _add_library)
      message(
        FATAL_ERROR
          "add_library was already redefined. Only one redefinitions is allowed."
      )
    endif()
    macro(add_library)
      _add_library(${ARGV})
      target_code_coverage(${ARGV0} ${CCOV_TARGETS_HOOK_ARGS})
    endmacro(add_library)
  endif(CCOV_TARGETS_HOOK)

endif()

# Adds code coverage instrumentation to a library, or instrumentation/targets
# for an executable target.
# ~~~
# Targets added (executables only):
# ccov-run-${TARGET_NAME} : Re-runs the executable, collecting fresh coverage data
# ccov-html-${TARGET_NAME} : Generates HTML code coverage report for the associated named target.
# ccov-html : Generates HTML code coverage report for every target added with 'AUTO' parameter.
# ccov-${TARGET_NAME} : Generates HTML code coverage report for the associated named target. (same as ccov-html-${TARGET_NAME})
# ccov : Generates HTML code coverage report for every target added with 'AUTO' parameter. (same as ccov-html)
#
# LLVM-based coverage targets added (executables only):
# ccov-report-${TARGET_NAME} : Prints to command line summary per-file coverage information.
# ccov-export-${TARGET_NAME} : Exports the coverage report to a JSON file.
# ccov-show-${TARGET_NAME} : Prints to command line detailed per-line coverage information.
# ccov-report : Generates HTML code coverage report for every target added with 'AUTO' parameter.
#
# Required Parameters:
# TARGET_NAME - Name of the target to generate code coverage for.
#
# Optional Parameters:
# PUBLIC - Sets the visibility for added compile options to targets to PUBLIC instead of the default of PRIVATE.
# INTERFACE - Sets the visibility for added compile options to targets to INTERFACE instead of the default of PRIVATE.
# PLAIN - Do not set any target visibility (backward compatibility with old cmake projects)
# AUTO - Adds the target to the 'ccov' target so that it can be run in a batch with others easily. Effective on executable targets.
# ALL - Adds the target to the 'ccov-all-*' targets created by a prior call to `add_code_coverage_all_targets` Effective on executable targets.
# EXTERNAL - For GCC's lcov, allows the profiling of 'external' files from the processing directory
# COVERAGE_TARGET_NAME - For executables ONLY, changes the outgoing target name so instead of `ccov-${TARGET_NAME}` it becomes `ccov-${COVERAGE_TARGET_NAME}`.
# OBJECTS <TARGETS> - For executables ONLY, if the provided targets are static or shared libraries, adds coverage information to the output
# PRE_ARGS <ARGUMENTS> - For executables ONLY, prefixes given arguments to the associated ccov-run-${TARGET_NAME} executable call ($<PRE_ARGS> ccov-*)
# ARGS <ARGUMENTS> - For executables ONLY, appends the given arguments to the associated ccov-run-${TARGET_NAME} executable call (ccov-* $<ARGS>)
# EXCLUDE <PATTERNS> - Excludes files of the patterns provided from coverage. Added to any LLVM/LCOV specified excludes. (These do not copy to the 'all' targets)
#
# Optional Parameters effective with the clang/LLVM backend:
# LLVM_EXCLUDE <PATTERNS> - Excludes files that match the provided patterns, LLVM excludes by regex patterns.
# LLVM_PROFDATA_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-profdata` call that merges/processes raw profile data. (.profraw -> .profdata)
# LLVM_COV_SHOW_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov show` call for the `ccov-show-${TARGET_NAME}` target.
# LLVM_COV_REPORT_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov report` call for the `ccov-report-${TARGET_NAME}` target.
# LLVM_COV_EXPORT_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov export` call for the `ccov-export-${TARGET_NAME}` target.
# LLVM_COV_HTML_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov show -format="html"` call for the `ccov-html-${TARGET_NAME}`/`ccov-${TARGET_NAME}` targets.
#
# Optional Parameters effective with the GCC/lcov backend:
# LCOV_EXCLUDE <PATTERNS> - Excludes files that match the provided patterns. LCOV exclude by glob patterns.
# LCOV_OPTIONS <OPTIONS> - Options are passed verbatim to the `lcov` call when capturing/filtering capture data
# GENHTML_OPTIONS <OPTIONS> - Options are passed verbatim to the `genhtml` call when generating the HTML report from lcov data for the `ccov-html-${TARGET_NAME}`/`ccov-${TARGET_NAME}` targets.
# ~~~
function(target_code_coverage TARGET_NAME)
  if(NOT CODE_COVERAGE)
    return()
  endif()

  # Argument parsing
  set(options AUTO ALL EXTERNAL PUBLIC INTERFACE PLAIN)
  set(single_value_keywords COVERAGE_TARGET_NAME)
  set(multi_value_keywords
      # common
      EXCLUDE
      OBJECTS
      PRE_ARGS
      ARGS
      # llvm
      LLVM_EXCLUDE
      LLVM_PROFDATA_OPTIONS
      LLVM_COV_SHOW_OPTIONS
      LLVM_COV_REPORT_OPTIONS
      LLVM_COV_EXPORT_OPTIONS
      LLVM_COV_HTML_OPTIONS
      # lcov
      LCOV_EXCLUDE
      LCOV_OPTIONS
      GENHTML_OPTIONS)
  cmake_parse_arguments(
    target_code_coverage "${options}" "${single_value_keywords}"
    "${multi_value_keywords}" ${ARGN})

  # Set the visibility of target functions to PUBLIC, INTERFACE or default to
  # PRIVATE.
  if(target_code_coverage_PUBLIC)
    set(TARGET_VISIBILITY PUBLIC)
    set(TARGET_LINK_VISIBILITY PUBLIC)
  elseif(target_code_coverage_INTERFACE)
    set(TARGET_VISIBILITY INTERFACE)
    set(TARGET_LINK_VISIBILITY INTERFACE)
  elseif(target_code_coverage_PLAIN)
    set(TARGET_VISIBILITY PUBLIC)
    set(TARGET_LINK_VISIBILITY)
  else()
    set(TARGET_VISIBILITY PRIVATE)
    set(TARGET_LINK_VISIBILITY PRIVATE)
  endif()

  if(NOT target_code_coverage_COVERAGE_TARGET_NAME)
    # If a specific name was given, use that instead.
    set(target_code_coverage_COVERAGE_TARGET_NAME ${TARGET_NAME})
  endif()

  # Add code coverage instrumentation to the target's linker command
  if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
     OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    target_compile_options(${TARGET_NAME} ${TARGET_VISIBILITY}
                           -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(${TARGET_NAME} ${TARGET_VISIBILITY}
                        -fprofile-instr-generate -fcoverage-mapping)
  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")
    target_compile_options(
      ${TARGET_NAME} ${TARGET_VISIBILITY} -fprofile-arcs -ftest-coverage
      $<$<COMPILE_LANGUAGE:CXX>:-fno-elide-constructors> -fno-default-inline)
    target_link_libraries(${TARGET_NAME} ${TARGET_LINK_VISIBILITY} gcov)
  endif()

  # Targets
  get_target_property(target_type ${TARGET_NAME} TYPE)

  # For executables add targets to run and produce output
  if(target_type STREQUAL "EXECUTABLE")
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
       OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")

      # If there are static or shared objects to also work with, generate the
      # string to add them here
      foreach(LINK_OBJECT ${target_code_coverage_OBJECTS})
        # Check to see if the target is a shared object
        if(TARGET ${LINK_OBJECT})
          get_target_property(LINK_OBJECT_TYPE ${LINK_OBJECT} TYPE)
          if(${LINK_OBJECT_TYPE} STREQUAL "STATIC_LIBRARY"
             OR ${LINK_OBJECT_TYPE} STREQUAL "SHARED_LIBRARY")
            set(LINKED_OBJECTS ${LINKED_OBJECTS}
                               -object=$<TARGET_FILE:${LINK_OBJECT}>)
          endif()
        endif()
      endforeach()

      if(${CMAKE_VERSION} VERSION_LESS "3.17.0")
        add_custom_target(
          ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME}
          COMMAND ${CMAKE_COMMAND} -E remove -f
                  ${target_code_coverage_COVERAGE_TARGET_NAME}.profraw)
      else()
        add_custom_target(
          ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME}
          COMMAND ${CMAKE_COMMAND} -E rm -f
                  ${target_code_coverage_COVERAGE_TARGET_NAME}.profraw)
      endif()
      add_dependencies(ccov-clean
                       ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Run the executable, generating raw profile data Make the run data
      # available for further processing.
      add_custom_command(
        OUTPUT ${target_code_coverage_COVERAGE_TARGET_NAME}.profraw
        COMMAND
          ${CMAKE_COMMAND} -E env ${CMAKE_CROSSCOMPILING_EMULATOR}
          ${target_code_coverage_PRE_ARGS}
          LLVM_PROFILE_FILE=${target_code_coverage_COVERAGE_TARGET_NAME}.profraw
          $<TARGET_FILE:${TARGET_NAME}> ${target_code_coverage_ARGS}
        COMMAND
          ${CMAKE_COMMAND} -E echo "-object=$<TARGET_FILE:${TARGET_NAME}>"
          ${LINKED_OBJECTS} >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/objects/${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${CMAKE_COMMAND} -E echo
          "${CMAKE_CURRENT_BINARY_DIR}/${target_code_coverage_COVERAGE_TARGET_NAME}.profraw"
          >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/profraw/${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E rm -f
                ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
        DEPENDS ${TARGET_NAME})

      # This is a copy of the above add_custom_command.
      #
      # Since add_custom_target items are always considered out-of-date, this
      # can be used by the user to perform another coverage run, but only when
      # invoked directly.
      add_custom_target(
        ccov-run-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${CMAKE_COMMAND} -E env ${CMAKE_CROSSCOMPILING_EMULATOR}
          ${target_code_coverage_PRE_ARGS}
          LLVM_PROFILE_FILE=${target_code_coverage_COVERAGE_TARGET_NAME}.profraw
          $<TARGET_FILE:${TARGET_NAME}> ${target_code_coverage_ARGS}
        COMMAND
          ${CMAKE_COMMAND} -E echo "-object=$<TARGET_FILE:${TARGET_NAME}>"
          ${LINKED_OBJECTS} >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/objects/${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${CMAKE_COMMAND} -E echo
          "${CMAKE_CURRENT_BINARY_DIR}/${target_code_coverage_COVERAGE_TARGET_NAME}.profraw"
          >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/profraw/${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E rm -f
                ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
        DEPENDS ${TARGET_NAME})

      # As custom targets with COMMANDs are always considered out of date, we
      # want the merged/all targets to depend on this, so that we don't
      # necessarily re-run the executbale every time, only at a minimum to
      # generate the file. If the user want to re-run targets, they can by
      # explicitly invoking the ccov-run-TARGET targets.
      #
      # If the option for a hidden target were possible, this would be.
      add_custom_target(
        ccov-profraw-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ${target_code_coverage_COVERAGE_TARGET_NAME}.profraw)

      # Merge the generated profile data so llvm-cov can process it
      add_custom_command(
        OUTPUT ${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
        COMMAND
          ${LLVM_PROFDATA_PATH} merge
          ${target_code_coverage_LLVM_PROFDATA_OPTIONS} -sparse
          ${target_code_coverage_COVERAGE_TARGET_NAME}.profraw -o
          ${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
        DEPENDS ccov-profraw-${target_code_coverage_COVERAGE_TARGET_NAME})
      add_custom_target(
        ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ${target_code_coverage_COVERAGE_TARGET_NAME}.profdata)

      # Ignore regex only works on LLVM >= 7
      set(EXCLUDE_REGEX)
      if(LLVM_COV_VERSION VERSION_GREATER_EQUAL "7.0.0")
        foreach(EXCLUDE_ITEM ${target_code_coverage_EXCLUDE})
          set(EXCLUDE_REGEX ${EXCLUDE_REGEX}
                            -ignore-filename-regex='${EXCLUDE_ITEM}')
        endforeach()
        foreach(EXCLUDE_ITEM ${target_code_coverage_LLVM_EXCLUDE})
          set(EXCLUDE_REGEX ${EXCLUDE_REGEX}
                            -ignore-filename-regex='${EXCLUDE_ITEM}')
        endforeach()
      endif()

      # Print out details of the coverage information to the command line
      add_custom_target(
        ccov-show-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${LLVM_COV_PATH} show $<TARGET_FILE:${TARGET_NAME}>
          -instr-profile=${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
          -show-line-counts-or-regions ${LINKED_OBJECTS} ${EXCLUDE_REGEX}
          ${target_code_coverage_LLVM_COV_SHOW_OPTIONS}
        DEPENDS ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Print out a summary of the coverage information to the command line
      add_custom_target(
        ccov-report-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${LLVM_COV_PATH} report $<TARGET_FILE:${TARGET_NAME}>
          -instr-profile=${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
          ${LINKED_OBJECTS} ${EXCLUDE_REGEX}
          ${target_code_coverage_LLVM_COV_REPORT_OPTIONS}
        DEPENDS ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Export coverage information so continuous integration tools (e.g.
      # Jenkins) can consume it
      add_custom_target(
        ccov-export-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${LLVM_COV_PATH} export $<TARGET_FILE:${TARGET_NAME}>
          -instr-profile=${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
          -format="text" ${LINKED_OBJECTS} ${EXCLUDE_REGEX}
          ${target_code_coverage_LLVM_COV_EXPORT_OPTIONS} >
          ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/${target_code_coverage_COVERAGE_TARGET_NAME}.json
        DEPENDS ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Only generates HTML output of the coverage information for perusal
      add_custom_target(
        ccov-html-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${LLVM_COV_PATH} show $<TARGET_FILE:${TARGET_NAME}>
          -instr-profile=${target_code_coverage_COVERAGE_TARGET_NAME}.profdata
          -show-line-counts-or-regions
          -output-dir=${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/${target_code_coverage_COVERAGE_TARGET_NAME}
          -format="html" ${LINKED_OBJECTS} ${EXCLUDE_REGEX}
          ${target_code_coverage_LLVM_COV_HTML_OPTIONS}
        DEPENDS ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Generates HTML output of the coverage information for perusal
      add_custom_target(
        ccov-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ccov-html-${target_code_coverage_COVERAGE_TARGET_NAME})

    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                "GNU")
      set(COVERAGE_INFO
          "${CMAKE_COVERAGE_DATA_DIRECTORY}/${target_code_coverage_COVERAGE_TARGET_NAME}.info"
      )

      # Generate exclusion string for use
      set(EXCLUDE_REGEX)
      foreach(EXCLUDE_ITEM ${target_code_coverage_EXCLUDE})
        set(EXCLUDE_REGEX ${EXCLUDE_REGEX} --remove ${COVERAGE_INFO}
                          '${EXCLUDE_ITEM}')
      endforeach()
      foreach(EXCLUDE_ITEM ${target_code_coverage_LCOV_EXCLUDE})
        set(EXCLUDE_REGEX ${EXCLUDE_REGEX} --remove ${COVERAGE_INFO}
                          '${EXCLUDE_ITEM}')
      endforeach()

      if(EXCLUDE_REGEX)
        set(EXCLUDE_COMMAND ${LCOV_PATH} ${EXCLUDE_REGEX} --output-file
                            ${COVERAGE_INFO})
      else()
        set(EXCLUDE_COMMAND ;)
      endif()

      if(NOT ${target_code_coverage_EXTERNAL})
        set(EXTERNAL_OPTION --no-external)
      endif()

      # Capture coverage data
      if(${CMAKE_VERSION} VERSION_LESS "3.17.0")
        add_custom_target(
          ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME}
          COMMAND ${CMAKE_COMMAND} -E remove -f ${COVERAGE_INFO}
                  ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run
          COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters)
      else()
        add_custom_target(
          ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME}
          COMMAND ${CMAKE_COMMAND} -E rm -f ${COVERAGE_INFO}
                  ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run
          COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters)
      endif()
      add_dependencies(ccov-clean
                       ccov-clean-${target_code_coverage_COVERAGE_TARGET_NAME})

      # Run the executable, generating raw profile data Make the run data
      # available for further processing.
      add_custom_command(
        OUTPUT ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run
        COMMAND
          ${CMAKE_CROSSCOMPILING_EMULATOR} ${target_code_coverage_PRE_ARGS}
          $<TARGET_FILE:${TARGET_NAME}> ${target_code_coverage_ARGS}
        COMMAND # add a dummy file to use as a dependency to indicate the target
                # has been run and data collected
                ${CMAKE_COMMAND} -E touch
                ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run
        DEPENDS ${TARGET_NAME})

      # This is a copy of the above add_custom_command.
      #
      # Since add_custom_target items are always considered out-of-date, this
      # can be used by the user to perform another coverage run, but only when
      # invoked directly.
      add_custom_target(
        ccov-run-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${CMAKE_CROSSCOMPILING_EMULATOR} ${target_code_coverage_PRE_ARGS}
          $<TARGET_FILE:${TARGET_NAME}> ${target_code_coverage_ARGS}
        COMMAND # add a dummy file to use as a dependency to indicate the target
                # has been run and data collected
                ${CMAKE_COMMAND} -E touch
                ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run
        DEPENDS ${TARGET_NAME})

      # As custom targets with COMMANDs are always considered out of date, we
      # want the merged/all targets to depend on this, so that we don't
      # necessarily re-run the executbale every time, only at a minimum to
      # generate the file. If the user want to re-run targets, they can by
      # explicitly invoking the ccov-run-TARGET targets.
      #
      # If the option for a hidden target were possible, this would be.
      add_custom_target(
        ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run)

      add_custom_command(
        OUTPUT ${COVERAGE_INFO}
        COMMAND
          ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --base-directory
          ${CMAKE_SOURCE_DIR} --capture ${EXTERNAL_OPTION} --output-file
          ${COVERAGE_INFO} ${target_code_coverage_LCOV_OPTIONS}
        COMMAND ${EXCLUDE_COMMAND}
        DEPENDS ${target_code_coverage_COVERAGE_TARGET_NAME}.ccov-run)
      add_custom_target(
        ccov-capture-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ${COVERAGE_INFO})

      # Only generates HTML output of the coverage information for perusal
      add_custom_target(
        ccov-html-${target_code_coverage_COVERAGE_TARGET_NAME}
        COMMAND
          ${GENHTML_PATH} ${target_code_coverage_GENHTML_OPTIONS} -o
          ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/${target_code_coverage_COVERAGE_TARGET_NAME}
          ${COVERAGE_INFO}
        COMMAND
          ${CMAKE_COMMAND} -E echo
          "Open ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/${target_code_coverage_COVERAGE_TARGET_NAME}/index.html in your browser to view the coverage report."
        DEPENDS ${COVERAGE_INFO})

      # Generates HTML output of the coverage information for perusal
      add_custom_target(
        ccov-${target_code_coverage_COVERAGE_TARGET_NAME}
        DEPENDS ccov-html-${target_code_coverage_COVERAGE_TARGET_NAME})
    endif()

    # AUTO
    if(target_code_coverage_AUTO)
      if(NOT TARGET ccov)
        add_custom_target(ccov)
      endif()
      add_dependencies(ccov ccov-${target_code_coverage_COVERAGE_TARGET_NAME})

      if(NOT TARGET ccov-html)
        add_custom_target(ccov-html)
      endif()
      add_dependencies(ccov-html
                       ccov-html-${target_code_coverage_COVERAGE_TARGET_NAME})

      if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU" AND NOT CMAKE_CXX_COMPILER_ID
                                                   MATCHES "GNU")
        if(NOT TARGET ccov-report)
          add_custom_target(ccov-report)
        endif()
        add_dependencies(
          ccov-report ccov-report-${target_code_coverage_COVERAGE_TARGET_NAME})
      endif()
    endif()

    # ALL
    if(target_code_coverage_ALL)
      if(NOT TARGET ccov-all-run)
        message(
          FATAL_ERROR
            "Calling target_code_coverage with 'ALL' must be after a call to 'add_code_coverage_all_targets' to create the 'ccov-all' target set."
        )
      endif()

      add_dependencies(ccov-all-run
                       ccov-run-${target_code_coverage_COVERAGE_TARGET_NAME})

      if(CMAKE_C_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_ID MATCHES
                                               "GNU")
        add_dependencies(
          ccov-all-ran
          ccov-profiledata-${target_code_coverage_COVERAGE_TARGET_NAME})
      else()
        add_dependencies(
          ccov-all-ran
          ccov-profraw-${target_code_coverage_COVERAGE_TARGET_NAME})
      endif()
    endif()
  endif()
endfunction()

# Adds code coverage instrumentation to all targets in the current directory and
# any subdirectories.
#
# To add coverage instrumentation to only specific targets, or to add targets,
# use `target_code_coverage`.
#
# @WARNING: Does not add targets to collect coverage data from executables, use
# `target_code_coverage` to do so.
function(add_code_coverage)
  if(NOT CODE_COVERAGE)
    return()
  endif()

  if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
     OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")
    add_compile_options(
      -fprofile-arcs -ftest-coverage
      $<$<COMPILE_LANGUAGE:CXX>:-fno-elide-constructors> -fno-default-inline)
    link_libraries(gcov)
  endif()
endfunction()

# Adds several 'ccov-all-*' targets that operates runs all targets added via
# `target_code_coverage` with the `ALL` parameter and merges all the coverage
# data into a single large report instead of numerous smaller ones.
# ~~~
# Targets added:
# ccov-all-run : Re-runs all tagged executables, collecting fresh coverage data
# ccov-all-html : Generates an HTML report of all tagged executable coverage data merged into one
# ccov-all : Generates an HTML report of all tagged executable coverage data merged into one (same as ccov-all-html)
#
# LLVM-based coverage targets added:
# ccov-all-report : Generates an HTML report of all tagged executable coverage data merged into one and displays it in the CLI
# ccov-all-export : Exports coverage data in JSON format for use in CI environments or similar
#
# GCC-based coverage targets added:
# ccov-all-capture :  Generates an all-merged.info file, for use with coverage dashboards (e.g. codecov.io, coveralls).
#
# Optional Parameters:
# EXCLUDE <PATTERNS> - Excludes files of the patterns provided from coverage. Note that GCC/lcov excludes by glob pattern, and clang/LLVM excludes via regex!
#
# Optional Parameters effective with the clang/LLVM backend:
# LLVM_EXCLUDE <PATTERNS> - Excludes files that match the provided patterns, LLVM excludes by regex patterns.
# LLVM_PROFDATA_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-profdata` call that merges/processes raw profile data. (.profraw -> .profdata)
# LLVM_COV_REPORT_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov report` call for the `ccov-report-all` target.
# LLVM_COV_EXPORT_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov export` call for the `ccov-export-all` target.
# LLVM_COV_HTML_OPTIONS <OPTIONS> - Options are passed verbatim to the `llvm-cov show -format="html"` call for the `ccov-html-all`/`ccov-all` targets.
#
# Optional Parameters effective with the GCC/lcov backend:
# LCOV_EXCLUDE <PATTERNS> - Excludes files that match the provided patterns. LCOV exclude by glob patterns.
# LCOV_OPTIONS <OPTIONS> - Options are passed verbatim to the `lcov` call when capturing/filtering capture data
# GENHTML_OPTIONS <OPTIONS> - Options are passed verbatim to the `genhtml` call when generating the HTML report from lcov data for the `ccov-html-all`/`ccov-all` targets.
# ~~~
function(add_code_coverage_all_targets)
  if(NOT CODE_COVERAGE)
    return()
  endif()

  # Argument parsing
  set(multi_value_keywords
      # common
      EXCLUDE
      # llvm
      LLVM_EXCLUDE
      LLVM_PROFDATA_OPTIONS
      LLVM_COV_REPORT_OPTIONS
      LLVM_COV_EXPORT_OPTIONS
      LLVM_COV_HTML_OPTIONS
      # lcov
      LCOV_EXCLUDE
      LCOV_OPTIONS
      GENHTML_OPTIONS)
  cmake_parse_arguments(add_code_coverage_all_targets "" ""
                        "${multi_value_keywords}" ${ARGN})

  # invoke to re-run all coverage-instrumented executables
  add_custom_target(ccov-all-run)

  # used to ensure profile data from all targets is available, without forcing a
  # re-run of previously run ones
  add_custom_target(ccov-all-ran)

  if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
     OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")

    # Merge the profile data for all of the run targets
    if(WIN32)
      add_custom_command(
        OUTPUT ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
        COMMAND powershell cat ${CMAKE_COVERAGE_DATA_DIRECTORY}/objects/* >
                ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list
        COMMAND powershell cat ${CMAKE_COVERAGE_DATA_DIRECTORY}/profraw/* >
                ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-profraw.list
        COMMAND
          powershell -Command $$FILELIST = Get-Content
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-profraw.list \; llvm-profdata.exe
          merge ${add_code_coverage_all_targets_LLVM_PROFDATA_OPTIONS} -o
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata -sparse $$FILELIST
        DEPENDS ccov-all-ran)
    else()
      add_custom_command(
        OUTPUT ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
        COMMAND
          ${CMAKE_COMMAND} -E cat ${CMAKE_COVERAGE_DATA_DIRECTORY}/objects/* >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list
        COMMAND
          ${CMAKE_COMMAND} -E cat ${CMAKE_COVERAGE_DATA_DIRECTORY}/profraw/* >
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-profraw.list
        COMMAND
          ${LLVM_PROFDATA_PATH} merge -o
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata -sparse `cat
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-profraw.list`
        DEPENDS ccov-all-ran)
    endif()
    add_custom_target(
      ccov-all-profiledata
      DEPENDS ${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata)

    # Regex exclude only available for LLVM >= 7
    set(EXCLUDE_REGEX)
    if(LLVM_COV_VERSION VERSION_GREATER_EQUAL "7.0.0")
      foreach(EXCLUDE_ITEM ${add_code_coverage_all_targets_EXCLUDE})
        set(EXCLUDE_REGEX ${EXCLUDE_REGEX}
                          -ignore-filename-regex='${EXCLUDE_ITEM}')
      endforeach()
      foreach(EXCLUDE_ITEM ${add_code_coverage_all_targets_LLVM_EXCLUDE})
        set(EXCLUDE_REGEX ${EXCLUDE_REGEX}
                          -ignore-filename-regex='${EXCLUDE_ITEM}')
      endforeach()
    endif()

    # Print summary of the code coverage information to the command line
    if(WIN32)
      add_custom_target(
        ccov-all-report
        COMMAND
          powershell -Command $$FILELIST = Get-Content
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list \; llvm-cov.exe
          report $$FILELIST
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_REPORT_OPTIONS}
        DEPENDS ccov-all-profiledata)
    else()
      add_custom_target(
        ccov-all-report
        COMMAND
          ${LLVM_COV_PATH} report `cat
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list`
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_REPORT_OPTIONS}
        DEPENDS ccov-all-profiledata)
    endif()

    # Export coverage information so continuous integration tools (e.g. Jenkins)
    # can consume it
    if(WIN32)
      add_custom_target(
        ccov-all-export
        COMMAND
          powershell -Command $$FILELIST = Get-Content
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list \; llvm-cov.exe
          export $$FILELIST
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          -format="text" ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_EXPORT_OPTIONS} >
          ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/coverage.json
        DEPENDS ccov-all-profiledata)
    else()
      add_custom_target(
        ccov-all-export
        COMMAND
          ${LLVM_COV_PATH} export `cat
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list`
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          -format="text" ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_EXPORT_OPTIONS} >
          ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/coverage.json
        DEPENDS ccov-all-profiledata)
    endif()

    # Generate HTML output of all added targets for perusal
    if(WIN32)
      add_custom_target(
        ccov-all
        COMMAND
          powershell -Command $$FILELIST = Get-Content
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list \; llvm-cov.exe show
          $$FILELIST
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          -show-line-counts-or-regions
          -output-dir=${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/all-merged
          -format="html" ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_HTML_OPTIONS}
        DEPENDS ccov-all-profiledata)
    else()
      add_custom_target(
        ccov-all
        COMMAND
          ${LLVM_COV_PATH} show `cat
          ${CMAKE_COVERAGE_DATA_DIRECTORY}/all-objects.list`
          -instr-profile=${CMAKE_COVERAGE_DATA_DIRECTORY}/ccov-all.profdata
          -show-line-counts-or-regions
          -output-dir=${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/all-merged
          -format="html" ${EXCLUDE_REGEX}
          ${add_code_coverage_all_targets_LLVM_COV_HTML_OPTIONS}
        DEPENDS ccov-all-profiledata)
    endif()

  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")
    set(COVERAGE_INFO "${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/ccov-all.info")

    # Exclusion regex string creation
    set(EXCLUDE_REGEX)
    foreach(EXCLUDE_ITEM ${add_code_coverage_all_targets_EXCLUDE})
      set(EXCLUDE_REGEX ${EXCLUDE_REGEX} --remove ${COVERAGE_INFO}
                        '${EXCLUDE_ITEM}')
    endforeach()
    foreach(EXCLUDE_ITEM ${add_code_coverage_all_targets_LCOV_EXCLUDE})
      set(EXCLUDE_REGEX ${EXCLUDE_REGEX} --remove ${COVERAGE_INFO}
                        '${EXCLUDE_ITEM}')
    endforeach()

    if(EXCLUDE_REGEX)
      set(EXCLUDE_COMMAND
          ${LCOV_PATH} ${add_code_coverage_all_targets_LCOV_OPTIONS}
          ${EXCLUDE_REGEX} --output-file ${COVERAGE_INFO})
    else()
      set(EXCLUDE_COMMAND ;)
    endif()

    # Capture coverage data
    if(${CMAKE_VERSION} VERSION_LESS "3.17.0")
      add_custom_target(
        ccov-all-clean
        COMMAND ${CMAKE_COMMAND} -E remove -f ${COVERAGE_INFO}
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters)
    else()
      add_custom_target(
        ccov-all-clean
        COMMAND ${CMAKE_COMMAND} -E rm -f ${COVERAGE_INFO}
        COMMAND ${LCOV_PATH} --directory ${CMAKE_BINARY_DIR} --zerocounters)
    endif()
    add_dependencies(ccov-clean ccov-all-clean)

    add_custom_command(
      OUTPUT ${COVERAGE_INFO}
      COMMAND
        ${LCOV_PATH} ${add_code_coverage_all_targets_LCOV_OPTIONS} --directory
        ${CMAKE_BINARY_DIR} --capture --output-file ${COVERAGE_INFO}
      COMMAND ${EXCLUDE_COMMAND}
      COMMAND ${CMAKE_COMMAND} -E echo
              "Generated coverage .info file at ${COVERAGE_INFO}"
      DEPENDS ccov-all-ran)
    add_custom_target(ccov-all-capture DEPENDS ${COVERAGE_INFO})

    # Only generates HTML output of all targets for perusal
    add_custom_target(
      ccov-all-html
      COMMAND
        ${GENHTML_PATH} ${add_code_coverage_all_targets_GENHTML_OPTIONS} -o
        ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/all-merged ${COVERAGE_INFO} -p
        ${CMAKE_SOURCE_DIR}
      COMMAND
        ${CMAKE_COMMAND} -E echo
        "Open ${CMAKE_COVERAGE_OUTPUT_DIRECTORY}/all-merged/index.html in your browser to view the coverage report."
      DEPENDS ${COVERAGE_INFO})

    # Generates HTML output of all targets for perusal
    add_custom_target(ccov-all DEPENDS ccov-all-html)

  endif()
endfunction()

# SignBinaries.cmake --- authenticode-sign Workrave's own executables
#
# Runs as an install(SCRIPT) step, after the runtime DLL dependencies have
# been resolved into CMAKE_INSTALL_PREFIX (see InstallDependencies.cmake in
# the gtkmm/qt Windows dist directories), so both the installer and the
# portable target consume already-signed binaries. Controlled by the
# top-level WITH_SIGN option; a no-op when it's OFF.
#
# Expects (via install(CODE "set(...)")):
#   WITH_SIGN    whether signing is enabled
#   SIGNTOOLSH   path to the authenticode-signing script (only set when WITH_SIGN is ON)

if (WITH_SIGN)
  find_program(BASH_CMD bash REQUIRED)
  file(GLOB_RECURSE exe_files "${CMAKE_INSTALL_PREFIX}/*[Ww]orkrave*.exe")
  if (exe_files)
    message(STATUS "Authenticode-signing: ${exe_files}")
    execute_process(
      COMMAND "${BASH_CMD}" "${SIGNTOOLSH}" ${exe_files}
      RESULT_VARIABLE sign_result
    )
    if (NOT sign_result EQUAL 0)
      message(FATAL_ERROR "Authenticode signing failed")
    endif()
  endif()
endif()

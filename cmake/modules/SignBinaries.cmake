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
#   SIGN_TOOL    path to the ship binary (only set when WITH_SIGN is ON)

if (WITH_SIGN)
  file(GLOB_RECURSE exe_files "${CMAKE_INSTALL_PREFIX}/*[Ww]orkrave*.exe")
  if (exe_files)
    message(STATUS "Authenticode-signing: ${exe_files}")
    execute_process(
      COMMAND "${SIGN_TOOL}" sign authenticode ${exe_files}
      RESULT_VARIABLE sign_result
    )
    if (NOT sign_result EQUAL 0)
      message(FATAL_ERROR "Authenticode signing failed")
    endif()
  endif()
endif()

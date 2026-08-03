# MakePortable.cmake --- assemble and zip the Windows portable distribution
#
# Runs against an already-populated CMAKE_INSTALL_PREFIX (i.e. after
# `ninja install`), the same way PrepareInnoSetup.cmake/setup.iss do for the
# installer. It intentionally does *not* just zip up the whole install
# prefix: that prefix also contains headers, CMake package files, pkg-config
# files and import libraries dropped there by dependencies that install
# themselves (e.g. spdlog, see SPDLOG_INSTALL in the top-level
# CMakeLists.txt). None of that is needed to run Workrave, so only the
# runtime-relevant directories are copied, mirroring what setup.iss ships.
#
# Expected variables (passed via -D):
#   INSTALL_PATH     CMAKE_INSTALL_PREFIX of the build to package
#   DIST_SOURCE_DIR  this dist/windows source directory (Workrave.lnk, workrave.ini)
#   STAGING_DIR      scratch directory to assemble the portable tree in
#   PORTABLE_NAME    name of the top-level folder inside the zip (e.g. Workrave)
#   OUTPUT_ZIP       path of the zip file to produce
#   SIGN             whether to sigstore-sign OUTPUT_ZIP (mirrors the top-level SIGN option)
#   SIGNCOSIGNSH     path to the sigstore-signing script (only required when SIGN is ON)

foreach(var INSTALL_PATH DIST_SOURCE_DIR STAGING_DIR PORTABLE_NAME OUTPUT_ZIP)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "MakePortable.cmake: ${var} was not set")
  endif()
endforeach()

set(app_dir "${STAGING_DIR}/${PORTABLE_NAME}")

file(REMOVE_RECURSE "${STAGING_DIR}")
file(MAKE_DIRECTORY "${app_dir}")

# bin/ can carry workrave.debug, a split-off gdb debug-symbols file only
# installed for Debug builds (see the MINGW block in ui/app/CMakeLists.txt).
# It's large and only useful for debugging with gdb, not for running the app.
if(EXISTS "${INSTALL_PATH}/bin")
  file(COPY "${INSTALL_PATH}/bin" DESTINATION "${app_dir}"
       PATTERN "*.debug" EXCLUDE
       PATTERN "*.pdb" EXCLUDE)
endif()

foreach(component bin32 etc share)
  if(EXISTS "${INSTALL_PATH}/${component}")
    file(COPY "${INSTALL_PATH}/${component}" DESTINATION "${app_dir}")
  endif()
endforeach()

# lib/ carries the gdk-pixbuf loaders and pango modules Workrave needs at
# runtime, but also CMake/pkg-config metadata and import libraries (*.a) for
# dependencies that ran their own `install()` rules. Skip the dev-only bits.
if(EXISTS "${INSTALL_PATH}/lib")
  file(COPY "${INSTALL_PATH}/lib" DESTINATION "${app_dir}"
       PATTERN "cmake" EXCLUDE
       PATTERN "pkgconfig" EXCLUDE
       PATTERN "*.a" EXCLUDE
       PATTERN "*.la" EXCLUDE)
endif()

file(GLOB txt_files "${INSTALL_PATH}/*.txt")
if(txt_files)
  file(COPY ${txt_files} DESTINATION "${app_dir}")
endif()

file(COPY "${DIST_SOURCE_DIR}/Workrave.lnk" DESTINATION "${app_dir}")
file(COPY "${DIST_SOURCE_DIR}/workrave.ini" DESTINATION "${app_dir}/etc")

# The staged executables were already authenticode-signed at `ninja install`
# time (see SignBinaries.cmake), so there's nothing left to sign here besides
# the zip itself.

if(EXISTS "${OUTPUT_ZIP}")
  file(REMOVE "${OUTPUT_ZIP}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar cf "${OUTPUT_ZIP}" --format=zip -- "${PORTABLE_NAME}"
  WORKING_DIRECTORY "${STAGING_DIR}"
  RESULT_VARIABLE tar_result
)

if(NOT tar_result EQUAL 0)
  message(FATAL_ERROR "Failed to create portable zip ${OUTPUT_ZIP}")
endif()

message(STATUS "Created portable zip: ${OUTPUT_ZIP}")

# Sigstore-sign the finished zip, producing "<OUTPUT_ZIP>.sigstore". SIGN and
# SIGNCOSIGNSH are passed in via -D from CMakeLists.txt (derived from the
# top-level SIGN option); SIGNCOSIGNSH is unset when SIGN is OFF.
if(SIGN)
  find_program(BASH_CMD bash REQUIRED)
  message(STATUS "Sigstore-signing: ${OUTPUT_ZIP}")
  execute_process(
    COMMAND "${BASH_CMD}" "${SIGNCOSIGNSH}" "${OUTPUT_ZIP}"
    RESULT_VARIABLE cosign_result
  )
  if(NOT cosign_result EQUAL 0)
    message(FATAL_ERROR "Sigstore signing failed for ${OUTPUT_ZIP}")
  endif()
endif()

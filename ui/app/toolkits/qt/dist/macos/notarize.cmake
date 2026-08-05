# Signs, notarizes and staples ${WORKRAVE_APP} with Apple's notary service.
# Invoked via `cmake --build . --target notarize` (see CMakeLists.txt); not
# meant to be run directly.
#
# Required variables (passed with -D by the `notarize` custom target):
#   WORKRAVE_APP                path to the .app bundle to notarize
#   WORKRAVE_SIGN_IDENTITY      "Developer ID Application: ..." codesign identity
#   WORKRAVE_NOTARIZE_PROFILE   notarytool keychain profile, created once via:
#                                  xcrun notarytool store-credentials <profile> \
#                                      --apple-id <apple-id> --team-id <team-id> \
#                                      --password <app-specific-password>
#   WORKRAVE_NOTARIZE_ZIP       scratch path for the zip submitted for notarization
#   WORKRAVE_BUILD_TYPE         value of CMAKE_BUILD_TYPE from the build

if(NOT WORKRAVE_SIGN_IDENTITY)
    message(FATAL_ERROR "WORKRAVE_SIGN_IDENTITY is not set. Reconfigure with "
        "-DWORKRAVE_SIGN_IDENTITY=\"Developer ID Application: ...\" "
        "(see `security find-identity -v -p codesigning`) and rebuild before notarizing.")
endif()

if(NOT WORKRAVE_NOTARIZE_PROFILE)
    message(FATAL_ERROR "WORKRAVE_NOTARIZE_PROFILE is not set. Store credentials once with "
        "`xcrun notarytool store-credentials <profile> --apple-id <apple-id> "
        "--team-id <team-id> --password <app-specific-password>`, then reconfigure with "
        "-DWORKRAVE_NOTARIZE_PROFILE=<profile>.")
endif()

if(WORKRAVE_BUILD_TYPE MATCHES "Debug")
    message(FATAL_ERROR "Cannot notarize a Debug build: debug builds are re-signed with an "
        "ad-hoc identity and debug entitlements on every install, which would invalidate "
        "notarization. Configure a Release build to notarize.")
endif()

if(NOT EXISTS "${WORKRAVE_APP}")
    message(FATAL_ERROR "App bundle not found at ${WORKRAVE_APP}. Build and install it first.")
endif()

# Two independent reasons the bundle can't be trusted to already be correctly
# signed at this point, so it is re-signed from scratch here rather than relying
# on macdeployqt's own signing:
#  1. `cmake --install` always re-copies a fresh, unsigned main executable from
#     the build tree and fixes up its rpaths with install_name_tool — after
#     macdeployqt has already run — leaving the outer bundle's seal stale.
#  2. Even right after a fresh macdeployqt run, some nested plugins/frameworks
#     have been observed left ad-hoc signed (TeamIdentifier "not set") despite
#     passing -sign-for-notarization=<identity>, which Apple's notary service
#     rejects. macdeployqt's own signing isn't reliable enough here to depend on.
# --deep re-signs every nested framework/plugin/dylib plus the main executable
# in one pass, overriding whatever macdeployqt left behind.
message("-- Re-sealing ${WORKRAVE_APP} with ${WORKRAVE_SIGN_IDENTITY} (deep)...")
execute_process(
    COMMAND codesign --force --deep --options runtime --timestamp
            --sign "${WORKRAVE_SIGN_IDENTITY}" "${WORKRAVE_APP}"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "codesign failed to re-sign ${WORKRAVE_APP} (exit ${_result}).")
endif()

message("-- Verifying code signature of ${WORKRAVE_APP}...")
execute_process(
    COMMAND codesign --verify --deep --strict --verbose=2 "${WORKRAVE_APP}"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "codesign verification failed (exit ${_result}). The app must be "
        "signed with WORKRAVE_SIGN_IDENTITY (a Developer ID identity) before notarizing — "
        "reconfigure/rebuild so macdeployqt signs it, then retry.")
endif()

execute_process(
    COMMAND codesign --display --verbose=2 "${WORKRAVE_APP}"
    ERROR_VARIABLE _codesign_info)
if(NOT _codesign_info MATCHES "Authority=Developer ID Application")
    message(FATAL_ERROR "${WORKRAVE_APP} is not signed with a Developer ID Application "
        "identity (found: ${_codesign_info}). Ad-hoc or self-signed builds cannot be notarized.")
endif()

message("-- Creating archive for notarization submission...")
file(REMOVE "${WORKRAVE_NOTARIZE_ZIP}")
execute_process(
    COMMAND ditto -c -k --keepParent "${WORKRAVE_APP}" "${WORKRAVE_NOTARIZE_ZIP}"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Failed to create notarization archive (exit ${_result}).")
endif()

message("-- Submitting to Apple's notary service (this can take several minutes)...")
execute_process(
    COMMAND xcrun notarytool submit "${WORKRAVE_NOTARIZE_ZIP}"
            --keychain-profile "${WORKRAVE_NOTARIZE_PROFILE}"
            --wait
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _submit_output
    ERROR_VARIABLE _submit_error)
file(REMOVE "${WORKRAVE_NOTARIZE_ZIP}")
message("${_submit_output}")
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Notarization submission failed to run (exit ${_result}): ${_submit_error}")
endif()

# --wait's exit code only reflects whether polling succeeded, not Apple's verdict —
# a rejected ("Invalid") submission still exits 0, so the actual status has to be
# parsed out of the output.
string(REGEX MATCH "id: ([a-zA-Z0-9-]+)" _unused "${_submit_output}")
set(_submission_id "${CMAKE_MATCH_1}")

if(NOT _submit_output MATCHES "status: Accepted")
    if(_submission_id)
        message("-- Apple rejected the submission; fetching the notary log for ${_submission_id}...")
        execute_process(
            COMMAND xcrun notarytool log "${_submission_id}"
                    --keychain-profile "${WORKRAVE_NOTARIZE_PROFILE}")
    endif()
    message(FATAL_ERROR "Apple did not accept ${WORKRAVE_APP} for notarization (see log "
        "above for the specific issue(s)). Fix them and rebuild the 'notarize' target.")
endif()

message("-- Stapling notarization ticket to ${WORKRAVE_APP}...")
execute_process(COMMAND xcrun stapler staple "${WORKRAVE_APP}" RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Stapling failed (exit ${_result}).")
endif()

message("-- Verifying Gatekeeper acceptance...")
execute_process(COMMAND spctl --assess --type execute --verbose=4 "${WORKRAVE_APP}" RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(WARNING "spctl assessment reported issues (exit ${_result}); review the output above.")
endif()

message("-- ${WORKRAVE_APP} is signed and notarized by Apple.")

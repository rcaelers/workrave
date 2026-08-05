# Packages ${WORKRAVE_APP} into a disk image. Invoked via
# `cmake --build . --target dmg`; not meant to be run directly.
#
# This deliberately does NOT use CPack's DragNDrop generator: CPack installs
# into its own temporary staging directory by re-running every install rule
# from scratch (a fresh, unsigned macdeployqt pass, ad-hoc/no signature, no
# notarization) rather than packaging the app that was actually notarized and
# stapled by the `notarize` target. Building the disk image directly from
# WORKRAVE_APP guarantees the exact stapled bundle is what ships.
#
# Required variables (passed with -D by the `dmg` custom target):
#   WORKRAVE_APP                 path to the .app bundle to package
#   WORKRAVE_DMG_STAGING          scratch directory to assemble the volume contents in
#   WORKRAVE_DMG_OUTPUT           output .dmg path
#   WORKRAVE_DMG_BACKGROUND       background image for the Finder window
#   WORKRAVE_DMG_ICON             .icns file used as the mounted volume's icon
#   WORKRAVE_DMG_APPLESCRIPT      AppleScript that lays out the Finder window/icons
#   WORKRAVE_VOLUME_NAME          volume name shown when the dmg is mounted (must
#                                 match the "disk ..." name inside WORKRAVE_DMG_APPLESCRIPT)
#
# Optional (leave both empty to build a plain, unsigned dmg for local testing —
# not something to distribute):
#   WORKRAVE_SIGN_IDENTITY        "Developer ID Application: ..." codesign identity
#   WORKRAVE_NOTARIZE_PROFILE     notarytool keychain profile (see notarize.cmake)
#
# The Finder layout (icon positions, background) is applied by running
# WORKRAVE_DMG_APPLESCRIPT against the freshly mounted volume rather than
# copying in a pre-made .DS_Store: a static .DS_Store's background reference is
# an alias tied to the specific volume/inode it was captured against, and does
# not reliably resolve once dropped into a differently-created disk image —
# the background silently fails to show even though the image file is present.

if(NOT EXISTS "${WORKRAVE_APP}")
    message(FATAL_ERROR "App bundle not found at ${WORKRAVE_APP}. Build and install it first.")
endif()

if(WORKRAVE_SIGN_IDENTITY)
    execute_process(
        COMMAND xcrun stapler validate "${WORKRAVE_APP}"
        RESULT_VARIABLE _staple_result
        OUTPUT_QUIET ERROR_QUIET)
    if(NOT _staple_result EQUAL 0)
        message(FATAL_ERROR "${WORKRAVE_APP} has no valid notarization staple. Run the "
            "'notarize' target first (the 'dmg' target normally depends on it automatically).")
    endif()
    if(NOT WORKRAVE_NOTARIZE_PROFILE)
        message(FATAL_ERROR "WORKRAVE_SIGN_IDENTITY is set but WORKRAVE_NOTARIZE_PROFILE is not.")
    endif()
else()
    message("-- WORKRAVE_SIGN_IDENTITY is not set: building a plain, unsigned dmg for local "
        "testing only (not signed, not notarized, not for distribution).")
endif()

message("-- Assembling disk image contents...")
file(REMOVE_RECURSE "${WORKRAVE_DMG_STAGING}")
file(MAKE_DIRECTORY "${WORKRAVE_DMG_STAGING}/.background")

# ditto (not cp -R) to preserve the extended attributes the notarization
# staple and code signatures are stored in.
execute_process(
    COMMAND ditto "${WORKRAVE_APP}" "${WORKRAVE_DMG_STAGING}/Workrave.app"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Failed to copy the app into the dmg staging area (exit ${_result}).")
endif()

execute_process(COMMAND ln -sf /Applications "${WORKRAVE_DMG_STAGING}/Applications")

# .tiff (not .png) so it can carry both a 1x and a 2x/Retina representation
# (see dmg_background.tiff, built via `tiffutil -cathidpicheck`) — Finder then
# picks the right one for the display instead of always drawing native pixel
# size, which is what made the previous single-resolution PNG look "too small".
if(EXISTS "${WORKRAVE_DMG_BACKGROUND}")
    configure_file("${WORKRAVE_DMG_BACKGROUND}" "${WORKRAVE_DMG_STAGING}/.background/background.png" COPYONLY)
endif()
if(EXISTS "${WORKRAVE_DMG_ICON}")
    configure_file("${WORKRAVE_DMG_ICON}" "${WORKRAVE_DMG_STAGING}/.VolumeIcon.icns" COPYONLY)
endif()

get_filename_component(_dmg_output_dir "${WORKRAVE_DMG_OUTPUT}" DIRECTORY)
file(MAKE_DIRECTORY "${_dmg_output_dir}")
set(_dmg_rw "${_dmg_output_dir}/.${WORKRAVE_VOLUME_NAME}-rw.dmg")

message("-- Creating disk image...")
file(REMOVE "${_dmg_rw}")
execute_process(
    COMMAND hdiutil create
            -volname "${WORKRAVE_VOLUME_NAME}"
            -srcfolder "${WORKRAVE_DMG_STAGING}"
            -fs HFS+
            -fsargs "-c c=64,a=16,e=16"
            -format UDRW
            -ov "${_dmg_rw}"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "hdiutil create failed (exit ${_result}).")
endif()
file(REMOVE_RECURSE "${WORKRAVE_DMG_STAGING}")

# Unmount any stale leftover volume from a previous failed run so hdiutil
# attach below doesn't mount this one as "Workrave 1" instead.
execute_process(COMMAND diskutil unmount force "/Volumes/${WORKRAVE_VOLUME_NAME}" OUTPUT_QUIET ERROR_QUIET)

message("-- Mounting disk image to set the icon layout...")
execute_process(
    COMMAND hdiutil attach -readwrite -noverify -noautoopen "${_dmg_rw}"
    RESULT_VARIABLE _result
    OUTPUT_VARIABLE _attach_output)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "hdiutil attach failed (exit ${_result}).")
endif()

string(REGEX MATCHALL "/dev/disk[0-9]+[^\n]*" _attach_lines "${_attach_output}")
list(GET _attach_lines 0 _first_line)
string(REGEX MATCH "(/dev/disk[0-9]+)" _unused "${_first_line}")
set(_device "${CMAKE_MATCH_1}")
set(_mount_point "")
foreach(_line ${_attach_lines})
    if(_line MATCHES "(/Volumes/.+)$")
        string(STRIP "${CMAKE_MATCH_1}" _mount_point)
    endif()
endforeach()
if(NOT _device OR NOT _mount_point)
    message(FATAL_ERROR "Could not determine the mounted device/volume from hdiutil output:\n${_attach_output}")
endif()

message("-- Mounted at ${_mount_point} (device ${_device})")
execute_process(COMMAND ls -la "${_mount_point}")
execute_process(COMMAND ls -la "${_mount_point}/.background")

if(EXISTS "${_mount_point}/.VolumeIcon.icns")
    message("-- Setting volume icon...")
    execute_process(COMMAND SetFile -c icnC "${_mount_point}/.VolumeIcon.icns" RESULT_VARIABLE _r)
    if(NOT _r EQUAL 0)
        message(WARNING "SetFile -c icnC on .VolumeIcon.icns failed (exit ${_r}).")
    endif()
    execute_process(COMMAND SetFile -a C "${_mount_point}" RESULT_VARIABLE _r)
    if(NOT _r EQUAL 0)
        message(WARNING "SetFile -a C on the volume root failed (exit ${_r}).")
    endif()
else()
    message(WARNING "${_mount_point}/.VolumeIcon.icns does not exist; skipping volume icon.")
endif()

# The leading dot on .background/.fseventsd/.VolumeIcon.icns only hides them
# when Finder's "show hidden files" preference is off — plenty of dev machines
# have that toggled on, which then shows these implementation-detail items
# cluttering the window. The Finder-level invisible bit (-a V) hides them
# unconditionally, regardless of that preference. .fseventsd is created
# automatically by the filesystem itself as soon as the volume is mounted
# read-write, so it's only guaranteed to exist from here on.
foreach(_hidden_item ".background" ".fseventsd" ".VolumeIcon.icns")
    if(EXISTS "${_mount_point}/${_hidden_item}")
        execute_process(COMMAND SetFile -a V "${_mount_point}/${_hidden_item}" RESULT_VARIABLE _r)
        if(NOT _r EQUAL 0)
            message(WARNING "SetFile -a V on ${_hidden_item} failed (exit ${_r}).")
        endif()
        execute_process(COMMAND GetFileInfo -a "${_mount_point}/${_hidden_item}" OUTPUT_VARIABLE _flags OUTPUT_STRIP_TRAILING_WHITESPACE)
        message("-- ${_hidden_item} flags after SetFile: ${_flags}")
    else()
        message(WARNING "${_mount_point}/${_hidden_item} does not exist; cannot hide it.")
    endif()
endforeach()

if(EXISTS "${WORKRAVE_DMG_APPLESCRIPT}")
    message("-- Applying Finder window layout...")
    execute_process(
        COMMAND osascript "${WORKRAVE_DMG_APPLESCRIPT}"
        RESULT_VARIABLE _as_result
        OUTPUT_VARIABLE _as_output
        ERROR_VARIABLE _as_error)
    message("-- osascript output: ${_as_output}")
    if(NOT _as_result EQUAL 0)
        message(WARNING "Applying the Finder layout failed (exit ${_as_result}): ${_as_error}\n"
            "The disk image will still work, just without the custom icon layout/background. "
            "This usually means Terminal/the IDE needs to be granted Automation access to "
            "control Finder (System Settings > Privacy & Security > Automation).")
    endif()
    # .DS_Store is only created once Finder opens/lays out the window above.
    if(EXISTS "${_mount_point}/.DS_Store")
        execute_process(COMMAND SetFile -a V "${_mount_point}/.DS_Store" RESULT_VARIABLE _r)
        if(NOT _r EQUAL 0)
            message(WARNING "SetFile -a V on .DS_Store failed (exit ${_r}).")
        endif()
    else()
        message(WARNING "${_mount_point}/.DS_Store was not created; the Finder layout script "
            "likely did not actually run (see osascript output above).")
    endif()
endif()

execute_process(COMMAND chmod -Rf go-w "${_mount_point}")

message("-- Unmounting disk image...")
execute_process(COMMAND hdiutil detach "${_device}" RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(WARNING "hdiutil detach failed (exit ${_result}); the volume may still be mounted.")
endif()

message("-- Compressing disk image...")
file(REMOVE "${WORKRAVE_DMG_OUTPUT}")
execute_process(
    COMMAND hdiutil convert "${_dmg_rw}"
            -format UDZO
            -imagekey zlib-level=9
            -o "${WORKRAVE_DMG_OUTPUT}"
    RESULT_VARIABLE _result)
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "hdiutil convert failed (exit ${_result}).")
endif()
file(REMOVE "${_dmg_rw}")

if(WORKRAVE_SIGN_IDENTITY)
    message("-- Signing disk image with ${WORKRAVE_SIGN_IDENTITY}...")
    execute_process(
        COMMAND codesign --force --timestamp --sign "${WORKRAVE_SIGN_IDENTITY}" "${WORKRAVE_DMG_OUTPUT}"
        RESULT_VARIABLE _result)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "codesign failed on the disk image (exit ${_result}).")
    endif()

    message("-- Submitting disk image to Apple's notary service (this can take several minutes)...")
    execute_process(
        COMMAND xcrun notarytool submit "${WORKRAVE_DMG_OUTPUT}"
                --keychain-profile "${WORKRAVE_NOTARIZE_PROFILE}"
                --wait
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _submit_output
        ERROR_VARIABLE _submit_error)
    message("${_submit_output}")
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "Notarization submission failed to run (exit ${_result}): ${_submit_error}")
    endif()

    string(REGEX MATCH "id: ([a-zA-Z0-9-]+)" _unused "${_submit_output}")
    set(_submission_id "${CMAKE_MATCH_1}")

    if(NOT _submit_output MATCHES "status: Accepted")
        if(_submission_id)
            message("-- Apple rejected the submission; fetching the notary log for ${_submission_id}...")
            execute_process(
                COMMAND xcrun notarytool log "${_submission_id}"
                        --keychain-profile "${WORKRAVE_NOTARIZE_PROFILE}")
        endif()
        message(FATAL_ERROR "Apple did not accept ${WORKRAVE_DMG_OUTPUT} for notarization (see log "
            "above for the specific issue(s)).")
    endif()

    message("-- Stapling notarization ticket to ${WORKRAVE_DMG_OUTPUT}...")
    execute_process(COMMAND xcrun stapler staple "${WORKRAVE_DMG_OUTPUT}" RESULT_VARIABLE _result)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "Stapling the disk image failed (exit ${_result}).")
    endif()
else()
    message("-- Skipping signing/notarization/stapling (WORKRAVE_SIGN_IDENTITY not set).")
endif()

# Deliberately done *after* signing/notarizing/stapling, not right after
# hdiutil convert: the volume icon set earlier (.VolumeIcon.icns + SetFile -a C
# on the mount point) only controls how the icon looks once mounted — the .dmg
# *file itself* (what Finder shows before it's ever double-clicked) needs its
# own custom icon via the classic resource-fork technique: `sips -i` embeds the
# icns as its own icon resource, DeRez extracts that resource, Rez appends it
# into the target file, and SetFile -a C tells Finder to use it. Apple's
# stapler embeds the notarization ticket using the same resource-fork
# mechanism; doing this first risked staple overwriting it rather than adding
# to it. Plain xattrs/resource-fork data aren't part of what codesign hashes
# for a flat (non-bundle) file, so adding this afterward doesn't invalidate
# the signature already checked by notarization.
if(EXISTS "${WORKRAVE_DMG_ICON}")
    message("-- Setting the disk image's own Finder icon...")
    set(_icon_copy "${_dmg_output_dir}/.${WORKRAVE_VOLUME_NAME}-icon.icns")
    set(_icon_rsrc "${_dmg_output_dir}/.${WORKRAVE_VOLUME_NAME}-icon.rsrc")
    configure_file("${WORKRAVE_DMG_ICON}" "${_icon_copy}" COPYONLY)
    execute_process(COMMAND sips -i "${_icon_copy}" RESULT_VARIABLE _r OUTPUT_QUIET)
    if(NOT _r EQUAL 0)
        message(WARNING "sips -i failed on the icon (exit ${_r}); the disk image will keep the generic icon.")
    else()
        execute_process(
            COMMAND DeRez -only icns "${_icon_copy}"
            OUTPUT_FILE "${_icon_rsrc}"
            RESULT_VARIABLE _r)
        if(NOT _r EQUAL 0)
            message(WARNING "DeRez failed to extract the icon resource (exit ${_r}); the disk image will keep the generic icon.")
        else()
            execute_process(COMMAND Rez -append "${_icon_rsrc}" -o "${WORKRAVE_DMG_OUTPUT}" RESULT_VARIABLE _r)
            if(NOT _r EQUAL 0)
                message(WARNING "Rez failed to attach the icon resource (exit ${_r}); the disk image will keep the generic icon.")
            else()
                execute_process(COMMAND SetFile -a C "${WORKRAVE_DMG_OUTPUT}" RESULT_VARIABLE _r)
                if(NOT _r EQUAL 0)
                    message(WARNING "SetFile -a C on the disk image failed (exit ${_r}).")
                endif()
            endif()
        endif()
    endif()
    file(REMOVE "${_icon_copy}" "${_icon_rsrc}")
endif()

if(WORKRAVE_SIGN_IDENTITY)
    message("-- Verifying Gatekeeper acceptance of the disk image...")
    execute_process(
        COMMAND spctl -a -t open --context context:primary-signature -v "${WORKRAVE_DMG_OUTPUT}"
        RESULT_VARIABLE _result)
    if(NOT _result EQUAL 0)
        message(WARNING "spctl assessment of the disk image reported issues (exit ${_result}); review the output above.")
    endif()
    message("-- ${WORKRAVE_DMG_OUTPUT} is signed, notarized, and ready to distribute.")
else()
    message("-- ${WORKRAVE_DMG_OUTPUT} built (unsigned, local testing only).")
endif()

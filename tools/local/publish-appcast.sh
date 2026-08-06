#!/bin/bash -xe
#
# Publishes the appcast staged by release-windows.sh (and any other build that
# calls appcast_git_push): merges the "staging" branch of the workrave-appcast
# repo into "main" and uploads the resulting appcast.xml to S3. This is the
# point where Workrave clients actually start seeing the update, so run this
# only once the GitHub release is finalized and all platform artifacts
# (including the AppImage) have been uploaded.

source "$(dirname "${BASH_SOURCE[0]}")/release-common.sh"

appcast_git_merge() {
    appcast_git_clone
    git -C "${APPCAST_REPO_DIR}" checkout -B main origin/main
    git -C "${APPCAST_REPO_DIR}" merge --ff-only "origin/${APPCAST_STAGING_BRANCH}"
    run_or_echo git -C "${APPCAST_REPO_DIR}" push origin main
}

upload_appcast_s3() {
    local APPCAST_FILE="${APPCAST_REPO_DIR}/${S3_ARTIFACT_DIR}/appcast.xml"

    if [ ! -f "${APPCAST_FILE}" ]; then
        echo "No appcast.xml found at ${APPCAST_FILE} after merging staging into main" 1>&2
        exit 1
    fi

    # aws.exe is a native Windows binary; MSYS2_ARG_CONV_EXCL="*" (needed below so the
    # s3:// URI isn't mangled) also disables MSYS's automatic POSIX->Windows path
    # conversion, so local paths must be converted explicitly via cygpath -w.
    MSYS2_ARG_CONV_EXCL="*" run_or_echo "${AWS}" s3 --endpoint-url ${SNAPSHOTS_S3_ENDPOINT} cp "$(cygpath -w "${APPCAST_FILE}")" s3://snapshots/${S3_ARTIFACT_DIR}/
    if [ -f "${APPCAST_FILE}.sigstore" ]; then
        MSYS2_ARG_CONV_EXCL="*" run_or_echo "${AWS}" s3 --endpoint-url ${SNAPSHOTS_S3_ENDPOINT} cp "$(cygpath -w "${APPCAST_FILE}.sigstore")" s3://snapshots/${S3_ARTIFACT_DIR}/
    fi
}

PLATFORM_OPTIONS="T"

parse_platform_argument() {
    case "${1}" in
    T)
        DEPLOY_ENVIRONMENT="staging"
        ;;
    *)
        usage
        ;;
    esac
}

init_common_defaults

parse_arguments $*

init_s3_artifact_dir
mkdir -p "${WORKSPACE_DIR}"

init_aws_tools
fetch_snapshots_secret
init_s3
appcast_git_merge
upload_appcast_s3

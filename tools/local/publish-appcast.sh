#!/bin/bash -xe
#
# Publishes the appcast staged by release-windows.sh (and any other build that
# calls appcast_git_push): merges the "staging" branch of the workrave-appcast
# repo into "main" and uploads the resulting appcast.xml to S3. This is the
# point where Workrave clients actually start seeing the update, so run this
# only once the GitHub release is finalized and all platform artifacts
# (including the AppImage) have been uploaded.

init_tools() {
    export AWS=${AWS:-"/c/Program Files/Amazon/AWSCLIV2/aws"}
    export AWS_REGION=us-east-1
    export PATH="/c/Program Files/nodejs:/opt/jq/bin":$PATH
}

init_s3() {
    "${AWS}" configure set aws_access_key_id github
    "${AWS}" configure set aws_secret_access_key ${SNAPSHOTS_SECRET_ACCESS_KEY}
    "${AWS}" configure set default.region us-east-1
    "${AWS}" configure set default.s3.signature_version s3v4
    "${AWS}" configure set s3.endpoint_url https://snapshots.workrave.org/
}

appcast_git_merge() {
    local APPCAST_REPO_URL=git@github.com:rcaelers/workrave-appcast.git
    local APPCAST_STAGING_BRANCH=staging

    if [ ! -d "${APPCAST_REPO_DIR}/.git" ]; then
        git clone "${APPCAST_REPO_URL}" "${APPCAST_REPO_DIR}"
    fi

    git -C "${APPCAST_REPO_DIR}" fetch origin
    git -C "${APPCAST_REPO_DIR}" checkout -B main origin/main
    git -C "${APPCAST_REPO_DIR}" merge --ff-only "origin/${APPCAST_STAGING_BRANCH}"
    git -C "${APPCAST_REPO_DIR}" push origin main
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
    MSYS2_ARG_CONV_EXCL="*" "${AWS}" s3 --endpoint-url https://snapshots.workrave.org/ cp "$(cygpath -w "${APPCAST_FILE}")" s3://snapshots/${S3_ARTIFACT_DIR}/
    if [ -f "${APPCAST_FILE}.sigstore" ]; then
        MSYS2_ARG_CONV_EXCL="*" "${AWS}" s3 --endpoint-url https://snapshots.workrave.org/ cp "$(cygpath -w "${APPCAST_FILE}.sigstore")" s3://snapshots/${S3_ARTIFACT_DIR}/
    fi
}

usage() {
    echo "Usage: $0 [-T] [-W <workspace>]" 1>&2
    exit 1
}

parse_arguments() {
    export DEPLOY_ENVIRONMENT=production

    while getopts "TW:" o; do
        case "${o}" in
        T)
            DEPLOY_ENVIRONMENT="staging"
            ;;
        W)
            WORKSPACE="${OPTARG}"
            ;;
        *)
            usage
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

export WORKSPACE=$(pwd)/_workrave_build_workspace

parse_arguments $*

if [ "${DEPLOY_ENVIRONMENT}" = "staging" ]; then
    export S3_ARTIFACT_DIR=staging/v1.11
else
    export S3_ARTIFACT_DIR=v1.11
fi

mkdir -p "${WORKSPACE}"
export APPCAST_REPO_DIR=${WORKSPACE}/workrave-appcast

SIGNING_SERVICE_URL="${SIGNING_SERVICE_URL:-https://studio.local:50051}"
export SNAPSHOTS_SECRET_ACCESS_KEY=$(curl -skf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.s3_access_key.${DEPLOY_ENVIRONMENT}" | jq -r .value)

init_tools
init_s3
appcast_git_merge
upload_appcast_s3

#!/bin/bash

# Shared functions for release.sh (linux/macos), release-windows.sh and
# publish-appcast.sh. Callers define PLATFORM_OPTIONS and
# parse_platform_argument for their own command line options, call
# init_common_defaults before parse_arguments, and set any platform defaults
# in between.

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

# Runs the given command, or only echoes it when -d (dryrun) is active. Used
# for every side effect a dryrun must not perform: uploads, signing, and any
# contact with the signing service.
run_or_echo() {
    if [ -n "${DRYRUN}" ]; then
        echo "DRYRUN: $*"
    else
        "$@"
    fi
}

init_common_defaults() {
    export WORKRAVE_OVERRIDE_GIT_VERSION=
    export CHANNEL=stable
    export COMMIT=
    export DRYRUN=
    export REPO=https://github.com/rcaelers/workrave.git
    export SIGNING_SERVICE_URL="${SIGNING_SERVICE_URL:-https://studio.local:50051}"
    export SNAPSHOTS_S3_ENDPOINT=https://snapshots.workrave.org/
    export APPCAST_REPO_URL=git@github.com:rcaelers/workrave-appcast.git
    export APPCAST_STAGING_BRANCH=staging
    export DEPLOY_ENVIRONMENT=production
    WORKSPACE_DIR=$(pwd)/_workrave_build_workspace
    SCRIPTS_DIR=
}

parse_arguments() {
    while getopts "c:C:dr:R:t:W:${PLATFORM_OPTIONS}" o; do
        case "${o}" in
        c)
            CHANNEL="${OPTARG}"
            ;;
        C)
            SCRIPTS_DIR=$(realpath "${OPTARG}")
            ;;
        d)
            DRYRUN=-d
            ;;
        r)
            export WORKRAVE_OVERRIDE_GIT_VERSION="${OPTARG}"
            ;;
        R)
            REPO="${OPTARG}"
            ;;
        t)
            COMMIT="${OPTARG}"
            ;;
        W)
            WORKSPACE_DIR="${OPTARG}"
            ;;
        *)
            parse_platform_argument "${o}" "${OPTARG:-}"
            ;;
        esac
    done
    shift $((OPTIND - 1))

    export WORKSPACE_DIR
    export SOURCES_DIR=${WORKSPACE_DIR}/source
    export SCRIPTS_DIR=${SCRIPTS_DIR:-${SOURCES_DIR}/tools}
    export APPCAST_REPO_DIR=${WORKSPACE_DIR}/workrave-appcast
}

init_aws_tools() {
    export AWS=${AWS:-"/c/Program Files/Amazon/AWSCLIV2/aws"}
    export AWS_REGION=us-east-1
    export PATH="/opt/jq/bin:$PATH"
}

init_s3() {
    "${AWS}" configure set aws_access_key_id github
    "${AWS}" configure set aws_secret_access_key ${SNAPSHOTS_SECRET_ACCESS_KEY}
    "${AWS}" configure set default.region us-east-1
    "${AWS}" configure set default.s3.signature_version s3v4
    "${AWS}" configure set s3.endpoint_url ${SNAPSHOTS_S3_ENDPOINT}
}

init_s3_artifact_dir() {
    if [ "${DEPLOY_ENVIRONMENT}" = "staging" ]; then
        export S3_ARTIFACT_DIR=staging/v1.12
    else
        export S3_ARTIFACT_DIR=v1.12
    fi
}

fetch_snapshots_secret() {
    if [ -n "${DRYRUN}" ]; then
        echo "DRYRUN: not fetching S3 access key from ${SIGNING_SERVICE_URL}"
        export SNAPSHOTS_SECRET_ACCESS_KEY=dryrun
        return
    fi
    export SNAPSHOTS_SECRET_ACCESS_KEY=$(curl -skf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.s3_access_key.${DEPLOY_ENVIRONMENT}" | jq -r .value)
}

appcast_git_clone() {
    if [ ! -d "${APPCAST_REPO_DIR}/.git" ]; then
        git clone "${APPCAST_REPO_URL}" "${APPCAST_REPO_DIR}"
    fi
    git -C "${APPCAST_REPO_DIR}" fetch origin
}

init_workspace() {
    if [ ! -d "${SOURCES_DIR}/.git" ]; then
        mkdir -p "$(dirname "${SOURCES_DIR}")"
        git clone "${REPO}" "${SOURCES_DIR}"
        cd "${SOURCES_DIR}"
        git checkout $COMMIT
    else
        cd "${SOURCES_DIR}"
        rm -rf _build _deploy _output
        git reset --hard HEAD
        git clean -fdx
        git fetch
        git checkout $COMMIT
    fi

    cd "${SOURCES_DIR}"
}

init_version() {
    cd "${SOURCES_DIR}"

    if [ -n "$WORKRAVE_OVERRIDE_GIT_VERSION" ]; then
        GIT_VERSION=$WORKRAVE_OVERRIDE_GIT_VERSION
        GIT_TAG=$WORKRAVE_OVERRIDE_GIT_VERSION
    else
        GIT_TAG=$(git describe --abbrev=0)
        GIT_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//')
    fi
    VERSION=$(echo $GIT_VERSION | sed -e 's/_/./g' | sed -e 's/-.*//g')
    WORKRAVE_VERSION=$(echo $GIT_VERSION | sed -e 's/_\([0-9]\)/.\1/g' | sed -E -e 's/-[0-9]+//g' | sed -e 's/_/-/g' | sed -e 's/^v//g')

    if [ $GIT_VERSION = $GIT_TAG ]; then
        echo "Release build"
        export WORKRAVE_RELEASE_TAG=$GIT_TAG
    else
        echo "Snapshot build ($GIT_VERSION) of release ($GIT_TAG)"
    fi
}

fetch_github_token() {
    if [ -n "${DRYRUN}" ]; then
        echo "DRYRUN: not fetching GitHub token from ${SIGNING_SERVICE_URL}"
        return
    fi
    export GH_TOKEN=$(curl -skf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.github_pat" | jq -r .value)
}

# Creates the draft GitHub release for GIT_TAG unless it already exists, so the
# platform release scripts can run in any order, or alone.
github_create_release() {
    GH="${GH:-gh}"
    cd "${SOURCES_DIR}"

    if [ -n "${DRYRUN}" ]; then
        echo "DRYRUN: not checking whether GitHub release ${GIT_TAG} already exists"
    elif "${GH}" release view "${GIT_TAG}" >/dev/null 2>&1; then
        echo "GitHub release ${GIT_TAG} already exists, skipping creation"
        return
    fi

    # The ship binary lives in the source tree when SCRIPTS_DIR is the default,
    # so a git clean since the last build_ship may have removed it.
    if ! type run_ship >/dev/null 2>&1; then
        source "${SCRIPTS_DIR}/ci/ship.sh"
    fi
    if [ ! -x "${SHIP:-}" ]; then
        build_ship
    fi

    NOTES_FILE="${SOURCES_DIR}/_deploy/github-release-news"
    mkdir -p "${SOURCES_DIR}/_deploy"
    run_ship newsgen \
        --input "${SOURCES_DIR}/changes.yaml" \
        --template github \
        --single \
        --release $(echo ${WORKRAVE_VERSION} | sed -e 's/-test$//g') \
        --output "${NOTES_FILE}"

    PRE_RELEASE_ARG=""
    if [ -n "${PRERELEASE}" ] || { [ -n "${CHANNEL}" ] && [ "${CHANNEL}" != "stable" ]; }; then
        PRE_RELEASE_ARG="--prerelease"
    fi

    run_or_echo "${GH}" release create \
        --draft \
        --title "${WORKRAVE_VERSION}" \
        --notes-file="${NOTES_FILE}" \
        ${PRE_RELEASE_ARG} \
        ${GIT_TAG}
}

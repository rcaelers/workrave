#!/bin/bash -xe

init_workspace() {
    if [ ! -d "${WORKSPACE}" ]; then
        mkdir -p "${WORKSPACE}"
    fi

    cd $WORKSPACE

    if [ ! -d ${SOURCES_DIR}/.git ]; then
        mkdir -p ${SOURCES_DIR}
        git clone $REPO source
        cd source
        git checkout $COMMIT
    else
        cd $WORKSPACE/source
        rm -rf _build _deploy _output
        git reset --hard HEAD
        git clean -fdx
        git fetch
        git checkout $COMMIT
    fi

    cd $SOURCES_DIR
}

init_version() {
    if [ -n "$WORKRAVE_OVERRIDE_GIT_VERSION" ]; then
        GIT_VERSION=$WORKRAVE_OVERRIDE_GIT_VERSION
        GIT_TAG=$WORKRAVE_OVERRIDE_GIT_VERSION
    else
        GIT_TAG=$(git describe --abbrev=0)
        GIT_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//')
        VERSION=$(echo $GIT_VERSION | sed -e 's/_/./g' | sed -e 's/-.*//g')
    fi

    if [ $GIT_VERSION = $GIT_TAG ]; then
        echo "Release build"
        export WORKRAVE_RELEASE_TAG=$GIT_TAG
    else
        echo "Snapshot build ($GIT_VERSION) of release ($GIT_TAG)"
    fi
}

init_ship() {
    build_ship
    cd ${SOURCES_DIR}
}

init_tools() {
    export AWS=${AWS:-"/c/Program Files/Amazon/AWSCLIV2/aws"}
    export GH=${GH:-"/c/Program Files/GitHub CLI/gh.exe"}
    export AWS_REGION=us-east-1

    if [ -n "$DOSIGN" ]; then
        SCRIPTS_LOCAL_DIR_WIN=$(cygpath -w ${SCRIPTS_DIR}/local)
        export SIGNTOOLPS1="powershell.exe -ExecutionPolicy Bypass -File ${SCRIPTS_LOCAL_DIR_WIN}\sign-authenticode.ps1"
        export SIGNTOOLSH="${SCRIPTS_DIR}/local/sign-authenticode.sh"
    fi

    export PATH="/c/Program Files/nodejs:/opt/jq/bin":$PATH
    export SYMBOL_SERVER_URL="${SYMBOL_SERVER_URL:-https://crashes.workrave.org}"
}

init() {
    init_workspace
    init_tools
    init_version
    init_ship
    init_s3
}

build_pre() {
    export WORKRAVE_JOB_INDEX=0
    $SCRIPTS_DIR/ci/catalog.sh -c $CHANNEL
}

build() {
    export CONF_CONFIGURATION=Release
    export WORKRAVE_JOB_INDEX=1
    export CONF_SOURCE_TARBALL=1
    export CONF_ENABLE="TESTS,AUTO_UPDATE,CRASHPAD"
    if [ -n "$DOSBOM" ]; then
        CONF_ENABLE="$CONF_ENABLE,SBOM"
    fi
    if [ "${DEPLOY_ENVIRONMENT}" = "staging" ]; then
        CONF_ENABLE="$CONF_ENABLE,UPDATER_STAGING,CRASHPAD_STAGING"
    fi

    $SCRIPTS_DIR/ci/build.sh

    if [ -n "$DODEBUG" ]; then
        export CONF_CONFIGURATION=Debug
        export WORKRAVE_JOB_INDEX=2
        export CONF_ENABLE="TESTS,AUTO_UPDATE"
        unset CONF_SOURCE_TARBALL
        $SCRIPTS_DIR/ci/build.sh
    fi
}

build_post() {
    export ARTIFACTS=$(cygpath -w ${SOURCES_DIR}/_deploy)
    ${SCRIPTS_DIR}/local/sign.sh

    if [ -n "$DOSIGN" ]; then
        for ext in exe zip xz; do
            ARTIFACT=${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.${ext}
            ${SCRIPTS_DIR}/local/sign-cosign.sh ${ARTIFACT}
        done
    fi
}

newsgen() {
    run_ship newsgen \
        --input changes.yaml \
        --template github \
        --single \
        --release $(echo $WORKRAVE_VERSION | sed -e 's/-test$//g') \
        --output "${SOURCES_DIR}/_deploy/github-release-news"
}

github_create_release() {
    newsgen
    PRE_RELEASE=""
    if [ ${CHANNEL} != "stable" ]; then
        PRE_RELEASE="--prerelease"
    fi

    "${GH}" release create \
        --draft \
        --title "${WORKRAVE_VERSION}" \
        --notes-file="${SOURCES_DIR}/_deploy/github-release-news" \
        ${PRE_RELEASE} \
        ${WORKRAVE_GIT_TAG}

    #      --verify-tag
}

upload() {
    upload_s3
    if [ -z "$GITHUB_NOUPLOAD" ]; then
        upload_github
    fi
}

catalog() {
    run_ship catalog --branch ${S3_ARTIFACT_DIR} --workspace ${SOURCES_DIR}
}

appcast() {
    node ${SCRIPTS_DIR}/citool/dist/citool.js appcast --branch ${S3_ARTIFACT_DIR} $([ "${DEPLOY_ENVIRONMENT}" = "staging" ] && echo --environment $DEPLOY_ENVIRONMENT) --file
    if [ -n "$DOSIGN" ]; then
        ${SCRIPTS_DIR}/local/sign-cosign.sh appcast.xml
    fi

    # Not uploaded to S3 here: the appcast is only staged in git. Uploading it now
    # would make clients start auto-updating before the (still draft) GitHub release
    # and the AppImage build are ready. Run publish-appcast.sh once everything is in
    # place to merge staging into main and upload it.
    appcast_git_push
}

upload_symbols() {
    local sym_found=0
    for SYM_FILE in ${SOURCES_DIR}/_build/Release/*.sym; do
        if [ ! -f "${SYM_FILE}" ]; then
            continue
        fi
        sym_found=1
        local SYMBOL_UPLOAD_TOKEN
        SYMBOL_UPLOAD_TOKEN=$(curl -ksf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.symbol_upload.production" | jq -r .value)
        curl -X POST "${SYMBOL_SERVER_URL}/api/symbols/hyltb0goi8jblxonczzw3fsi/upload" \
            --insecure \
            -H "Authorization: Bearer ${SYMBOL_UPLOAD_TOKEN}" \
            -Fupload_file_symbols=@"${SYM_FILE}" \
            -Fversion="${WORKRAVE_VERSION}" \
            -Fchannel="${CHANNEL}" \
            -Fcommit="${WORKRAVE_COMMIT_HASH}" \
            -Fbuild_id="${WORKRAVE_BUILD_ID}"
    done
    if [ ${sym_found} -eq 0 ]; then
        echo "No symbol files found, skipping symbol upload"
    fi
}

appcast_git_push() {
    local APPCAST_REPO_URL=git@github.com:rcaelers/workrave-appcast.git
    local APPCAST_REPO_DIR=${WORKSPACE}/workrave-appcast
    local APPCAST_STAGING_BRANCH=staging

    if [ ! -d "${APPCAST_REPO_DIR}/.git" ]; then
        git clone "${APPCAST_REPO_URL}" "${APPCAST_REPO_DIR}"
    fi

    git -C "${APPCAST_REPO_DIR}" fetch origin
    # Staging is always rebuilt from the tip of main, so it only ever carries this
    # release's pending appcast change (nothing accumulates across releases).
    git -C "${APPCAST_REPO_DIR}" checkout -B "${APPCAST_STAGING_BRANCH}" origin/main

    mkdir -p "${APPCAST_REPO_DIR}/${S3_ARTIFACT_DIR}"
    cp appcast.xml "${APPCAST_REPO_DIR}/${S3_ARTIFACT_DIR}/appcast.xml"
    if [ -n "$DOSIGN" ] && [ -f appcast.xml.sigstore ]; then
        cp appcast.xml.sigstore "${APPCAST_REPO_DIR}/${S3_ARTIFACT_DIR}/appcast.xml.sigstore"
    fi

    git -C "${APPCAST_REPO_DIR}" add -A
    git -C "${APPCAST_REPO_DIR}" commit -m "Update appcast for ${WORKRAVE_VERSION}" || true
    git -C "${APPCAST_REPO_DIR}" push --force-with-lease origin "${APPCAST_STAGING_BRANCH}"
}

init_s3() {
    "${AWS}" configure set aws_access_key_id github
    "${AWS}" configure set aws_secret_access_key ${SNAPSHOTS_SECRET_ACCESS_KEY}
    "${AWS}" configure set default.region us-east-1
    "${AWS}" configure set default.s3.signature_version s3v4
    "${AWS}" configure set s3.endpoint_url https://snapshots.workrave.org/
}

upload_s3() {
    MSYS2_ARG_CONV_EXCL="*" "${AWS}" s3 --endpoint-url https://snapshots.workrave.org/ cp --recursive ${ARTIFACTS} s3://snapshots/${S3_ARTIFACT_DIR}/
}

upload_github() {
    github_create_release
    for ext in exe zip xz; do
        ARTIFACT=${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.${ext}
        if [ -n "$DOSIGN" ]; then
            ${SCRIPTS_DIR}/local/sign-cosign.sh ${ARTIFACT}
            "$GH" release upload ${WORKRAVE_GIT_TAG} ${ARTIFACT}.sigstore
        fi
        "$GH" release upload ${WORKRAVE_GIT_TAG} ${ARTIFACT}
    done
}

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {

    export CHANNEL=stable
    export SCRIPTS_DIR=${WORKSPACE}/source/tools/
    export DOSBOM=
    export DODEBUG=
    export DRYRUN=
    export WORKRAVE_OVERRIDE_GIT_VERSION=
    export REPO=https://github.com/rcaelers/workrave.git
    export DOSIGN=
    export COMMIT=
    export DEPLOY_ENVIRONMENT=production
    export GITHUB_NOUPLOAD=

    while getopts "Bc:C:D:dr:R:st:TW:" o; do
        case "${o}" in
        B)
            DOSBOM=1
            ;;
        c)
            CHANNEL="${OPTARG}"
            ;;
        C)
            pwd
            SCRIPTS_DIR=$(realpath "${OPTARG}")
            ;;
        D)
            DODEBUG=1
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
        s)
            DOSIGN=1
            ;;
        t)
            COMMIT="${OPTARG}"
            ;;
        T)
            DEPLOY_ENVIRONMENT="staging"
            GITHUB_NOUPLOAD=1
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

export WORKRAVE_ENV=local-windows-msys2
export WORKSPACE=$(pwd)/_workrave_build_workspace

parse_arguments $*

export SOURCES_DIR=${WORKSPACE}/source
mkdir -p ${SOURCES_DIR}

export WORKRAVE_BUILD_ID_SUFFIX=local
source ${SCRIPTS_DIR}/ci/config.sh
source ${SCRIPTS_DIR}/ci/ship.sh

SIGNING_SERVICE_URL="${SIGNING_SERVICE_URL:-https://studio.local:50051}"
export SNAPSHOTS_SECRET_ACCESS_KEY=$(curl -skf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.s3_access_key.${DEPLOY_ENVIRONMENT}" | jq -r .value)
export GH_TOKEN=$(curl -skf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.github_pat" | jq -r .value)

init
source ${SCRIPTS_DIR}/ci/config.sh

if [ "${DEPLOY_ENVIRONMENT}" = "staging" ]; then
    export S3_ARTIFACT_DIR=staging/v1.12
else
    export S3_ARTIFACT_DIR=v1.12
fi
export WORKRAVE_UPLOAD_DIR="snapshots/${S3_ARTIFACT_DIR}/${WORKRAVE_BUILD_ID}"

build_pre
build
build_post
if [ -z "${DRYRUN}" ]; then
    upload
    catalog
    appcast
    upload_symbols
fi

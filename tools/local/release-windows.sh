#!/bin/bash -xe

source "$(dirname "${BASH_SOURCE[0]}")/release-common.sh"

init_ship() {
    build_ship
    cd ${SOURCES_DIR}
}

init_tools() {
    init_aws_tools
    export GH=${GH:-"/c/Program Files/GitHub CLI/gh.exe"}
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
    run_or_echo ${SCRIPTS_DIR}/local/sign.sh

    if [ -n "$DOSIGN" ]; then
        for ext in exe zip xz; do
            ARTIFACT=${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.${ext}
            run_or_echo ${SCRIPTS_DIR}/local/sign-cosign.sh ${ARTIFACT}
        done
    fi
}

upload() {
    upload_s3
    if [ -z "$GITHUB_NOUPLOAD" ]; then
        upload_github
    fi
}

catalog() {
    # Needs the S3 credentials from the signing service, so a dryrun only echoes.
    run_or_echo run_ship catalog --branch ${S3_ARTIFACT_DIR} --workspace ${SOURCES_DIR}
}

appcast() {
    local appcast_args=(appcast --branch "${S3_ARTIFACT_DIR}" --file)
    if [ "${DEPLOY_ENVIRONMENT}" = "staging" ]; then
        appcast_args+=(--environment "${DEPLOY_ENVIRONMENT}")
    fi
    # Needs the S3 credentials from the signing service, so a dryrun only echoes.
    run_or_echo run_ship "${appcast_args[@]}"
    if [ -n "$DOSIGN" ]; then
        run_or_echo ${SCRIPTS_DIR}/local/sign-cosign.sh appcast.xml
    fi

    # Not uploaded to S3 here: the appcast is only staged in git. Uploading it now
    # would make clients start auto-updating before the (still draft) GitHub release
    # and the AppImage build are ready. Run publish-appcast.sh once everything is in
    # place to merge staging into main and upload it.
    run_or_echo appcast_git_push
}

upload_symbols() {
    local sym_found=0
    for SYM_FILE in ${SOURCES_DIR}/_build/Release/*.sym; do
        if [ ! -f "${SYM_FILE}" ]; then
            continue
        fi
        sym_found=1
        local SYMBOL_UPLOAD_TOKEN=
        if [ -z "${DRYRUN}" ]; then
            SYMBOL_UPLOAD_TOKEN=$(curl -ksf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.symbol_upload.production" | jq -r .value)
        fi
        run_or_echo curl -X POST "${SYMBOL_SERVER_URL}/api/symbols/hyltb0goi8jblxonczzw3fsi/upload" \
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

upload_mingw_symbols() {
    # The GTK stack we bundle comes from MSYS2 without debug info, so its
    # symbols are recovered from the export tables instead. Read them from the
    # staged install tree rather than /mingw64/bin, so they describe the DLLs
    # this release actually ships.
    local BIN_DIR=${MINGW_SYMBOLS_DIR:-${SOURCES_DIR}/_output/Release/bin}

    if ! command -v python3 >/dev/null 2>&1; then
        echo "python3 not found, skipping MinGW symbol upload"
        return
    fi
    if [ ! -d "${BIN_DIR}" ]; then
        echo "No staged bin directory at ${BIN_DIR}, skipping MinGW symbol upload"
        return
    fi

    local SYMBOL_UPLOAD_TOKEN
    SYMBOL_UPLOAD_TOKEN=$(curl -ksf "${SIGNING_SERVICE_URL}/secrets/secrets.tokens.symbol_upload.production" | jq -r .value)

    # Everything else is already published by now, so a symbol upload that fails
    # is worth shouting about but not worth failing the release over.
    run_or_echo python3 "${SCRIPTS_DIR}/local/upload-mingw-symbols.py" "${BIN_DIR}" \
        --upload \
        --insecure \
        --server "${SYMBOL_SERVER_URL}" \
        --product-token hyltb0goi8jblxonczzw3fsi \
        --token "${SYMBOL_UPLOAD_TOKEN}" \
        --channel "${CHANNEL}" \
        --version "${WORKRAVE_VERSION}" \
        --commit "${WORKRAVE_COMMIT_HASH}" \
        --build-id "${WORKRAVE_BUILD_ID}" \
        || echo "WARNING: MinGW symbol upload failed; crashes in the GTK libraries will not be symbolised"
}

appcast_git_push() {
    appcast_git_clone
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

upload_s3() {
    MSYS2_ARG_CONV_EXCL="*" run_or_echo "${AWS}" s3 --endpoint-url ${SNAPSHOTS_S3_ENDPOINT} cp --recursive ${ARTIFACTS} s3://snapshots/${S3_ARTIFACT_DIR}/
}

upload_github() {
    github_create_release
    for ext in exe zip xz; do
        ARTIFACT=${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.${ext}
        if [ -n "$DOSIGN" ]; then
            run_or_echo ${SCRIPTS_DIR}/local/sign-cosign.sh ${ARTIFACT}
            run_or_echo "$GH" release upload ${WORKRAVE_GIT_TAG} ${ARTIFACT}.sigstore
        fi
        run_or_echo "$GH" release upload ${WORKRAVE_GIT_TAG} ${ARTIFACT}
    done
}

PLATFORM_OPTIONS="BDsT"

parse_platform_argument() {
    case "${1}" in
    B)
        DOSBOM=1
        ;;
    D)
        DODEBUG=1
        ;;
    s)
        DOSIGN=1
        ;;
    T)
        DEPLOY_ENVIRONMENT="staging"
        GITHUB_NOUPLOAD=1
        ;;
    *)
        usage
        ;;
    esac
}

export WORKRAVE_ENV=local-windows-msys2

init_common_defaults
export DOSBOM=
export DODEBUG=
export DOSIGN=
export GITHUB_NOUPLOAD=

parse_arguments $*

export WORKSPACE=${WORKSPACE_DIR}
mkdir -p ${SOURCES_DIR}

export WORKRAVE_BUILD_ID_SUFFIX=local
source ${SCRIPTS_DIR}/ci/config.sh
source ${SCRIPTS_DIR}/ci/ship.sh

fetch_snapshots_secret
fetch_github_token

init
source ${SCRIPTS_DIR}/ci/config.sh

init_s3_artifact_dir
export WORKRAVE_UPLOAD_DIR="snapshots/${S3_ARTIFACT_DIR}/${WORKRAVE_BUILD_ID}"

build_pre
build
build_post
upload
catalog
appcast
upload_symbols
upload_mingw_symbols

#!/bin/bash -ex

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

init_citool() {
    cd ${SCRIPTS_DIR}/citool
    npm ci
    npm run build
    cd ${SOURCES_DIR}
}

init_tools() {
    export OPENSSL=${OPENSSL:-"/opt/openssl/bin/openssl"}
    export AWS=${AWS:-"/c/Program Files/Amazon/AWSCLIV2/aws"}
    export GH=${GH:-"/c/Program Files/GitHub CLI/gh.exe"}

    if [ -n "$DOSIGN" ]; then
        export SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
        export SIGNTOOL_SIGN_ARGS="/n Rob /t http://time.certum.pl /fd sha256 /v"
    fi

    export PATH="/c/Program Files/nodejs":$PATH
}

init() {
    init_workspace
    init_tools
    init_version
    init_citool
}

build_pre() {
    export WORKRAVE_JOB_INDEX=0
    $SCRIPTS_DIR/ci/catalog.sh -c $CHANNEL
}

build() {
    export CONF_CONFIGURATION=Release
    export WORKRAVE_JOB_INDEX=1
    export CONF_SOURCE_TARBALL=1
    export CONF_ENABLE="TESTS,AUTO_UPDATE"
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
    ${SCRIPTS_DIR}/ci/sign.sh
}

newsgen() {
    node ${SCRIPTS_DIR}/citool/dist/citool.js newsgen \
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
    if [ -z "${DRYRUN}" ]; then
        upload_s3
        if [ -z "$GITHUB_NOUPLOAD" ]; then
            upload_github
        fi
    fi
}

catalog() {
    node ${SCRIPTS_DIR}/citool/dist/citool.js catalog --branch ${S3_ARTIFACT_DIR} --workspace ${SOURCES_DIR}
}

appcast() {
    node ${SCRIPTS_DIR}/citool/dist/citool.js appcast --branch ${S3_ARTIFACT_DIR} ${ARTIFACT_ENVIRONMENT:+--environment $ARTIFACT_ENVIRONMENT}
}

upload_s3() {
    "${AWS}" configure set aws_access_key_id travis
    "${AWS}" configure set aws_secret_access_key ${SNAPSHOTS_SECRET_ACCESS_KEY}
    "${AWS}" configure set default.region us-east-1
    "${AWS}" configure set default.s3.signature_version s3v4
    "${AWS}" configure set s3.endpoint_url https://snapshots.workrave.org/
    MSYS2_ARG_CONV_EXCL="*" "${AWS}" s3 --endpoint-url https://snapshots.workrave.org/ cp --recursive ${ARTIFACTS} s3://snapshots/${S3_ARTIFACT_DIR}
}

upload_github() {
    github_create_release
    "$GH" release upload ${WORKRAVE_GIT_TAG} ${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.exe
    "$GH" release upload ${WORKRAVE_GIT_TAG} ${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.zip
    "$GH" release upload ${WORKRAVE_GIT_TAG} ${SOURCES_DIR}/_deploy/${WORKRAVE_BUILD_ID}/*.xz
}

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {

    export CHANNEL=stable
    export SCRIPTS_DIR=${WORKSPACE}/source/tools/
    export DODEBUG=
    export DRYRUN=
    export WORKRAVE_OVERRIDE_GIT_VERSION=
    export REPO=https://github.com/rcaelers/workrave.git
    export DOSIGN=
    export SECRETS_DIR=
    export COMMIT=
    export ARTIFACT_ENV=
    export GITHUB_NOUPLOAD=

    while getopts "c:C:D:dr:R:S:st:TW:" o; do
        case "${o}" in
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
        S)
            SECRETS_DIR=$(realpath "${OPTARG}")
            ;;
        t)
            COMMIT="${OPTARG}"
            ;;
        T)
            ARTIFACT_ENVIRONMENT="staging"
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

    if [ -z ${SECRETS_DIR} ]; then
        echo No secrets directory specified.
        exit 1
    fi
}

export WORKRAVE_ENV=local-windows-msys2
export WORKSPACE=$(pwd)/_workrave_build_workspace

parse_arguments $*

export SOURCES_DIR=${WORKSPACE}/source
mkdir -p ${SOURCES_DIR}

source ${SCRIPTS_DIR}/ci/config.sh
source ${SECRETS_DIR}/env-snapshots

init
source ${SCRIPTS_DIR}/ci/config.sh

export S3_ARTIFACT_DIR=${ARTIFACT_ENVIRONMENT:+$ARTIFACT_ENVIRONMENT/}v1.11
export WORKRAVE_UPLOAD_DIR="snapshots/${S3_ARTIFACT_DIR}/${WORKRAVE_BUILD_ID}"

build_pre
build
build_post
upload
catalog
appcast

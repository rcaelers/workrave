#!/bin/bash -ex

init() {
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
        cd source
        git fetch
        git checkout $COMMIT

    fi

    cd $SOURCES_DIR

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

setup() {
    cd $WORKSPACE/source
    rm -rf _build _deploy _output
    git reset --hard HEAD
    git clean -fdx
    git checkout $COMMIT
}

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {
    while getopts "t:r:C:R:S:W:dP" o; do
        case "${o}" in
        d)
            DRYRUN=-d
            ;;
        t)
            COMMIT="${OPTARG}"
            ;;
        r)
            export WORKRAVE_OVERRIDE_GIT_VERSION="${OPTARG}"
            ;;
        C)
            SCRIPTS_DIR="${OPTARG}"
            ;;
        P)
            PRERELEASE=1
            ;;
        R)
            REPO="${OPTARG}"
            ;;
        S)
            SECRETS_DIR="${OPTARG}"
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
export SCRIPTS_DIR=${WORKSPACE}/source/tools/
export SECRETS_DIR=

export PRERELEASE=
export WORKRAVE_OVERRIDE_GIT_VERSION=
export REPO=https://github.com/rcaelers/workrave.git
export DRYRUN=

parse_arguments $*

export SOURCES_DIR=${WORKSPACE}/source
mkdir -p ${SOURCES_DIR}

# WORKRAVE_BUILD_DATETIME: ${{ needs.prep.outputs.WORKRAVE_BUILD_DATETIME }}
# WORKRAVE_BUILD_DATE: ${{ needs.prep.outputs.WORKRAVE_BUILD_DATE }}

if [ -z ${SECRETS_DIR} ]; then
    echo No secrets directory specified.
    exit 1
fi

BASEDIR=$(dirname "$0")
source ${BASEDIR}/../ci/config.sh

init
setup

export OPENSSL=/opt/openssl/bin/openssl
export SIGNTOOL="c:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\signtool.exe"
export SIGNTOOL_SIGN_ARGS="/n Rob /t http://time.certum.pl /fd sha256 /v"

export CONF_CONFIGURATION=Release
# export CONF_ENABLE="TESTS, CRASHPAD, AUTO_UPDATE"
export CONF_ENABLE="TESTS"
$SCRIPTS_DIR/ci/build.sh

export ARTIFACTS=${DEPLOY_DIR}
${SCRIPTS_DIR}/ci/sign.sh

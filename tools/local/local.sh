#!/bin/bash -e

run_docker_mingw_build() {
    printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g'
    docker run --rm \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -xc "/workspace/source/tools/ci/build.sh"
}

run_docker_ppa() {
    if [ -n "$DEBIAN_DIR" ]; then
        DEBVOL="-v $DEBIAN_DIR:/workspace/debian"
    fi
    docker run --rm \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SECRETS_DIR:/workspace/secrets" \
        -v "$SCRIPTS_DIR:/workspace/scripts" $DEBVOL \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/ci/ppa.sh -p $PPA $DRYRUN"
}

upload() {
    if [ -n "$WORKRAVE_RELEASE_TAG" ]; then
        echo github-release release \
            --user "rcaelers" \
            --repo "workrave" \
            --tag "$WORKRAVE_RELEASE_TAG" \
            --name "Workrave $VERSION" \
            --description "New release" \
            --draft

        shopt -s nullglob
        cd $DEPLOY_DIR
        for file in *.exe *.tar.gz; do
            echo github-release upload \
                --user "rcaelers" \
                --repo "workrave" \
                --tag "$WORKRAVE_RELEASE_TAG" \
                --name $file \
                --file $file
        done

        echo github-release edit \
            --user "rcaelers" \
            --repo "workrave" \
            --tag "$WORKRAVE_RELEASE_TAG" \
            --name "Workrave $VERSION" \
            --description "New release"
    fi
}

init_newsgen() {
    cd ${SCRIPTS_DIR}/newsgen
    npm install
}

init() {
    if [ ! -d "${WORKSPACE_DIR}" ]; then
        mkdir -p "${WORKSPACE_DIR}"
    fi
    if [ -d "${WORKSPACE_DIR}/deploy" ]; then
        rm -rf ${WORKSPACE_DIR}/deploy
    fi
    mkdir -p "${WORKSPACE_DIR}/deploy"

    cd $WORKSPACE_DIR

    if [ ! -d ${SOURCES_DIR} ]; then
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
    GIT_TAG=$(git describe --abbrev=0)
    GIT_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//')
    VERSION=$(echo $GIT_VERSION | sed -e 's/_/./g' | sed -e 's/-.*//g')

    if [ $GIT_VERSION = $GIT_TAG ]; then
        echo "Release build"
        export WORKRAVE_RELEASE_TAG=$GIT_TAG
    else
        echo "Snapshot build ($GIT_VERSION) of release ($GIT_TAG)"
    fi

    init_newsgen
}

generate_news() {
    series=$1

    cd /
    node ${SCRIPTS_DIR}/newsgen/main.js \
        --input "${SOURCES_DIR}/changes.yaml" \
        --template github \
        --release $(echo $VERSION | sed -e 's/^v//g') \
        --output "$WORKSPACE_DIR/deploy/NEWS"

    cd ${SOURCES_DIR}
    DIR_DATE=$(date +"%Y_%m_%d")
    DIR_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//' | sed -e 's/^v//g')
    DIR="${WEBSITE_DIR}/content/en/blog/${DIR_DATE}_workrave-${DIR_VERSION}-released"

    if [ ! -d $DIR ]; then
        mkdir -p ${DIR}
        cd /
        node ${SCRIPTS_DIR}/newsgen/main.js \
            --input "${SOURCES_DIR}/changes.yaml" \
            --template blog \
            --release $(echo $VERSION | sed -e 's/^v//g') \
            --single \
            --output "${DIR}/_index.md"
    fi
}

setup() {
    cd $WORKSPACE_DIR/source
    git reset --hard HEAD
    git clean -fdx
    git checkout $COMMIT
}

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {
    while getopts "At:C:D:R:S:W:B:p:d" o; do
        case "${o}" in
        A)
            CONF_APPIMAGE=1
            ;;
        p)
            PPA="${OPTARG}"
            ;;
        d)
            DRYRUN=-d
            ;;
        t)
            COMMIT="${OPTARG}"
            ;;
        C)
            SCRIPTS_DIR="${OPTARG}"
            ;;
        D)
            DEBIAN_DIR="${OPTARG}"
            ;;
        R)
            REPO="${OPTARG}"
            ;;
        S)
            SECRETS_DIR="${OPTARG}"
            ;;
        W)
            WORKSPACE_DIR="${OPTARG}"
            ;;
        B)
            WEBSITE_DIR="${OPTARG}"
            ;;
        *)
            usage
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

DEBIAN_DIR=
WEBSITE_DIR=
WORKSPACE_DIR=$(pwd)/_workrave_build_workspace
SCRIPTS_DIR=${WORKSPACE_DIR}/source/tools/
REPO=https://github.com/rcaelers/workrave.git
DRYRUN=

parse_arguments $*

if [ -z $SECRETS_DIR ]; then
    echo No secrets directory specified.
    exit 1
fi
if [ -z $WEBSITE_DIR ]; then
    echo No website directory specified.
    exit 1
fi

SOURCES_DIR=$WORKSPACE_DIR/source
DEPLOY_DIR=$WORKSPACE_DIR/deploy

export WORKRAVE_ENV=local
init

export CONF_APPIMAGE=
export CONF_COMPILER="gcc"
export CONF_CONFIGURATION="Release"
export DOCKER_IMAGE="mingw-gtk-rawhide"
setup
run_docker_mingw_build

CONF_COMPILER="gcc"
CONF_CONFIGURATION="Debug"
CONF_DOCKER_IMAGE="mingw-gtk-rawhide"
setup
run_docker_mingw_build

DOCKER_IMAGE="ubuntu-groovy"
setup
run_docker_ppa

generate_news

if [ -z $DRYRUN ]; then
    echo uploading
    upload
fi

#!/bin/bash -ex

run_docker_ppa() {
    if [ -n "$DEBIAN_DIR" ]; then
        DEBVOL="-v $DEBIAN_DIR:/workspace/debian"
    fi
    if [ -n "$PRERELEASE" ]; then
        PRERELEASE_ARG="-P"
    fi
    docker run --rm \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SECRETS_DIR:/workspace/secrets" \
        -v "$SCRIPTS_DIR:/workspace/scripts" $DEBVOL \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/local/ppa.sh -p $PPA $DRYRUN $PRERELEASE_ARG"
}

run_docker_deb() {
    docker run --rm \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        ghcr.io/rcaelers/workrave-build:ubuntu-cowbuilder \
        sh -c "/workspace/scripts/local/cow-build.sh"
}

init_newsgen() {
    cd ${SCRIPTS_DIR}/citool
    npm install
    npm run build
}

init() {
    if [ ! -d "${WORKSPACE_DIR}" ]; then
        mkdir -p "${WORKSPACE_DIR}"
    fi
    if [ ! -d "$DEPLOY_DIR" ]; then
        mkdir -p "$DEPLOY_DIR"
    fi

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

    init_newsgen
}

generate_blog() {
    cd ${SOURCES_DIR}
    DIR_DATE=$(date +"%Y_%m_%d")
    if [ -n "$WORKRAVE_OVERRIDE_GIT_VERSION" ]; then
        DIR_VERSION=$(echo $WORKRAVE_OVERRIDE_GIT_VERSION | sed -e 's/^v//g')
    else
        DIR_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//' | sed -e 's/^v//g')
    fi
    DIR="${WEBSITE_DIR}/content/en/blog/${DIR_DATE}_workrave-${DIR_VERSION}-released"

    if [ ! -d $DIR ]; then
        mkdir -p ${DIR}
        cd /
        node ${SCRIPTS_DIR}/citool/dist/citool.js newsgen \
            --input "${SOURCES_DIR}/changes.yaml" \
            --template blog \
            --release $(echo $WORKRAVE_VERSION | sed -e 's/^v//g') \
            --single \
            --output "${DIR}/index.md"
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
    while getopts "bt:r:C:D:R:S:W:B:p:dP" o; do
        case "${o}" in
        p)
            PPA="${OPTARG}"
            ;;
        b)
            BUILD_DEB=1
            ;;
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

export WORKRAVE_OVERRIDE_GIT_VERSION=
BUILD_DEB=
PRERELEASE=
DEBIAN_DIR=
WEBSITE_DIR=
SECRETS_DIR=
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

DOCKER_IMAGE="ubuntu-kinetic"
setup
run_docker_ppa

if [ -n "$BUILD_DEB" ]; then
    echo Build all debian packages.
    run_docker_deb
fi

generate_blog

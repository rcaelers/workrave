#!/bin/bash -ex

run_docker_ppa() {
    if [ -n "$DEBIAN_DIR" ]; then
        DEBVOL="-v $DEBIAN_DIR:/workspace/debian"
    fi
    docker run --rm \
        -v "$SOURCE_DIR:/workspace/source" \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SECRETS_DIR:/workspace/secrets" \
        -v "$SCRIPTS_DIR:/workspace/scripts" $DEBVOL \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/local/ppa.sh -p $PPA $DRYRUN"
}

run_docker_deb() {
    docker run --rm \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        rcaelers/workrave-build:cowbuilder \
        sh -c "/workspace/scripts/local/cow-build.sh"
}


init_newsgen() {
    cd ${SCRIPTS_DIR}/newsgen
    npm install
}

init() {
    if [ ! -d "${WORKSPACE_DIR}" ]; then
        mkdir -p "${WORKSPACE_DIR}"
    fi
    #if [ -d "$DEPLOY_DIR" ]; then
    #    rm -rf "$DEPLOY_DIR"
    #fi
    #mkdir -p "$DEPLOY_DIR"

    cd $WORKSPACE_DIR

    if [ ! -d ${SOURCE_DIR} ]; then
        mkdir -p ${SOURCE_DIR}
        git clone $REPO source
        cd source
        git checkout $COMMIT
    else
        cd source
        git fetch
        git checkout $COMMIT

    fi

    cd $SOURCE_DIR
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

generate_blog() {
    cd ${SOURCE_DIR}
    DIR_DATE=`date +"%Y_%m_%d"`
    DIR_VERSION=`git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//' | sed -e 's/^v//g'`
    DIR="${WEBSITE_DIR}/content/en/blog/${DIR_DATE}_workrave-${DIR_VERSION}-released"

    if [ ! -d $DIR ]; then
        mkdir -p ${DIR}
        cd /
        node ${SCRIPTS_DIR}/newsgen/main.js \
            --input "${SOURCE_DIR}/changes.yaml" \
            --template blog \
            --release `echo $VERSION | sed -e 's/^v//g'` \
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
    while getopts "bt:C:D:R:S:W:B:p:d" o; do
        case "${o}" in
        p)
            PPA="${OPTARG}"
            ;;
        b)
            BUILD_DEP=1
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

BUILD_DEP=
DEBIAN_DIR=
WEBSITE_DIR=
SECRETS_DIR=
WORKSPACE_DIR=$(pwd)/_workrave_build_workspace
SCRIPTS_DIR=${WORKSPACE_DIR}/source/build/
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

SOURCE_DIR=$WORKSPACE_DIR/source
DEPLOY_DIR=$WORKSPACE_DIR/deploy

export WORKRAVE_ENV=local
init

DOCKER_IMAGE="ubuntu-groovy"
setup
run_docker_ppa

if [ -n $BUILD_DEP ]; then
    echo Build all debian packages.
    run_docker_deb
fi

generate_blog

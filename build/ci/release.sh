#!/bin/bash -x

run_docker_build()
{
    printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed  -e 's/^/-e/g'
    docker run --rm \
           -v "$SOURCE_DIR:/workspace/source" \
           -v "$DEPLOY_DIR:/workspace/deploy" \
           -v "$CI_DIR:/workspace/ci" \
           `printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed  -e 's/^/-e/g' ` \
           rcaelers/workrave-build:${DOCKER_IMAGE} \
           sh -xc "/workspace/ci/build.sh"
}

run_docker_ppa()
{
    if [ -n "$DEBIAN_DIR" ]; then
        DEBVOL="-v $DEBIAN_DIR:/workspace/debian"
    fi
    docker run --rm \
           -v "$SOURCE_DIR:/workspace/source" \
           -v "$DEPLOY_DIR:/workspace/deploy" \
           -v "$SECRETS_DIR:/workspace/secrets" \
           -v "$CI_DIR:/workspace/ci" $DEBVOL \
           `printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed  -e 's/^/-e/g' ` \
            rcaelers/workrave-build:${DOCKER_IMAGE} \
           sh -c "/workspace/ci/ppa.sh"
}

upload()
{
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

init()
{
    if [ ! -d "${WORKSPACE_DIR}" ]; then
        mkdir -p "${WORKSPACE_DIR}"
    fi
    if [ ! -d "${WORKSPACE_DIR}/deploy" ]; then
        mkdir -p "${WORKSPACE_DIR}/deploy"
    fi

    cd $WORKSPACE_DIR
    rm -rf source
    git clone $REPO source
    cd source
    git checkout $COMMIT

    GIT_TAG=`git describe --abbrev=0`
    GIT_VERSION=`git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//'`
    VERSION=`echo $GIT_VERSION | sed -e 's/_/./g' | sed -e 's/-/./g'`

    if [ $GIT_VERSION = $GIT_TAG ]; then
        echo "Release build"
        export WORKRAVE_RELEASE_TAG=$GIT_TAG
    else
        echo "Snapshot build ($GIT_VERSION) of release ($GIT_TAG)"
    fi
}

setup()
{
    cd $WORKSPACE_DIR/source
    git reset --hard HEAD
    git clean -fdx
    git checkout $COMMIT
}


usage()
{
    echo "Usage: $0 " 1>&2;
    exit 1;
}

parse_arguments()
{
    while getopts "t:C:D:R:S:W:" o; do
        case "${o}" in
            t)
                COMMIT="${OPTARG}"
                ;;
            C)
                CI_DIR="${OPTARG}"
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
            *)
                usage
                ;;
        esac
    done
    shift $((OPTIND-1))
}

DEBIAN_DIR=
WORKSPACE_DIR=`pwd`/_workrave_build_workspace
CI_DIR=${WORKSPACE_DIR}/build/ci
REPO=https://github.com/rcaelers/workrave.git

parse_arguments $*

if [ -z $SECRETS_DIR ]; then
    SECRETS_DIR=${WORKSPACE_DIR}/../secrets
fi
SOURCE_DIR=$WORKSPACE_DIR/source
DEPLOY_DIR=$WORKSPACE_DIR/deploy

export WORKRAVE_ENV=local
init

#export CONF_COMPILER="gcc"
#export CONF_CONFIGURATION="Release"
#export DOCKER_IMAGE="mingw-gtk2"
#setup
#run_docker_build
#
#CONF_COMPILER="gcc"
#CONF_CONFIGURATION="Debug"
#CONF_DOCKER_IMAGE="mingw-gtk2"
#setup
#run_docker_build

DOCKER_IMAGE="ubuntu-groovy"
setup
run_docker_ppa

upload


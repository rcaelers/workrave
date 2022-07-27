case "$WORKRAVE_ENV" in
local)
    echo "Running locally"
    WORKSPACE=/workspace
    OUTPUT_DIR=${WORKSPACE}/output
    SOURCES_DIR=${WORKSPACE}/source
    BUILD_DIR=${WORKSPACE}/build
    DEPLOY_DIR=${WORKSPACE}/deploy
    SECRETS_DIR=${WORKSPACE}/secrets
    SCRIPTS_DIR=${WORKSPACE}/scripts
    CI_DIR=${SCRIPTS_DIR}/ci
    ;;

local-msys2)
    echo "Running locally on Windows with msys2"
    WORKSPACE=$(pwd)
    SOURCES_DIR=${WORKSPACE}
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    SCRIPTS_DIR=${SOURCES_DIR}/build
    CI_DIR=${SCRIPTS_DIR}/ci
    ;;

inline)
    echo "Running inline in docker"
    WORKSPACE=/workspace
    SOURCES_DIR=${WORKSPACE}/source
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    SCRIPTS_DIR=${SOURCES_DIR}/build
    CI_DIR=${SCRIPTS_DIR}/ci
    ;;

github-docker)
    echo "Running on Github in docker on Linux"
    WORKSPACE=/workspace
    OUTPUT_DIR=${WORKSPACE}/output
    SOURCES_DIR=${WORKSPACE}/source
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_dist/build
    SCRIPTS_DIR=${SOURCES_DIR}/build
    CI_DIR=${SCRIPTS_DIR}/ci
    ;;

github-windows-docker)
    echo "Running on Git in docker on Windows"
    WORKSPACE=/c/workspace
    OUTPUT_DIR=${WORKSPACE}/output
    SOURCES_DIR=${WORKSPACE}/source
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_dist/build
    SCRIPTS_DIR=${SOURCES_DIR}/build
    CI_DIR=${SCRIPTS_DIR}/build/ci
    ;;

github-ubuntu)
    echo "Running natively on Github/Ununtu"
    WORKSPACE=$GITHUB_WORKSPACE
    SOURCES_DIR=${WORKSPACE}
    OUTPUT_DIR=${WORKSPACE}/output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_dist/build
    CI_DIR=${SOURCES_DIR}/build/ci
    ;;

github-msys2)
    echo "Running natvely on Github/Windows"
    WORKSPACE=$(cygpath $GITHUB_WORKSPACE)
    SOURCES_DIR=${WORKSPACE}
    OUTPUT_DIR=${WORKSPACE}/output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_dist/build
    CI_DIR=${SOURCES_DIR}/build/ci
    ;;

*)
    echo "Unknown environment ($WORKRAVE_ENV)"
    ;;
esac

export DEBFULLNAME="Rob Caelers"
export DEBEMAIL="robc@krandor.org"
export WORKRAVE_PPA=ppa:rob-caelers/workrave
export WORKRAVE_TESTING_PPA=ppa:rob-caelers/workrave-testing

cd ${SOURCES_DIR}

if [ -n "$WORKRAVE_OVERRIDE_GIT_VERSION" ]; then
    export WORKRAVE_GIT_TAG=$WORKRAVE_OVERRIDE_GIT_VERSION
    export WORKRAVE_GIT_VERSION=$WORKRAVE_OVERRIDE_GIT_VERSION
    export WORKRAVE_LONG_GIT_VERSION=$WORKRAVE_OVERRIDE_GIT_VERSION
    export WORKRAVE_COMMIT_COUNT=0
else
    export WORKRAVE_GIT_TAG=$(git describe --abbrev=0)
    export WORKRAVE_GIT_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//')
    export WORKRAVE_LONG_GIT_VERSION=$(git describe --tags --abbrev=10 2>/dev/null)
    export WORKRAVE_COMMIT_COUNT=$(git rev-list ${WORKRAVE_GIT_TAG}..HEAD --count)
fi

export WORKRAVE_VERSION=$(echo $WORKRAVE_GIT_VERSION | sed -e 's/_\([0-9]\)/.\1/g' | sed -E -e 's/-[0-9]+//g' | sed -e 's/_/-/g' | sed -e 's/^v//g')
export WORKRAVE_COMMIT_HASH=$(git rev-parse HEAD)
export WORKRAVE_BUILD_DATE=$(date +"%Y%m%d")
export WORKRAVE_BUILD_DATETIME=$(date --iso-8601=seconds)
export WORKRAVE_BUILD_ID="$WORKRAVE_BUILD_DATE-$WORKRAVE_LONG_GIT_VERSION"
export WORKRAVE_UPLOAD_DIR="snapshots/v1.11/$WORKRAVE_BUILD_ID"

if [ $WORKRAVE_GIT_VERSION != $WORKRAVE_GIT_TAG ]; then
    echo "Snapshot build ($WORKRAVE_GIT_VERSION) of release ($WORKRAVE_GIT_TAG)"
fi

case "$WORKRAVE_ENV" in
local)
    export WORKRAVE_JOB_NUMBER=$WORKRAVE_BUILD_ID
    ;;

inline)
    export WORKRAVE_JOB_NUMBER=$WORKRAVE_BUILD_ID
    ;;

github-docker)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

github-ubuntu)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

github-msys2)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

local-msys2)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    ;;

*)
    echo "Unknown environment"
    ;;
esac

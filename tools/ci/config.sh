case "$WORKRAVE_ENV" in
local)
    echo "Running locally"
    WORKSPACE=/workspace
    OUTPUT_DIR=${WORKSPACE}/output
    SOURCES_DIR=${WORKSPACE}/source
    BUILD_DIR=${WORKSPACE}/build
    DEPLOY_DIR=${WORKSPACE}/deploy
    SECRETS_DIR=${WORKSPACE}/secrets
    ;;

local-windows-msys2)
    echo "Running locally on Windows with msys2"
    WORKSPACE=${WORKSPACE:-$(pwd)}
    SOURCES_DIR=${SOURCES_DIR:-${WORKSPACE}}
    OUTPUT_DIR=${SOURCES_DIR}/_output/${CONF_CONFIGURATION}
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build/${CONF_CONFIGURATION}
    ;;

docker-linux)
    echo "Running on Github in docker on Linux"
    WORKSPACE=/workspace
    SOURCES_DIR=${WORKSPACE}/source
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    ;;

docker-windows-msys2)
    echo "Running on Git in docker on Windows"
    WORKSPACE=/c/workspace
    SOURCES_DIR=${WORKSPACE}/source
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    ;;

github-ubuntu)
    echo "Running natively on Github/Ununtu"
    WORKSPACE=$GITHUB_WORKSPACE
    SOURCES_DIR=${WORKSPACE}
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    ;;

github-windows-msys2)
    echo "Running natvely on Github/Windows"
    WORKSPACE=$(cygpath $GITHUB_WORKSPACE)
    SOURCES_DIR=${WORKSPACE}
    OUTPUT_DIR=${SOURCES_DIR}/_output
    DEPLOY_DIR=${SOURCES_DIR}/_deploy
    BUILD_DIR=${SOURCES_DIR}/_build
    ;;

*)
    echo "Unknown environment ($WORKRAVE_ENV)"
    ;;
esac

SCRIPTS_DIR=${SCRIPTS_DIR:-${SOURCES_DIR}/tools}
SECRETS_DIR=${SECRETS_DIR:-${WORKSPACE}}
CI_DIR=${SCRIPTS_DIR}/ci

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

if [ -n "$WORKRAVE_BUILD_DATETIME" ]; then
    echo "Using WORKRAVE_BUILD_DATETIME=$WORKRAVE_BUILD_DATETIME from environment"
fi
if [ -n "$WORKRAVE_BUILD_DATE" ]; then
    echo "Using WORKRAVE_BUILD_DATE=$WORKRAVE_BUILD_DATE from environment"
fi

export WORKRAVE_VERSION=$(echo $WORKRAVE_GIT_VERSION | sed -e 's/_\([0-9]\)/.\1/g' | sed -E -e 's/-[0-9]+//g' | sed -e 's/_/-/g' | sed -e 's/^v//g')
export WORKRAVE_COMMIT_HASH=$(git rev-parse HEAD)
export WORKRAVE_BUILD_DATE=${WORKRAVE_BUILD_DATE:-$(date +"%Y%m%d")}
if [ "$(uname)" == "Darwin" ]; then
    export WORKRAVE_BUILD_DATETIME=${WORKRAVE_BUILD_DATETIME:-$(date -Iseconds)}
else
    export WORKRAVE_BUILD_DATETIME=${WORKRAVE_BUILD_DATETIME:-$(date --iso-8601=seconds)}
fi
export WORKRAVE_BUILD_ID="$WORKRAVE_BUILD_DATE-$WORKRAVE_LONG_GIT_VERSION${WORKRAVE_BUILD_ID_SUFFIX:-$WORKRAVE_BUILD_ID_SUFFIX}"
export WORKRAVE_UPLOAD_DIR=${WORKRAVE_UPLOAD_DIR:-"snapshots/v1.11/$WORKRAVE_BUILD_ID"}

if [ $WORKRAVE_GIT_VERSION != $WORKRAVE_GIT_TAG ]; then
    echo "Snapshot build ($WORKRAVE_GIT_VERSION) of release ($WORKRAVE_GIT_TAG)"
else
    echo "Release build ($WORKRAVE_VERSION)"
    export WORKRAVE_RELEASE=$WORKRAVE_VERSION
fi

case "$WORKRAVE_ENV" in
local)
    export WORKRAVE_JOB_NUMBER=$WORKRAVE_BUILD_ID
    ;;

local-windows-msys2)
    export WORKRAVE_JOB_NUMBER=${WORKRAVE_BUILD_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

docker-linux)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

docker-windows-msys2)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

github-ubuntu)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

github-windows-msys2)
    export WORKRAVE_JOB_NUMBER=gh${GITHUB_RUN_ID}.${WORKRAVE_JOB_INDEX}
    export DEPLOY_DIR=$DEPLOY_DIR/$WORKRAVE_BUILD_ID
    ;;

*)
    echo "Unknown environment"
    ;;
esac

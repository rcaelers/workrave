if [ -d /workspace ]; then
    echo "Running in Docker environment"
    WORKSPACE=/workspace

    SOURCES_DIR=${WORKSPACE}/source

    ISCC=${WORKSPACE}/inno/app/ISCC.exe
else
    echo "Running in Travis/Native environment"
    WORKSPACE=$TRAVIS_BUILD_DIR

    SOURCES_DIR=${TRAVIS_BUILD_DIR}
fi

OUTPUT_DIR=${WORKSPACE}/output
DEPLOY_DIR=${SOURCES_DIR}/_deploy
BUILD_DIR=${SOURCES_DIR}/_dist/build
SECRETS_DIR=${SOURCES_DIR}/_dist/secrets

MINGW_MAKE_RUNTIME=${SOURCES_DIR}/build/travis/mingw-make-runtime.sh
MINGW_ENV=${SOURCES_DIR}/build/travis/mingw-env

export DEBFULLNAME="Rob Caelers"
export DEBEMAIL="robc@krandor.org"

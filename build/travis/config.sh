if [ -d /workspace ]; then
    echo "Running in Docker env."
    ROOT_DIR=/workspace
    SOURCES_DIR=${ROOT_DIR}/source

    ISCC=${ROOT_DIR}/inno/app/ISCC.exe
else
    echo "Running in Travis env."
    SOURCES_DIR=${TRAVIS_BUILD_DIR}
fi

DEPLOY_DIR=${SOURCES_DIR}/_deploy
BUILD_DIR=${SOURCES_DIR}/_dist/build
SECRETS_DIR=${SOURCES_DIR}/_dist/secrets

MINGW_MAKE_RUNTIME=${SOURCES_DIR}/build/travis/mingw-make-runtime.sh
MINGW_ENV=${SOURCES_DIR}/build/travis/mingw-env

export DEBFULLNAME="Rob Caelers"
export DEBEMAIL="robc@krandor.org"

#!/bin/bash

BUILD_DATE=`date +"%Y%m%d"`

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

WORKRAVE_LONG_GIT_VERSION=`( cd ${SOURCES_DIR} ; git describe --tags --abbrev=10 --dirty 2>/dev/null )`
WORKRAVE_GIT_VERSION=`( cd ${SOURCES_DIR} ; git describe --tags --abbrev=10 --dirty 2>/dev/null | sed -e 's/-g.*//' )`
WORKRAVE_VERSION=`cat ${SOURCES_DIR}/configure.ac | grep AM_INIT_AUTOMAKE | cut -d ','  -f2 | cut -d' ' -f2 | cut -d')' -f1`

prepare_runtime()
{
    cd ${SOURCES_DIR}/frontend/gtkmm/win32/setup/
    ${MINGW_MAKE_RUNTIME} ${SOURCES_DIR}
}

prepare_prebuilt()
{
    cd ${SOURCES_DIR}/common/win32/harpoonHelper/src
    mkdir Release
    cp -a ${ROOT_DIR}/prebuilt/${CONFIGURATION}64/harpoonHelper.exe Release
    cd ${SOURCES_DIR}/common/win32/harpoon/src
    mkdir Release
    cp -a ${ROOT_DIR}/prebuilt/${CONFIGURATION}/harpoon.dll Release
    cp -a ${ROOT_DIR}/prebuilt/${CONFIGURATION}64/harpoon64.dll Release
    cd ${SOURCES_DIR}/frontend/applets/win32/src
    mkdir Release
    cp -a ${ROOT_DIR}/prebuilt/${CONFIGURATION}/workrave-applet.dll Release
    cp -a ${ROOT_DIR}/prebuilt/${CONFIGURATION}64/workrave-applet64.dll Release
}

build()
{
    cd ${SOURCES_DIR}
    . ${MINGW_ENV}

    if [ $CONFIGURATION == "Release" ]; then
        EXTRA_CONF="--disable-debug --disable-tracing"
    else
        EXTRA_CONF="--enable-debug --enable-tracing"
    fi

    bash ./build/win32/autogencross.sh ${CONF_FLAGS} ${EXTRA_CONF} && \
        make && \
        cp -a ${SOURCES_DIR}/frontend/gtkmm/src/.libs/workrave.exe ${SOURCES_DIR}/frontend/gtkmm/src && \
        $TARGET-nm -nosC --line-numbers ${SOURCES_DIR}/frontend/gtkmm/src/workrave.exe >${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym && \
        if [ $CONFIGURATION == "Release" ]; then \
            $$TARGET-strip ${SOURCES_DIR}/frontend/gtkmm/src/workrave.exe; \
        fi
}

make_installer()
{
    cd ${SOURCES_DIR}/frontend/gtkmm/win32/setup/
    unix2dos setup.iss
    wine "${ISCC}" setup.iss

    mkdir ${DEPLOY_DIR}

    EXTRA=
    if [ $CONFIGURATION == "Debug" ]; then
        EXTRA="-Debug"
    fi

    if [[ -z "$TRAVIS_TAG" ]]; then
        echo "No tag build."
        mv ${SOURCES_DIR}/frontend/gtkmm/win32/setup/Output/setup.exe ${DEPLOY_DIR}/workrave-win32-${WORKRAVE_LONG_GIT_VERSION}-${BUILD_DATE}${EXTRA}.exe
        bzip2 -c ${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym >${DEPLOY_DIR}/workrave-win32-${WORKRAVE_LONG_GIT_VERSION}-${BUILD_DATE}${EXTRA}.sym.bz2
    else
        echo "Tag build : $TRAVIS_TAG"
        mv ${SOURCES_DIR}/frontend/gtkmm/win32/setup/Output/setup.exe ${DEPLOY_DIR}/workrave-win32-${TRAVIS_TAG}${EXTRA}.exe
        bzip2 -c ${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym >${DEPLOY_DIR}/workrave-win32-${TRAVIS_TAG}${EXTRA}.sym.bz2
    fi
}

git status

prepare_runtime
prepare_prebuilt
build
make_installer

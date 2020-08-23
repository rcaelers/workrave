#!/bin/bash -xe

echo Mingw

env

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

prepare_runtime()
{
    cd ${SOURCES_DIR}/frontend/gtkmm/win32/setup/
    ${MINGW_MAKE_RUNTIME} ${SOURCES_DIR}
}

prepare_prebuilt()
{
    cd ${SOURCES_DIR}/common/win32/harpoonHelper/src
    if [ ! -d Release ]; then
        mkdir Release
    fi
    cp -a ${PREBUILT_DIR}/${CONF_CONFIGURATION}64/harpoonHelper.exe Release
    cd ${SOURCES_DIR}/common/win32/harpoon/src
    if [ ! -d Release ]; then
        mkdir Release
    fi
    cp -a ${PREBUILT_DIR}/${CONF_CONFIGURATION}/harpoon.dll Release
    cp -a ${PREBUILT_DIR}/${CONF_CONFIGURATION}64/harpoon64.dll Release
    cd ${SOURCES_DIR}/frontend/applets/win32/src
    if [ ! -d Release ]; then
        mkdir Release
    fi
    cp -a ${PREBUILT_DIR}/${CONF_CONFIGURATION}/workrave-applet.dll Release
    cp -a ${PREBUILT_DIR}/${CONF_CONFIGURATION}64/workrave-applet64.dll Release
}

build()
{
    cd ${SOURCES_DIR}
    . ${MINGW_ENV}

    if [ $CONF_CONFIGURATION == "Release" ]; then
        EXTRA_CONF="--disable-debug --disable-tracing"
    else
        EXTRA_CONF="--enable-debug --enable-tracing"
    fi

    bash ./build/win32/autogencross.sh ${CONF_FLAGS} ${EXTRA_CONF} && \
        make -j4 && \
        cp -a ${SOURCES_DIR}/frontend/gtkmm/src/.libs/workrave.exe ${SOURCES_DIR}/frontend/gtkmm/src && \
        $TARGET-nm -nosC --line-numbers ${SOURCES_DIR}/frontend/gtkmm/src/workrave.exe >${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym && \
        if [ $CONF_CONFIGURATION == "Release" ]; then \
            $TARGET-strip ${SOURCES_DIR}/frontend/gtkmm/src/workrave.exe; \
        fi
}

make_installer()
{
    cd ${SOURCES_DIR}/frontend/gtkmm/win32/setup/
    unix2dos setup.iss
    wine "${ISCC}" setup.iss

    mkdir -p ${DEPLOY_DIR}

    CATALOG_CONFIG="release"
    EXTRA=
    if [ $CONF_CONFIGURATION == "Debug" ]; then
        CATALOG_CONFIG="debug"
        EXTRA="-Debug"
    fi

    if [[ -z "$WORKRAVE_TAG" ]]; then
        echo "No tag build."

        baseFilename=workrave-win32-${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
        installerFilename=${baseFilename}.exe
        symbolsFilename=${baseFilename}.sym.bz2

        mv ${SOURCES_DIR}/frontend/gtkmm/win32/setup/Output/setup.exe ${DEPLOY_DIR}/${installerFilename}
        bzip2 -c ${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym >${DEPLOY_DIR}/${symbolsFilename}

    else
        echo "Tag build : $WORKRAVE_TAG"

        baseFilename=workrave-win32-${WORKRAVE_VERSION}${EXTRA}
        installerFilename=${baseFilename}.exe
        symbolsFilename=${baseFilename}.sym.bz2

        mv ${SOURCES_DIR}/frontend/gtkmm/win32/setup/Output/setup.exe ${DEPLOY_DIR}/${installerFilename}
        bzip2 -c ${SOURCES_DIR}/frontend/gtkmm/src/workrave.sym >${DEPLOY_DIR}/${symbolsFilename}
    fi

    ${CI_DIR}/catalog.sh -f ${installerFilename} -k installer -c ${CATALOG_CONFIG} -p windows
    ${CI_DIR}/catalog.sh -f ${symbolsFilename} -k symbols -c ${CATALOG_CONFIG} -p windows

    PORTABLE_DIR=${BUILD_DIR}/portable
    portableFilename=${baseFilename}-portable.zip

    mkdir -p ${PORTABLE_DIR}
    innoextract -d ${PORTABLE_DIR} ${DEPLOY_DIR}/${installerFilename}

    mv ${PORTABLE_DIR}/app ${PORTABLE_DIR}/Workrave

    rm -f ${PORTABLE_DIR}/Workrave/libzapper-0.dll
    cp -a ${SOURCES_DIR}/frontend/gtkmm/win32/Workrave.lnk ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/frontend/gtkmm/win32/workrave.ini ${PORTABLE_DIR}/Workrave/etc

    cd ${PORTABLE_DIR}
    zip -9 -r ${DEPLOY_DIR}/${portableFilename} .

    ${CI_DIR}/catalog.sh -f ${portableFilename} -k portable -c ${CATALOG_CONFIG} -p windows
}

prepare_runtime
prepare_prebuilt
build
make_installer

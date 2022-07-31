#!/bin/bash -e

set -x

if [[ ! $DOCKER_IMAGE =~ "windows" ]]; then
    git config --global --add safe.directory /workspace/source
fi

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

CMAKE_FLAGS=()
CMAKE_FLAGS32=()
MAKE_FLAGS=()
REL_DIR=

build() {
    config=$1
    rel_dir=$2
    cmake_args=("${!3}")

    if [ ! -d ${BUILD_DIR}/${config} ]; then
        mkdir -p ${BUILD_DIR}/${config}
    fi
    if [ ! -d ${OUTPUT_DIR}/${config} ]; then
        mkdir -p ${OUTPUT_DIR}/${config}
    fi

    if [ -n "${rel_dir}" -a ! -d "${BUILD_DIR}/${rel_dir}" ]; then
        echo Performing build at toplevel first
        rel_dir=
    fi

    cd ${BUILD_DIR}/${config}/${rel_dir}

    if [ -z "${rel_dir}" ]; then
        if [ -n "${CONF_APPIMAGE}" ]; then
            cmake ${SOURCES_DIR} -G Ninja -DCMAKE_INSTALL_PREFIX=/usr ${cmake_args[@]}
        else
            cmake ${SOURCES_DIR} -G Ninja -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${config} ${cmake_args[@]}
        fi
    fi

    ninja ${MAKE_FLAGS[@]}

    if [ -n "${CONF_APPIMAGE}" ]; then
        DESTDIR=${OUTPUT_DIR}/AppData ninja ${MAKE_FLAGS[@]} install
    else
        ninja ${MAKE_FLAGS[@]} install
    fi

    ctest
}

parse_arguments() {
    while getopts "d:C:D:M:S:" o; do
        case "${o}" in
        d)
            DOCKER_IMAGE=${OPTARG}
            ;;
        C)
            REL_DIR=${OPTARG}
            ;;
        D)
            CMAKE_FLAGS+=("-D${OPTARG}")
            ;;
        M)
            MAKE_FLAGS+=("${OPTARG}")
            ;;
        S)
            MSYSTEM=${OPTARG}
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

parse_arguments $*

if [[ ${CONF_ENABLE} ]]; then
    for i in ${CONF_ENABLE//,/ }; do
        CMAKE_FLAGS+=("-DWITH_$i=ON")
        echo Enabling $i
    done
fi

if [[ ${CONF_DISABLE} ]]; then
    for i in ${CONF_DISABLE//,/ }; do
        CMAKE_FLAGS+=("-DWITH_$i=OFF")
        echo Disabling $i
    done
fi

if [[ ${CONF_CONFIGURATION} ]]; then
    CMAKE_FLAGS+=("-DCMAKE_BUILD_TYPE=$CONF_CONFIGURATION")
fi

if [ "$(uname)" == "Darwin" ]; then
    CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=$(brew --prefix qt5)")
fi

if [[ -n "$WORKRAVE_TAG" ]]; then
    cd ${SCRIPTS_DIR}/newsgen
    npm install
    cd ${SOURCES_DIR}
    node ${SCRIPTS_DIR}/newsgen/main.js \
        --input "${SOURCE_DIR}/changes.yaml" \
        --template github \
        --single \
        --release $(echo $WORKRAVE_VERSION | sed -e 's/^v//g') \
        --output "release-notes.md"

    ${SOURCES_DIR}/tools/ci/catalog.sh -n release-notes.md
fi

if [[ $DOCKER_IMAGE =~ "mingw" || $DOCKER_IMAGE =~ "windows" || $WORKRAVE_ENV =~ "-msys2" ]]; then
    # CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=${SOURCES_DIR}/_ext -DWITH_CRASHPAD=ON")
    OUT_DIR=""

    if [[ $MSYSTEM == "" ]]; then
        if [[ $WORKRAVE_ENV =~ "-msys2" ]]; then
            MSYSTEM="CLANG64"
        else
            MSYSTEM="MINGW64"
        fi
    fi

    if [[ $MSYSTEM == "MINGW32" ]]; then
        CONF_SYSTEM=mingw32
        CONF_COMPILER=gcc
        OUT_DIR=".32"
        CONF_UI=None
        echo Building 32 bit
    elif [[ $MSYSTEM == "MINGW64" ]]; then
        CONF_SYSTEM=mingw64
    fi

    if [[ $WORKRAVE_ENV =~ "-msys2" || $WORKRAVE_ENV == "docker-windows-msys2" ]]; then
        TOOLCHAIN_FILE=${SOURCES_DIR}/cmake/toolchains/msys2.cmake
        echo Building on MSYS2
    else
        TOOLCHAIN_FILE=${SOURCES_DIR}/cmake/toolchains/${CONF_SYSTEM}-${CONF_COMPILER}.cmake
        echo Building on Linux cross compile environment
        CMAKE_FLAGS+=("-DISCC=/workspace/inno/app/ISCC.exe")
    fi
    CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}")
else
    if [[ $CONF_COMPILER == gcc-* ]]; then
        gccversion=$(echo $CONF_COMPILER | sed -e 's/.*-//')
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++-$gccversion")
        CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=gcc-$gccversion")
    elif [[ $CONF_COMPILER = 'gcc' ]]; then
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++")
        CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=gcc")
    elif [[ $CONF_COMPILER = 'clang' ]]; then
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=clang++")
        CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=clang")
    fi
fi

if [[ ${CONF_UI} ]]; then
    CMAKE_FLAGS+=("-DWITH_UI=${CONF_UI}")
fi

build "${OUT_DIR}" "${REL_DIR}" CMAKE_FLAGS[@]

if [[ $DOCKER_IMAGE =~ "ubuntu" ]]; then
    if [ -n "${CONF_APPIMAGE}" ]; then
        if [ ! -d ${SOURCES_DIR}/_ext ]; then
            mkdir -p ${SOURCES_DIR}/_ext
        fi

        if [ ! -d ${SOURCES_DIR}/_ext/appimage ]; then
            mkdir -p ${SOURCES_DIR}/_ext/appimage
            cd ${SOURCES_DIR}/_ext/appimage
            curl -L -O https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
            chmod +x linuxdeploy-x86_64.AppImage
            curl -L -O https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh
            chmod +x linuxdeploy-plugin-gtk.sh
        fi

        EXTRA=
        CONFIG=release
        if [ "$CONF_CONFIGURATION" == "Debug" ]; then
            EXTRA="-Debug"
            CONFIG="debug"
        fi

        if [[ -z "$WORKRAVE_TAG" ]]; then
            echo "No tag build."
            version=${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
        else
            echo "Tag build : $WORKRAVE_TAG"
            version=${WORKRAVE_VERSION}${EXTRA}
        fi

        export LD_LIBRARY_PATH=${OUTPUT_DIR}/AppData/usr/lib:${OUTPUT_DIR}/AppData/usr/lib/x86_64-linux-gnu

        cd ${OUTPUT_DIR}
        VERSION="$version" ${SOURCES_DIR}/_ext/appimage/linuxdeploy-x86_64.AppImage \
            --appdir ${OUTPUT_DIR}/AppData \
            --plugin gtk \
            --output appimage \
            --icon-file ${OUTPUT_DIR}/AppData/usr/share/icons/hicolor/scalable/apps/workrave.svg \
            --desktop-file ${OUTPUT_DIR}/AppData/usr/share/applications/org.workrave.Workrave.desktop

        mkdir -p ${DEPLOY_DIR}
        cp ${OUTPUT_DIR}/Workrave*.AppImage ${DEPLOY_DIR}/
        ${SOURCES_DIR}/tools/ci/catalog.sh -f Workrave*.AppImage -k appimage -c ${CONFIG} -p linux
    fi
fi

if [[ $MSYSTEM == "MINGW64" || $MSYSTEM == "CLANG64" ]]; then
    echo Deploying
    mkdir -p ${DEPLOY_DIR}

    EXTRA=
    CONFIG=release
    if [ "$CONF_CONFIGURATION" == "Debug" ]; then
        EXTRA="-Debug"
        CONFIG="debug"
    fi

    if [[ -z "$WORKRAVE_TAG" ]]; then
        echo "No tag build."
        baseFilename=workrave-${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
    else
        echo "Tag build : $WORKRAVE_TAG"
        baseFilename=workrave-${WORKRAVE_VERSION}${EXTRA}
    fi

    PORTABLE_DIR=${BUILD_DIR}/portable
    portableFilename=${baseFilename}-portable.zip

    mkdir -p ${PORTABLE_DIR}/Workrave
    cp -a ${OUTPUT_DIR}/*.txt ${OUTPUT_DIR}/lib32 ${OUTPUT_DIR}/lib ${OUTPUT_DIR}/etc/ ${OUTPUT_DIR}/share ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/ui/app/toolkits/gtkmm/dist/windows/Workrave.lnk ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/ui/app/toolkits/gtkmm/dist/windows/workrave.ini ${PORTABLE_DIR}/Workrave/etc

    cd ${PORTABLE_DIR}
    zip -9 -r ${DEPLOY_DIR}/${portableFilename} .

    cd ${BUILD_DIR}
    ${SOURCES_DIR}/tools/ci/catalog.sh -f ${portableFilename} -k portable -c ${CONFIG} -p windows
    ninja ${MAKE_FLAGS[@]} installer

    if [[ -e ${OUTPUT_DIR}/workrave-installer.exe ]]; then

        filename=${baseFilename}.exe
        symbolsFilename=${baseFilename}.sym

        cp ${OUTPUT_DIR}/workrave-installer.exe ${DEPLOY_DIR}/${filename}
        if [[ -e ${OUTPUT_DIR}/workrave.sym ]]; then
            cp ${OUTPUT_DIR}/workrave.sym ${DEPLOY_DIR}/${symbolsFilename}
        fi

        ${SOURCES_DIR}/tools/ci/catalog.sh -f ${filename} -k installer -c $CONFIG -p windows

        if [[ -e ${symbolsFilename} ]]; then
            ${SOURCES_DIR}/tools/ci/catalog.sh -f ${symbolsFilename} -k symbols -c $CONFIG -p windows
        fi
    fi
fi

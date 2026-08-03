#!/bin/bash -ex

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
            cmake ${SOURCES_DIR} -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DAPPIMAGE_APPDIR=${OUTPUT_DIR}/AppData ${cmake_args[@]}
        else
            cmake ${SOURCES_DIR} -G Ninja -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${config} "${cmake_args[@]}"
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
        if [[ $i == "SBOM" ]]; then
            CONF_SBOM=1
        fi
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
    CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=$(brew --prefix qt)")
fi

if [[ $DOCKER_IMAGE =~ "mingw" || $DOCKER_IMAGE =~ "windows" || $WORKRAVE_ENV =~ "-msys2" ]]; then
    OUT_DIR=""

    MSYSTEM="CLANG64"
    CONF_SYSTEM=mingw64

    if [[ $WORKRAVE_ENV =~ "-msys2" || $WORKRAVE_ENV == "docker-windows-msys2" ]]; then
        TOOLCHAIN_FILE=${SOURCES_DIR}/cmake/toolchains/msys2.cmake
        echo Building on MSYS2

        if [[ -n "$DOSIGN" ]]; then
            CMAKE_FLAGS+=("-DSIGN=ON")
            CMAKE_FLAGS+=("-DSIGN_SCRIPTS_ROOT=${SCRIPTS_DIR}/local")
        fi
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

EXTRA=
CONFIG=release
if [ "$CONF_CONFIGURATION" == "Debug" ]; then
    EXTRA="-Debug"
    CONFIG="debug"
fi

if [[ -z "$WORKRAVE_RELEASE" ]]; then
    echo "No tag build."
    baseFilenamePostfix=${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
else
    echo "Tag build : $WORKRAVE_RELEASE"
    baseFilenamePostfix=${WORKRAVE_VERSION}${EXTRA}
fi
baseFilename=workrave-${baseFilenamePostfix}
mkdir -p ${DEPLOY_DIR}

if [ -n "$CONF_SBOM" ]; then
    echo "Generating SBOM"
    set +x
    source ${SCRIPTS_DIR}/local/sbom.sh
    sbom
    set -x
fi

# Source tarball
if [ -n "${CONF_SOURCE_TARBALL}" ]; then
    git -C ${SOURCES_DIR} archive --prefix=${baseFilenamePostfix}/ HEAD | xz >${DEPLOY_DIR}/${baseFilename}.tar.xz
    ${SCRIPTS_DIR}/ci/artifact.sh -f ${baseFilename}.tar.xz -k source -c none -p all
fi

# AppImage
if [[ $DOCKER_IMAGE =~ "ubuntu" ]]; then
    if [ -n "${CONF_APPIMAGE}" ]; then
        ninja ${MAKE_FLAGS[@]} appimage

        appImageFile=$(find "${BUILD_DIR}" -maxdepth 1 -name "Workrave*.AppImage" | head -n1)
        if [ -n "$appImageFile" ]; then
            baseLinuxFilename=workrave-linux-${baseFilenamePostfix}
            filename=${baseLinuxFilename}.AppImage

            cp "$appImageFile" ${DEPLOY_DIR}/${filename}
            ${SCRIPTS_DIR}/ci/artifact.sh -f ${filename} -k appimage -c ${CONFIG} -p linux
        fi
    fi
fi

if [[ $MSYSTEM == "CLANG64" ]]; then
    echo Deploying
    baseWindowsFilename=workrave-windows-${baseFilenamePostfix}

    # The gtkmm and Qt toolkits' dist/windows/CMakeLists.txt name their
    # installer/portable targets differently (workrave-installer.exe vs
    # workrave-qt-installer.exe, etc.) so the same build can carry both
    # side by side; pick the names matching whichever toolkit this build
    # was configured with (CONF_UI, defaults to Gtk+3 like WITH_UI itself).
    if [[ "$CONF_UI" == "Qt" ]]; then
        installerBaseName=workrave-qt-installer
        portableBaseName=workrave-qt-portable
    else
        installerBaseName=workrave-installer
        portableBaseName=workrave-portable
    fi

    # Portable

    portableFilename=${baseWindowsFilename}-portable.zip

    ninja ${MAKE_FLAGS[@]} portable

    if [[ -e ${OUTPUT_DIR}/${portableBaseName}.zip ]]; then
        cp ${OUTPUT_DIR}/${portableBaseName}.zip ${DEPLOY_DIR}/${portableFilename}
        ${SCRIPTS_DIR}/ci/artifact.sh -f ${portableFilename} -k portable -c ${CONFIG} -p windows
    fi

    # Installer

    ninja ${MAKE_FLAGS[@]} installer

    if [[ -e ${OUTPUT_DIR}/${installerBaseName}.exe ]]; then

        # if [[ $WORKRAVE_ENV != "local-windows-msys2" ]]; then
        #
        #     deployFilename=baseFilename=workrave-deploy-${baseFilenamePostfix}.tar.zst
        #
        #     issdir=${BUILD_DIR}/${config}/ui/app/toolkits/gtkmm/dist/windows/
        #     prefix="$(grep ^LicenseFile ${issdir}/setup.iss | sed -e 's/LicenseFile=\(.*\)/\1/' | rev | cut -d\\ -f2- | rev)\\"
        #     for iss in ${issdir}/*.iss; do
        #         cat $iss | sed -e "s|${prefix//\\/\\\\}||" >${OUTPUT_DIR}/$(basename $iss)
        #     done
        #
        #     tar cavf ${DEPLOY_DIR}/${deployFilename} -C $(dirname ${OUTPUT_DIR}) --exclude "**/workrave-installer.exe" ${OUTPUT_DIR}
        #     ${SCRIPTS_DIR}/ci/artifact.sh -f ${deployFilename} -k deploy -c $CONFIG -p windows
        # fi

        filename=${baseWindowsFilename}.exe
        symbolsFilename=${baseWindowsFilename}.sym

        cp ${OUTPUT_DIR}/${installerBaseName}.exe ${DEPLOY_DIR}/${filename}
        if [[ -e ${OUTPUT_DIR}/workrave.sym ]]; then
            cp ${OUTPUT_DIR}/workrave.sym ${DEPLOY_DIR}/${symbolsFilename}
        fi

        ${SCRIPTS_DIR}/ci/artifact.sh -f ${filename} -k installer -c $CONFIG -p windows

        if [[ -e ${symbolsFilename} ]]; then
            ${SCRIPTS_DIR}/ci/artifact.sh -f ${symbolsFilename} -k symbols -c $CONFIG -p windows
        fi
    fi
fi

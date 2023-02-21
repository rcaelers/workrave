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

        if [[ -n "$SIGNTOOL" ]]; then
            CMAKE_FLAGS+=("-DISCC_FLAGS=/DSignTool=Certum;/SCertum=\$q$SIGNTOOL\$q sign $SIGNTOOL_SIGN_ARGS \$f")
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

# Source tarball
if [ -n "${CONF_SOURCE_TARBALL}" ]; then
    git -C ${SOURCES_DIR} archive --prefix=${baseFilenamePostfix}/ HEAD | xz >${DEPLOY_DIR}/${baseFilename}.tar.xz
    ${SCRIPTS_DIR}/ci/artifact.sh -f ${baseFilename}.tar.xz -k source -c none -p all
fi

# AppImage
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

        if [[ -z "$WORKRAVE_RELEASE" ]]; then
            echo "No tag build."
            version=${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
        else
            echo "Tag build : $WORKRAVE_RELEASE"
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

        cp ${OUTPUT_DIR}/Workrave*.AppImage ${DEPLOY_DIR}/
        ${SCRIPTS_DIR}/ci/artifact.sh -f Workrave*.AppImage -k appimage -c ${CONFIG} -p linux
    fi
fi

# Sign Windows binaries
if [[ $WORKRAVE_ENV == "local-windows-msys2" && -n "$SIGNTOOL" ]]; then

    files_to_sign=$(find ${OUTPUT_DIR} -name "*[Ww]orkrave*.exe")

    echo "Signing files : $files_to_sign"

    export MSYS2_ARG_CONV_EXCL="/n;/t;/fd;/v"
    powershell -c "(New-Object Media.SoundPlayer C:/Windows/Media/Alarm05.wav).PlaySync();" &
    "$SIGNTOOL" sign $SIGNTOOL_SIGN_ARGS $files_to_sign
    unset MSYS2_ARG_CONV_EXCL
fi

if [[ $MSYSTEM == "CLANG64" ]]; then
    echo Deploying
    baseWindowsFilename=workrave-windows-${baseFilenamePostfix}

    # Portable

    PORTABLE_DIR=${BUILD_DIR}/portable
    portableFilename=${baseWindowsFilename}-portable.zip

    mkdir -p ${PORTABLE_DIR}/Workrave
    for d in ${OUTPUT_DIR}/bin ${OUTPUT_DIR}/bin32 ${OUTPUT_DIR}/lib ${OUTPUT_DIR}/etc/ ${OUTPUT_DIR}/share; do
        if [ -d $d ]; then
            cp -a $d ${PORTABLE_DIR}/Workrave
        fi
    done
    cp -a ${OUTPUT_DIR}/*.txt ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/ui/app/toolkits/gtkmm/dist/windows/Workrave.lnk ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/ui/app/toolkits/gtkmm/dist/windows/workrave.ini ${PORTABLE_DIR}/Workrave/etc

    cd ${PORTABLE_DIR}
    zip -9 -r ${DEPLOY_DIR}/${portableFilename} .

    cd ${BUILD_DIR}
    ${SCRIPTS_DIR}/ci/artifact.sh -f ${portableFilename} -k portable -c ${CONFIG} -p windows

    # Installer

    ninja ${MAKE_FLAGS[@]} installer

    if [[ -e ${OUTPUT_DIR}/workrave-installer.exe ]]; then

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

        cp ${OUTPUT_DIR}/workrave-installer.exe ${DEPLOY_DIR}/${filename}
        if [[ -e ${OUTPUT_DIR}/workrave.sym ]]; then
            cp ${OUTPUT_DIR}/workrave.sym ${DEPLOY_DIR}/${symbolsFilename}
        fi

        ${SCRIPTS_DIR}/ci/artifact.sh -f ${filename} -k installer -c $CONFIG -p windows

        if [[ -e ${symbolsFilename} ]]; then
            ${SCRIPTS_DIR}/ci/artifact.sh -f ${symbolsFilename} -k symbols -c $CONFIG -p windows
        fi
    fi
fi

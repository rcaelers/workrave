#!/bin/bash -e

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

CMAKE_FLAGS=()
CMAKE_FLAGS32=()
MAKE_FLAGS=()
REL_DIR=

build()
{
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
      cmake ${SOURCES_DIR} -G Ninja -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}/${config} ${cmake_args[@]}
  fi

  ninja ${MAKE_FLAGS[@]}
  ninja ${MAKE_FLAGS[@]} install
}

parse_arguments()
{
  while getopts "d:C:D:M:" o; do
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
      esac
  done
  shift $((OPTIND-1))
}

parse_arguments $*

if [[ ${CONF_ENABLE} ]]; then
    for i in ${CONF_ENABLE//,/ }
    do
        CMAKE_FLAGS+=("-DWITH_$i=ON")
        CMAKE_FLAGS32+=("-DWITH_$i=ON")
    done
fi

if [[ ${CONF_DISABLE} ]]; then
    for i in ${CONF_DISABLE//,/ }
    do
        CMAKE_FLAGS+=("-DWITH_$i=OFF")
        CMAKE_FLAGS32+=("-DWITH_$i=OFF")
    done
fi

if [[ ${CONF_UI} ]]; then
    CMAKE_FLAGS+=("-DWITH_UI=${CONF_UI}")
fi

if [[ $DOCKER_IMAGE =~ "mingw" ]] ; then
    if [ ! -d ${SOURCES_DIR}/_ext ]; then
        mkdir -p ${SOURCES_DIR}/_ext
    fi

    crashpad_name=crashpad-mingw64-20201224-18-52ff3f4d6a8277499327ee8d0c9fdc759016ee2e
    crashpad_ext=.tar.xz
    if [ ! -d ${SOURCES_DIR}/_ext/${crashpad_name} ]; then
        curl https://snapshots.workrave.org/crashpad/${crashpad_name}${crashpad_ext} | tar xvJ -C ${SOURCES_DIR}/_ext
        rm -f ${SOURCES_DIR}/_ext/crashpad
        cd ${SOURCES_DIR}/_ext
        ln -s ${crashpad_name} crashpad
        cd ${SOURCES_DIR}/
    fi
else
    if [[ $CONF_COMPILER = 'gcc' ]] ; then
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=g++")
        CMAKE_FLAGS32+=("-DCMAKE_CXX_COMPILER=g++")
        CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=gcc")
        CMAKE_FLAGS32+=("-DCMAKE_C_COMPILER=gcc")
    elif [[ $CONF_COMPILER = 'clang' ]] ; then
        CMAKE_FLAGS+=("-DCMAKE_CXX_COMPILER=clang++")
        CMAKE_FLAGS32+=("-DCMAKE_CXX_COMPILER=clang++")
        CMAKE_FLAGS+=("-DCMAKE_C_COMPILER=clang")
        CMAKE_FLAGS32+=("-DCMAKE_C_COMPILER=clang")
    fi
fi

if [[ ${CONF_CONFIGURATION} ]]; then
    CMAKE_FLAGS+=("-DCMAKE_BUILD_TYPE=$CONF_CONFIGURATION")
fi

if [ "$(uname)" == "Darwin" ]; then
    CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=$(brew --prefix qt5)")
fi

case "$DOCKER_IMAGE" in
    mingw-qt5)
        CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCES_DIR}/build/cmake/mingw64-${CONF_COMPILER}.cmake")
        CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
        CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=${SOURCES_DIR}/_ext")
        ;;

    mingw-gtk*)
        CMAKE_FLAGS+=("-DCMAKE_PREFIX_PATH=${SOURCES_DIR}/_ext")
        CMAKE_FLAGS32+=("-DCMAKE_PREFIX_PATH=${SOURCES_DIR}/_ext")

        case "$PREBUILT" in
            vs)
                CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCES_DIR}/build/cmake/mingw64-${CONF_COMPILER}.cmake")
                CMAKE_FLAGS+=("-DPREBUILT_PATH=${WORKSPACE}/prebuilt")
                ;;

            *)
                CMAKE_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCES_DIR}/build/cmake/mingw64-${CONF_COMPILER}.cmake")
                CMAKE_FLAGS+=("-DPREBUILT_PATH=${OUTPUT_DIR}/.32")

                CMAKE_FLAGS32+=("-DCMAKE_TOOLCHAIN_FILE=${SOURCES_DIR}/build/cmake/mingw32-gcc.cmake")
                CMAKE_FLAGS32+=("-DWITH_UI=None")
                CMAKE_FLAGS32+=("-DCMAKE_BUILD_TYPE=Release")

                build ".32" "" CMAKE_FLAGS32[@]
                ;;
        esac
        ;;
esac

build "" "${REL_DIR}" CMAKE_FLAGS[@]

mkdir -p ${DEPLOY_DIR}

EXTRA=
CONFIG=release
if [ "$CONF_CONFIGURATION" == "Debug" ]; then
    EXTRA="-Debug"
    CONFIG="debug"
fi

if [[ -e ${OUTPUT_DIR}/mysetup.exe ]]; then
    if [[ -z "$WORKRAVE_TAG" ]]; then
        echo "No tag build."
        baseFilename=workrave-${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}${EXTRA}
    else
        echo "Tag build : $WORKRAVE_TAG"
        baseFilename=workrave-${WORKRAVE_VERSION}${EXTRA}
    fi

    filename=${baseFilename}.exe
    symbolsFilename=${baseFilename}.sym

    cp ${OUTPUT_DIR}/mysetup.exe ${DEPLOY_DIR}/${filename}
    cp ${OUTPUT_DIR}/workrave.sym ${DEPLOY_DIR}/${symbolsFilename}

    ${SOURCES_DIR}/build/ci/catalog.sh -f ${filename} -k installer -c $CONFIG -p windows
    ${SOURCES_DIR}/build/ci/catalog.sh -f ${symbolsFilename} -k symbols -c $CONFIG -p windows

    PORTABLE_DIR=${BUILD_DIR}/portable
    portableFilename=${baseFilename}-portable.zip

    mkdir -p ${PORTABLE_DIR}
    innoextract -d ${PORTABLE_DIR} ${DEPLOY_DIR}/${filename}

    mv ${PORTABLE_DIR}/app ${PORTABLE_DIR}/Workrave

    rm -f ${PORTABLE_DIR}/Workrave/libzapper-0.dll
    cp -a ${SOURCES_DIR}/ui/apps/gtkmm/dist/win32/Workrave.lnk ${PORTABLE_DIR}/Workrave
    cp -a ${SOURCES_DIR}/ui/apps/gtkmm/dist/win32/workrave.ini ${PORTABLE_DIR}/Workrave/etc

    cd ${PORTABLE_DIR}
    zip -9 -r ${DEPLOY_DIR}/${portableFilename} .

    ${SOURCES_DIR}/build/ci/catalog.sh -f ${portableFilename} -k portable -c ${CONFIG} -p windows
fi

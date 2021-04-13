#!/bin/bash -e

BUILD_DATE=`date +"%Y%m%d"`

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

WORKRAVE_LONG_GIT_VERSION=`( cd ${SOURCES_DIR} ; git describe --tags --abbrev=10 --dirty 2>/dev/null )`
WORKRAVE_GIT_VERSION=`( cd ${SOURCES_DIR} ; git describe --tags --abbrev=10 --dirty 2>/dev/null | sed -e 's/-g.*//' )`
WORKRAVE_VERSION=`cat ${SOURCES_DIR}/configure.ac | grep AM_INIT_AUTOMAKE | cut -d ','  -f2 | cut -d' ' -f2 | cut -d')' -f1`

conf_opt()
{
    if [[ $CONF_ENABLE =~ (^|[, ])$1([, ]|$) ]]; then
        echo "--enable-$1";
    else
        if [[ $CONF_DISABLE =~ (^|[, ])$1([, ]|$) ]]; then
            echo "--disable-$1";
        else
            echo "Parameter $1 is not in \$CONF_ENABLE or \$CONF_DISABLE " 1>&2
        fi
    fi
}

conf_opt_packages()
{
    if [[ $CONF_ENABLE =~ (^|[, ])$1([, ]|$) ]]; then
        shift
        echo "$@"
    else
        if [[ !( $CONF_DISABLE =~ (^|[, ])$1([, ]|$) ) ]]; then
            echo "Parameter $1 is not in \$CONF_ENABLE or \$CONF_DISABLE " 1>&2
        fi
    fi
}

cd ${SOURCES_DIR}

export DEBIAN_FRONTEND=noninteractive
apt-get update -q
apt-get dist-upgrade -y -q
apt-get -y -q -V --no-install-recommends install \
        build-essential \
        automake \
        autoconf \
        autoconf-archive \
        libtool \
        libgdome2-dev \
        libgconf2-dev \
        python3-jinja2 \
        `[[ $CONF_GTK_VER = 2 ]] && echo libgtk2.0-dev libgtkmm-2.4-dev`

if [[ $COMPILER = 'clang' ]] ; then
    export CC=/usr/bin/clang
    export CXX=/usr/bin/clang++
fi

./autogen.sh
./configure \
    --enable-option-checking=fatal \
    --disable-static \
    --disable-dependency-tracking \
    `conf_opt gconf` \
    `conf_opt gsettings` \
    `conf_opt xml` \
    `conf_opt gnome3` \
    `conf_opt indicator` \
    `conf_opt xfce` \
    `conf_opt mate` \
    `conf_opt dbus` \
    `conf_opt distribution` \
    `conf_opt gstreamer` \
    `conf_opt pulse` \
    `conf_opt exercises` \
    `conf_opt experimental`

mkdir -p ${DEPLOY_DIR}

if [ -z "$DISTCHECK" ]; then
    make && make check
else
    make && make dist && make distcheck

    if [[ -z "$TRAVIS_TAG" ]]; then
        echo "No tag build."
        filename=workrave-${WORKRAVE_LONG_GIT_VERSION}-${BUILD_DATE}.tar.gz
        cp -a workrave*tar.gz ${DEPLOY_DIR}/${filename}
    else
        echo "Tag build : $TRAVIS_TAG"
        filename=`echo workrave*tar.gz`
        cp -a workrave*tar.gz ${DEPLOY_DIR}
    fi

    ${SOURCES_DIR}/build/travis/catalog.sh -f ${filename} -k source -c none -p all
fi

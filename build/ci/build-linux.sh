#!/bin/bash -e

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

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

git config --global --add safe.directory ${SOURCES_DIR}

export DEBIAN_FRONTEND=noninteractive
apt-get update -q
apt-get dist-upgrade -y -q
apt-get -y -q -V --no-install-recommends install \
        libgdome2-dev \
        python3-jinja2 \
        `[[ $CONF_GTK_VER = 2 ]] && echo libgtk2.0-dev libgtkmm-2.4-dev`

if [[ $CONF_COMPILER = 'clang' ]] ; then
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

MAKE="make -j4"
if [ -z "$CONF_DISTCHECK" ]; then
    $MAKE && $MAKE check
else
    $MAKE && $MAKE dist && $MAKE distcheck

    if [[ -z "$WORKRAVE_TAG" ]]; then
        echo "No tag build."
        filename=workrave-${WORKRAVE_LONG_GIT_VERSION}-${WORKRAVE_BUILD_DATE}.tar.gz
        cp -a workrave*tar.gz ${DEPLOY_DIR}/${filename}
    else
        echo "Tag build : $WORKRAVE_TAG"
        filename=`echo workrave*tar.gz`
        cp -a workrave*tar.gz ${DEPLOY_DIR}
    fi

    ${CI_DIR}/catalog.sh -f ${filename} -k source -c none -p all
fi

#if [ -n $CONF_DEB ]; then
#    rm -rf debian
#    git clone -b debian-packaging https://github.com/rcaelers/workrave.git $WORKSPACE/debian-packaging
#
#    series=`lsb_release -cs`
#    cp -a $WORKSPACE/debian-packaging/debian .
#    if [ -d "$WORKSPACE/debian-packaging/debian-${series}" ]; then
#        cp -a $WORKSPACE/debian-packaging/debian-${series}/* debian/
#    fi
#
#    dpkg-buildpackage -b -rfakeroot -us -uc
#fi

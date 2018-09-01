#!/bin/bash

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

cd /workspace/source

apt-get update -q
apt-get -y -q -V --no-install-recommends install \
        build-essential \
        automake \
        autoconf \
        autoconf-archive \
        libtool \
        autopoint \
        intltool \
        libgdome2-dev \
        libgconf2-dev \
        python-cheetah \
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

make && make check

if [[ -z "$TRAVIS_TAG" ]]; then
    make && make check
else
    make && make dist && make distcheck
fi

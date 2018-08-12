#!/bin/bash -x

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

pip install --user Cheetah # required for dbus support, system (distro) wide include paths are ignored
sudo apt-get update -q
sudo apt-get -y -q -V --no-install-recommends install \
     build-essential \
     automake \
     autoconf \
     autoconf-archive \
     libtool \
     autopoint \
     intltool \
     libxtst-dev \
     libxss-dev \
     gobject-introspection \
     `conf_opt_packages xml libgdome2-dev` \
     `conf_opt_packages gconf libgconf2-dev` \
     `[[ $CONF_GTK_VER = 2 ]] && echo libgtk2.0-dev libgtkmm-2.4-dev` \
     `[[ $CONF_GTK_VER = 3 ]] && echo libgtk-3-dev libgtkmm-3.0-dev` \
     `conf_opt_packages gnome3 libglib2.0-dev libpanel-applet-4-dev` \
     `conf_opt_packages dbus libdbus-glib-1-dev` \
     `conf_opt_packages gstreamer libgstreamer1.0-dev` \
     `conf_opt_packages pulse libpulse-dev` \
     `conf_opt_packages indicator gobject-introspection libindicator3-dev libdbusmenu-glib-dev libdbusmenu-gtk3-dev libgirepository1.0-dev` \
     `conf_opt_packages xfce xfce4-panel-dev` \
     `conf_opt_packages manual docbook-utils xmlto`

./autogen.sh
./configure \
    --enable-option-checking=fatal \
    --disable-static \
    --disable-dependency-tracking \
    `conf_opt gconf` \
    `conf_opt gsettings` \
    `conf_opt xml` \
    `conf_opt gnome2` \
    `conf_opt gnome3` \
    `conf_opt indicator` \
    `conf_opt xfce` \
    `conf_opt mate` \
    `conf_opt dbus` \
    `conf_opt distribution` \
    `conf_opt gstreamer` \
    `conf_opt pulse` \
    `conf_opt exercises` \
    `conf_opt experimental` \
    `conf_opt manual`

make && make check

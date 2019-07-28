FROM ubuntu:xenial

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get update -y && \
  apt-get install -y \
    autoconf \
    autoconf-archive \
    automake \
    autopoint \
    build-essential \
    cmake \
    debhelper \
    devscripts \
    dh-autoreconf \
    gettext \
    git \
    gobject-introspection \
    gsettings-desktop-schemas-dev \
    intltool \
    jq \
    libboost-chrono-dev \
    libboost-context-dev \
    libboost-coroutine-dev \
    libboost-date-time-dev \
    libboost-dev \
    libboost-filesystem-dev \
    libboost-log-dev \
    libboost-regex-dev \
    libboost-system-dev \
    libboost-test-dev \
    libboost-thread-dev \
    libdbusmenu-glib-dev \
    libdbusmenu-gtk3-dev \
    libgirepository1.0-dev \
    libgstreamer1.0-dev \
    libgtk-3-dev \
    libgtkmm-3.0-dev \
    libindicator3-dev \
    libmate-panel-applet-dev \
    libpanel-applet-dev \
    libpulse-dev \
    libqt5svg5-dev \
    libqt5x11extras5-dev \
    libssl-dev \
    libtool \
    libxi-dev \
    libxmu-dev \
    libxss-dev \
    libxtst-dev \
    python-cheetah \
    python3-pip \
    python3-setuptools \
    qtbase5-dev \
    qtbase5-private-dev \
    qttools5-dev-tools \
    xfce4-panel-dev && \
  apt-get clean

RUN pip3 install --upgrade pip
RUN pip3 install Jinja2

CMD ["/bin/bash"]

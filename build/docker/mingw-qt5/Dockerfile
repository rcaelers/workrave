FROM fedora:28

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN dnf -y update && dnf clean all
RUN dnf -y install \
                cmake \
                git \
                intltool \
                make \
                mingw64-boost \
                mingw64-dbus \
                mingw64-gcc \
                mingw64-gcc-c++ \
                mingw64-qt5-qttools \
                mingw64-qt5-qtbase-devel \
                mingw64-qt5-qtsvg \
                tar \
                xz

RUN mkdir -p /workspace && \
    cd /workspace && \
    curl -O https://snapshots.workrave.org/.prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN pip3 install --upgrade pip
RUN pip3 install Jinja2

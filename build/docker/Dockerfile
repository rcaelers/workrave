FROM fedora:23

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN dnf -y update && dnf clean all
RUN dnf -y install \
                boost-devel \
                clang \
                cmake \
                gcc \
                gcc-c++ \
                gettext-devel \
                git \
                gstreamer1-devel \
                gtkmm24-devel \
                gtkmm30-devel \
                intltool \
                libXScrnSaver-devel \
                libXtst-devel \
                libdbusmenu-devel \
                libdbusmenu-gtk3-devel \
                libindicator-devel \
                libindicator-gtk3-devel \
                libtool \
                make \
                mate-panel-devel \
                mingw32-boost \
                mingw32-dbus \
                mingw32-gcc \
                mingw32-gcc-c++ \
                mingw32-gettext \
                mingw32-gtkmm24 \
                mingw32-gtkmm30 \
                mingw32-openssl \
                mingw32-qt5-qttools \
                mingw32-qt5-qtbase-devel \
                mingw32-qt5-qtsvg \
                mingw32-boost \
                mingw64-dbus \
                mingw64-gcc \
                mingw64-gcc-c++ \
                mingw64-gettext \
                mingw64-openssl \
                mingw64-qt5-qttools \
                mingw64-qt5-qtbase-devel \
                mingw64-qt5-qtsvg \
                openssl-devel \
                pulseaudio-libs-devel \
                python-cheetah \
                qt5-qtbase-devel \
                qt5-qtsvg-devel \
                qt5-qttools-devel \
                tar \
                which \
                wine \
                xorg-x11-server-Xvfb \
                xorg-x11-xauth \
                xz
                
RUN curl -O http://files.jrsoftware.org/is/5/isetup-5.5.5-unicode.exe
RUN xvfb-run wine isetup-5.5.5-unicode.exe /SILENT /VERYSILENT /SP- /SUPPRESSMSGBOXES /NORESTART

# RUN innoextract -d /inno /isetup-5.5.5-unicode.exe

RUN mkdir /build 
RUN cd /build && curl -O http://snapshots.workrave.org/.prebuilt/workrave-win32-prebuilt.tar.xz && tar xvfJ ./workrave-win32-prebuilt.tar.xz

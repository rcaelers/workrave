FROM fedora:rawhide

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN dnf -y update && dnf clean all
RUN dnf -y install \
                cmake \
                git \
                innoextract \
                intltool \
                make \
                mingw32-boost \
                mingw32-dbus \
                mingw32-gcc \
                mingw32-gcc-c++ \
                mingw32-gettext \
                mingw32-gtkmm30 \
                mingw32-librsvg2 \
                mingw32-boost \
                mingw32-openssl \
                mingw64-gcc \
                mingw64-gcc-c++ \
                gtk-update-icon-cache \
                tar \
                which \
                wine \
                xz

RUN mkdir -p /workspace

RUN cd /workspace && \
    curl -O http://files.jrsoftware.org/is/5/innoisetup-5.5.9-unicode.exe && \
    innoextract -d /workspace/inno ./innosetup-5.5.9-unicode.exe && rm ./innosetup-5.5.9-unicode.exe

RUN cd /workspace && \
    curl -O https://snapshots.workrave.org/.prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN pip3 install --upgrade pip
RUN pip3 install Jinja2 # ==2.8

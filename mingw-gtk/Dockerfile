FROM fedora:31

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN echo "%_install_langs all" >/etc/rpm/macros.image-language-conf && \
    dnf -y update && dnf clean all && \
    dnf -y install \
                cmake \
                git \
                gtk-update-icon-cache \
                innoextract \
                intltool \
                jq \
                make \
                mingw32-boost \
                mingw32-dbus \
                mingw32-gcc \
                mingw32-gcc-c++ \
                mingw32-gettext \
                mingw32-gtkmm30 \
                mingw32-librsvg2 \
                mingw32-openssl \
                mingw32-adwaita-icon-theme \
                mingw64-boost \
                mingw64-dbus \
                mingw64-gcc \
                mingw64-gcc-c++ \
                mingw64-gettext \
                mingw64-gtkmm30 \
                mingw64-librsvg2 \
                mingw64-openssl \
                mingw64-adwaita-icon-theme \
                python3-pip \
                tar \
                which \
                wine \
                xz

RUN mkdir -p /workspace

RUN cd /workspace && \
    curl -O http://files.jrsoftware.org/is/5/innosetup-5.5.9-unicode.exe && \
    innoextract -d /workspace/inno ./innosetup-5.5.9-unicode.exe && rm ./innosetup-5.5.9-unicode.exe

RUN cd /workspace && \
    curl -O https://snapshots.workrave.org/prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN pip3 install --upgrade pip
RUN pip3 install Jinja2

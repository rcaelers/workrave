FROM fedora:33

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN echo "%_install_langs all" >/etc/rpm/macros.image-language-conf && \
    dnf -y upgrade && dnf clean all && \
    dnf -y install \
                autoconf-archive \
                bzip2 \
                clang \
                cmake \
                dos2unix \
                gcc \
                git \
                gobject-introspection-devel \
                gtk-update-icon-cache \
                innoextract \
                intltool \
                jq \
                libtool \
                lld \
                make \
                mingw32-boost \
                mingw32-dbus \
                mingw32-gcc \
                mingw32-gcc-c++ \
                mingw32-gettext \
                mingw32-gtkmm30 \
                mingw32-gtkmm24 \
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
    curl -O http://files.jrsoftware.org/is/5/innosetup-5.6.1-unicode.exe && \
    innoextract -d /workspace/inno ./innosetup-5.6.1-unicode.exe && rm ./innosetup-5.6.1-unicode.exe

RUN cd /workspace && \
    curl -O https://snapshots.workrave.org/prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN cd /workspace && \
    git clone https://github.com/tpoechtrager/wclang.git && \
    cd /workspace/wclang && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local/ && \
    make && \
    make install && \
    rm -rf /workspace/wclang

RUN cd /workspace && \
    curl -OL https://github.com/mozilla/dump_syms/releases/download/v0.0.7/dump_syms-linux-x86_64.tar.gz && \
    tar xvfz dump_syms-linux-x86_64.tar.gz && \
    cp -a dump_syms-linux-x86_64/dump_syms /usr/local/bin && \
    rm -rf dump_syms-linux-x86_64.tar.gz dump_syms-linux-x86_64

RUN pip3 install --upgrade pip
RUN pip3 install Jinja2

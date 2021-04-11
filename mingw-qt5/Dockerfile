FROM fedora:33

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN echo "%_install_langs all" >/etc/rpm/macros.image-language-conf && \
    dnf -y upgrade && dnf clean all && \
    dnf -y install \
                bzip2 \
                clang \
                cmake \
                dos2unix \
                gcc \
                git \
                intltool \
                jq \
                lld \
                make \
                mingw32-gcc \
                mingw32-gcc-c++ \
                mingw64-boost \
                mingw64-dbus \
                mingw64-gcc \
                mingw64-gcc-c++ \
                mingw64-qt5-qttools \
                mingw64-qt5-qtbase-devel \
                mingw64-qt5-qtsvg \
                ninja-build \
                python3-pip \
                tar \
                xz

RUN mkdir -p /workspace && \
    cd /workspace && \
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

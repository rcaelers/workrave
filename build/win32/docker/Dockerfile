FROM ubuntu:trusty

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN export DEBIAN_FRONTEND=noninteractive && \
    echo 'Acquire::Languages "none";' | tee /etc/apt/apt.conf.d/99translations > /dev/null && \
    dpkg --add-architecture i386 && \
    apt-get update -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:arx/release && \
    apt-get update -y && \
    apt-get install -y \
        make \
        autoconf \
        autoconf-archive \
        automake \
        autopoint \
        autotools-dev \
        libtool \
        intltool \
        pkg-config \
        git \
        gcc-mingw-w64 \
        g++-mingw-w64 \
        wine \
        cmake \
        innoextract \
        gobject-introspection \
        libglib2.0-bin \
        dos2unix \
        xvfb \
        wget \
        curl

RUN mkdir /workspace && \
    cd /workspace && \
    curl -O https://snapshots.workrave.org/.prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN mkdir -p /workspace/inno && \
    cd /workspace && \
    curl -O http://files.jrsoftware.org/is/5/isetup-5.5.8-unicode.exe && \
    innoextract -d /workspace/inno ./isetup-5.5.8-unicode.exe && rm ./isetup-5.5.8-unicode.exe

RUN mkdir -p /workspace/source && \
    cd /workspace/source && \
    git clone https://github.com/rcaelers/workrave.git

RUN mkdir -p /workspace/runtime && \
    cd /workspace/runtime && \
    cp -a /workspace/source/workrave/build/win32/crossbuilder/* . &&  \
    bash -x build-packages.sh && \
    rm -rf source build

COPY Makefile env make-runtime.sh /workspace/

RUN mkdir /root/.ssh && echo "StrictHostKeyChecking no " > /root/.ssh/config

ENTRYPOINT ["make", "-C", "/workspace/"]

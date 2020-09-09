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
        autoconf \
        automake \
        autopoint \
        autotools-dev \
        build-essential \
        cmake \
        curl \
        dos2unix \ 
        g++-mingw-w64 \
        gcc-mingw-w64 \
        git \
        gobject-introspection \
        innoextract \
        intltool \
        libglib2.0-bin \
        libtool \
        make \
        pkg-config \
        python3-jinja2 \
        wget \
        wine \
        xvfb


RUN mkdir /workspace && \
    cd /workspace && \
    curl -O https://snapshots.workrave.org/prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

RUN mkdir -p /workspace/inno && \
    cd /workspace && \
    curl -O http://files.jrsoftware.org/is/5/isetup-5.5.8-unicode.exe && \
    innoextract -d /workspace/inno ./isetup-5.5.8-unicode.exe && rm ./isetup-5.5.8-unicode.exe

RUN mkdir -p /workspace/autoconf-archive && \
    cd /workspace/autoconf-archive && \
    curl -O http://ftp.snt.utwente.nl/pub/software/gnu/autoconf-archive/autoconf-archive-2019.01.06.tar.xz && \
    tar xvfJ ./autoconf-archive-2019.01.06.tar.xz && \
    cd /workspace/autoconf-archive/autoconf-archive-2019.01.06 && \
    ./configure && make install && \
    rm -rf /workspace/autoconf-archive

RUN mkdir -p /workspace/jq && \
    cd /workspace/jq && \
    curl -OL https://github.com/stedolan/jq/releases/download/jq-1.6/jq-linux64 && \
    chmod +x jq-linux64 && \
    mv jq-linux64 /usr/local/bin/jq && \
    rm -rf /workspace/jq

RUN mkdir -p /workspace/runtime

COPY crossbuilder/* /workspace/runtime/

RUN cd /workspace/runtime && \
    bash -x build-packages.sh && \
    rm -rf source build

RUN mkdir -p /workspace/source /workspace/scripts

COPY env make-runtime.sh /workspace/scripts/

RUN mkdir /root/.ssh && echo "StrictHostKeyChecking no " > /root/.ssh/config

CMD ["/bin/bash"]

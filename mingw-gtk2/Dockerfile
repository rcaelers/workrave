FROM ubuntu:focal

MAINTAINER Rob Caelers <rob.caelers@gmail.com>

RUN sed -i 's:^path-exclude=/usr/share/locale:#path-exclude=/usr/share/locale:' /etc/dpkg/dpkg.cfg.d/excludes && \
    export DEBIAN_FRONTEND=noninteractive && \
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
        g++ \
        git \
        gobject-introspection \
        intltool \
        libboost-iostreams-dev \
        libboost-filesystem-dev \
        libboost-date-time-dev \
        libboost-system-dev \
        libboost-program-options-dev \
        liblzma-dev \
        libglib2.0-bin \
        libtool \
        make \
        pkg-config \
        python3-jinja2 \
        unzip \
        wget \
        wine \
        xvfb \
        zip

RUN mkdir -p /workspace/innoextract && \
    cd /workspace/innoextract && \
    curl -O https://constexpr.org/innoextract/files/innoextract-1.9/innoextract-1.9.tar.gz && \
    tar xvfz ./innoextract-1.9.tar.gz && \
    mkdir -p /workspace/innoextract/build && \
    cd /workspace/innoextract/build && \
    cmake ../innoextract-1.9 && make install && \
    rm -rf /workspace/innoextract

RUN mkdir -p /workspace/inno && \
    cd /workspace && \
    curl -O http://files.jrsoftware.org/is/6/innosetup-6.1.2.exe && \
    /usr/local/bin/innoextract -d /workspace/inno ./innosetup-6.1.2.exe && rm ./innosetup-6.1.2.exe

RUN cd /workspace && \
    curl -O https://snapshots.workrave.org/prebuilt/workrave-prebuilt-windows-vs2015.tar.xz && \
    tar xvfJ ./workrave-prebuilt-windows-vs2015.tar.xz && \
    rm ./workrave-prebuilt-windows-vs2015.tar.xz && \
    ln -s workrave-prebuilt-windows-vs2015 prebuilt

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

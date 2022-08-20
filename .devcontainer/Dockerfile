ARG VARIANT="35"
FROM fedora:${VARIANT}

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    echo $USERNAME ALL=\(root\) NOPASSWD:ALL >/etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

RUN echo "%_install_langs all" >/etc/rpm/macros.image-language-conf && \
    dnf clean all && \
    dnf -y update && \
    dnf -y install \
    bzip2 \
    clang \
    clang-tools-extra \
    cmake \
    dos2unix \
    gcc \
    gettext \
    git \
    gobject-introspection-devel \
    gtk-update-icon-cache \
    innoextract \
    jq \
    lld \
    mingw32-boost \
    mingw32-boost-static \
    mingw32-gcc \
    mingw32-gcc-c++ \
    mingw64-adwaita-icon-theme \
    mingw64-boost \
    mingw64-boost-static \
    mingw64-gcc \
    mingw64-gcc-c++ \
    mingw64-gettext \
    mingw64-gtkmm30 \
    mingw64-librsvg2 \
    ninja-build \
    openssl1.1 \
    python3-jinja2 \
    python3-pip \
    strace \
    tar \
    which \
    wine-core.i686 \
    xz && \
    dnf -y clean all

RUN mkdir -p /home/vscode/.local/share/CMakeTools && \
    echo '[{"name":"MinGW","toolchainFile": "${workspaceFolder}/cmake/toolchains/mingw64-clang.cmake"}]' >/home/vscode/.local/share/CMakeTools/cmake-tools-kits.json && \
    chown vscode /home/vscode/.local/share/CMakeTools/cmake-tools-kits.json

RUN mkdir -p /workspace

RUN cd /workspace && \
    curl -O http://files.jrsoftware.org/is/6/innosetup-6.2.0.exe && \
    innoextract -d /workspace/inno ./innosetup-6.2.0.exe && \
    rm ./innosetup-6.2.0.exe

RUN cd /workspace && \
    curl -OL https://github.com/mozilla/dump_syms/releases/download/v0.0.7/dump_syms-linux-x86_64.tar.gz && \
    tar xvfz dump_syms-linux-x86_64.tar.gz && \
    cp -a dump_syms-linux-x86_64/dump_syms /usr/local/bin && \
    rm -rf dump_syms-linux-x86_64.tar.gz dump_syms-linux-x86_64

USER "$USER_UID"

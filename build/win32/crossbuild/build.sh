#!/bin/bash

export SYSROOT="/mnt/bulk/sources/workrave/runtime/usr/i686-w64-mingw32/sys-root/mingw/"
WORKRAVE_SRC_DIR=${HOME}/src/workrave-next
TINDERBOX_HOME=${HOME}/src/tinderbox

WIN32_MAKERUNTIME=${WORKRAVE_SRC_DIR}/build/win32/runtime/make-runtime.sh
WIN32_ENV=${WORKRAVE_SRC_DIR}/build/win32/runtime/env.sh

WIN32_ISCC="${HOME}/.wine/drive_c/Program Files (x86)/Inno Setup 5/ISCC.exe"

export TINDERBOX_BUILD=yes

export PATH=/mnt/bulk/sources/workrave/runtime/toolchain/bin:$PATH

setup_prebuilt()
{
    mkdir -p libs/hooks/harpoonHelper/src/Release
    cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/harpoonHelper.exe libs/hooks/harpoonHelper/src/Release

    mkdir -p libs/hooks/harpoon/src/Release
    cp -a ${TINDERBOX_HOME}/prebuilt/Debug/harpoon.dll libs/hooks/harpoon/src/Release
    cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/harpoon64.dll libs/hooks/harpoon/src/Release

    mkdir -p frontend/applets/win32/src/Release
    cp -a ${TINDERBOX_HOME}/prebuilt/Debug/workrave-applet.dll frontend/applets/win32/src/Release
    cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/workrave-applet.dll frontend/applets/win32/src/Release/workrave-applet64.dll
}

setup_runtime()
{
    ( cd frontend/gtkmm/win32/setup/
      ${WIN32_MAKERUNTIME}
    )
}

setup_configure()
{
    conf_flags="--with-sysroot=${SYSROOT} --target=i686-w64-mingw32 --host=i686-w64-mingw32 --build=i386-linux --enable-maintainer-mode --enable-debug --without-x --enable-distribution --enable-exercises --disable-gstreamer --enable-dbus --with-boost=${SYSROOT}"
 
    echo Running $WORKRAVE_SRC_DIR/configure $conf_flags
    $WORKRAVE_SRC_DIR/configure $conf_flags
}

create_installer()
{
    cp -a frontend/gtkmm/src/.libs/workrave.exe frontend/gtkmm/src
    
    ( cd frontend/gtkmm/win32/setup
      unix2dos setup.iss
      wine "${WIN32_ISCC}" setup.iss
    )
}

. ${WIN32_ENV}

#setup_prebuilt
#setup_runtime
#setup_configure

#make || exit 1

create_installer


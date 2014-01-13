#!/bin/bash
TINDERBOX_HOME=${HOME}/src/tinderbox
WORKRAVE_GIT_DIR=${HOME}/src/workrave-next

WIN32_MAKERUNTIME=${TINDERBOX_HOME}/bin/make-runtime.sh
WIN32_MINGW_ENV=${TINDERBOX_HOME}/bin/mingw32
WIN32_ISCC="${HOME}/.wine/drive_c/Program Files (x86)/Inno Setup 5/ISCC.exe"

export TINDERBOX_BUILD=yes

unix2dos=perl -e 'while (<>) { s/$$/\r/; print; }'

cd ${WORKRAVE_GIT_DIR}/frontend/gtkmm/win32/setup/
${WIN32_MAKERUNTIME} ${WORKRAVE_GIT_DIR}

cd ${WORKRAVE_GIT_DIR}/common/win32/harpoonHelper/src
mkdir Release
cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/harpoonHelper.exe Release
cd ${WORKRAVE_GIT_DIR}/common/win32/harpoon/src
mkdir Release
cp -a ${TINDERBOX_HOME}/prebuilt/Debug/harpoon.dll Release
cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/harpoon64.dll Release
cd ${WORKRAVE_GIT_DIR}/frontend/applets/win32/src
mkdir Release;
cp -a ${TINDERBOX_HOME}/prebuilt/Debug/workrave-applet.dll Release
cp -a ${TINDERBOX_HOME}/prebuilt/Debug64/workrave-applet.dll Release/workrave-applet64.dll

cd ${WORKRAVE_GIT_DIR}
. ${WIN32_MINGW_ENV}
${WORKRAVE_GIT_DIR}/build/win32/autogencross.sh || exit 1

make || exit 1
cp -a ${WORKRAVE_GIT_DIR}/frontend/gtkmm/src/.libs/workrave.exe ${WORKRAVE_GIT_DIR}/frontend/gtkmm/src

cd ${WORKRAVE_GIT_DIR}/frontend/gtkmm/win32/setup
unix2dos setup.iss
wine "${WIN32_ISCC}" setup.iss

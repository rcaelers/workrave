#!/bin/bash -e

BUILD_DIR=_dist/build
DEBIAN_PACKAGING_DIR=_dist/debian-packaging

mkdir -p ${BUILD_DIR}
mkdir -p ${DEBIAN_PACKAGING_DIR}

git worktree add -B debian-packaging ${DEBIAN_PACKAGING_DIR} origin/debian-packaging

SOURCE="workrave-*.tar.gz"
VERSION=`echo $SOURCE | sed -e 's/.*-\(.*\).tar.gz/\1/'`

echo "Preparing build environment"

tar xzfC "$SOURCE" "$BUILD"
cp -a "${DEBIAN_PACKAGING_DIR}/debian" "$BUILD/workrave-$VERSION/debian"
cp -a "$SOURCE" "$BUILD/workrave_$VERSION.orig.tar.gz"

for series in cosmic # bionic artful xenial trusty
do
    echo Create $series source package

    rm -rf "$BUILD/$series"
    mkdir -p "$BUILD/$series"

    cp -a "$BUILD/workrave-$VERSION" "$BUILD/$series"
    ln "$SOURCE" "$BUILD/$series/workrave_$VERSION.orig.tar.gz"

    if [ -d "${DEBIAN_PACKAGING_DIR}/debian-${series}" ]; then
        cp -a "${DEBIAN_PACKAGING_DIR}/debian-${series}/*" "$BUILD/$series/workrave-$VERSION/debian/"
    fi

    pushd .
    cd "$BUILD/$series/workrave-$VERSION"

    LAST_VERSION=`dpkg-parsechangelog | sed -n -e 's/^Version: \(.*\)/\1/p'`

    dch -D "$series" --force-distribution -v "${LAST_VERSION}ppa1~$series}1"

    debuild -S -sa -us -uc -j8

    # debuild -d -S -sa -k3300F30F -j12
done

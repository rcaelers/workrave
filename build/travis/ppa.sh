#!/bin/bash -e

export DEBEMAIL="robc@krandor.org"

BUILD_DIR=/workspace/source/_dist/build
DEBIAN_PACKAGING_DIR=/workspace/source/_dist/debian-packaging

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
mkdir -p ${DEBIAN_PACKAGING_DIR}

cd /workspace/source

git remote set-branches --add origin debian-packaging
git fetch

git worktree add -B debian-packaging ${DEBIAN_PACKAGING_DIR} origin/debian-packaging

apt-get update -q
apt-get -y -q -V --no-install-recommends install python-cheetah

./autogen.sh
./configure

make dist

SOURCE=`ls workrave-*.tar.gz`
VERSION=`echo $SOURCE | sed -e 's/.*-\(.*\).tar.gz/\1/'`

tar xzfC "$SOURCE" "$BUILD_DIR"
cp -a "${DEBIAN_PACKAGING_DIR}/debian" "$BUILD_DIR/workrave-$VERSION/debian"
cp -a "$SOURCE" "$BUILD_DIR/workrave_$VERSION.orig.tar.gz"

# gpg --list-secret-keys --keyid-format LONG
# gpg --output pubring.gpg --export D5A196C1776BD06C
# gpg --output secring.gpg --export-secret-key D5A196C1776BD06C
# travis encrypt-file secring.gpg

echo allow-loopback-pinentry >> ~/.gnupg/gpg-agent.conf

gpg --import build/travis/pubring.gpg
gpg --passphrase $GPG_PASSPHRASE --batch --no-tty --yes --pinentry-mode loopback --allow-secret-key-import --import build/travis/secring.gpg

for series in cosmic bionic artful xenial trusty
do
    echo Create $series source package
    cd /workspace/source

    rm -rf "$BUILD_DIR/$series"
    mkdir -p "$BUILD_DIR/$series"

    cp -a "$BUILD_DIR/workrave-$VERSION" "$BUILD_DIR/$series"
    ln "$SOURCE" "$BUILD_DIR/$series/workrave_$VERSION.orig.tar.gz"

    if [ -d "${DEBIAN_PACKAGING_DIR}/debian-${series}" ]; then
        cp -a "${DEBIAN_PACKAGING_DIR}/debian-${series}"/* "$BUILD_DIR/$series/workrave-$VERSION/debian/"
    fi

    cd "$BUILD_DIR/$series/workrave-$VERSION"

    LAST_VERSION=`dpkg-parsechangelog | sed -n -e 's/^Version: \(.*\)/\1/p'`

    # TODO: process news
    NEWS=`awk '/${VERSION}/{f=1} /${LAST_VERSION}/{f=0} f' < NEWS`
    echo $NEWS

    dch -b -D "$series" --force-distribution -v "${VERSION}-ppa1~${series}1" "New release"

    yes $GPG_PASSPHRASE | debuild -p"gpg --batch --pinentry-mode loopback --passphrase-fd 0" -d -S -sa -kD5A196C1776BD06C -j8 --lintian-opts --suppress-tags bad-distribution-in-changes-file

    if [[ -z "$TRAVIS_TAG" ]]; then
        echo "No tag build."
        dpkg-buildpackage -rfakeroot
    else
        echo "Tag build : $TRAVIS_TAG"
        # dput ppa:rob-caelers/workrave ../workrave_*_source.changes
    fi

done

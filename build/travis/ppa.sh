#!/bin/bash -e

export DEBFULLNAME="Rob Caelers"
export DEBEMAIL="robc@krandor.org"

SOURCE_DIR=/workspace/source
BUILD_DIR=/workspace/source/_dist/build
DEBIAN_PACKAGING_DIR=/workspace/source/_dist/debian-packaging

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
mkdir -p ${DEBIAN_PACKAGING_DIR}

cd ${SOURCE_DIR}

mkdir -p ~/.gnupg
chmod 600 ~/.gnupg

# Workaround for 'error building skey array: No such file or directory'
mkdir -p /home/robc/.gnupg/private-keys-v1.d

echo allow-loopback-pinentry > ~/.gnupg/gpg-agent.conf

gpg --import ${SOURCE_DIR}/build/travis/pubring.gpg
gpg --passphrase-file /tmp/secrets/priv-key --batch --no-tty --yes --pinentry-mode loopback --allow-secret-key-import --import /tmp/secrets/secring.gpg

git remote set-branches --add origin debian-packaging
git fetch

git worktree add -B debian-packaging ${DEBIAN_PACKAGING_DIR} origin/debian-packaging

./autogen.sh
./configure

make dist

apt-get update -q
apt-get -y -q -V --no-install-recommends install devscripts libxfce4panel-2.0-dev

SOURCE=`ls workrave-*.tar.gz`
VERSION=`echo $SOURCE | sed -e 's/.*-\(.*\).tar.gz/\1/'`

tar xzfC "$SOURCE" "$BUILD_DIR"
cp -a "${DEBIAN_PACKAGING_DIR}/debian" "$BUILD_DIR/workrave-$VERSION/debian"
cp -a "$SOURCE" "$BUILD_DIR/workrave_$VERSION.orig.tar.gz"

# gpg --list-secret-keys --keyid-format LONG
# gpg --output pubring.gpg --export 9D5F98D3149A28DB
# gpg --output secring.gpg --export-secret-key 9D5F98D3149A28DB
# travis encrypt-file secring.gpg

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

    LAST_DEB_VERSION=`dpkg-parsechangelog | sed -n -e 's/^Version: \(.*\)/\1/p'`
    LAST_VERSION=`echo $LAST_DEB_VERSION | sed -e 's/^\(.*\)-.*$/\1/'`
    LAST_PPA_VERSION=`echo $LAST_DEB_VERSION | sed -e 's/^[^-]\+-ppa\([0-9]\+\).*$/\1/'`

    # TODO: process news
    NEWS=`awk '/${VERSION}/{f=1} /${LAST_VERSION}/{f=0} f' < NEWS`
    echo $NEWS

    PPA=$LAST_PPA_VERSION
    if [ "x$VERSION" != "x$LAST_VERSION" ]; then
        PPA=1
    fi

    dch -b -D "$series" --force-distribution -v "${VERSION}-ppa${PPA}~${series}1" "New release"

    debuild -p"gpg --passphrase-file /tmp/secrets/priv-key --batch --no-tty --yes --pinentry-mode loopback" -d -S -sa -k9D5F98D3149A28DB -j8 --lintian-opts --suppress-tags bad-distribution-in-changes-file

    if [[ -z "$TRAVIS_TAG" ]]; then
        echo "No tag build."
        if [[ $series == "bionic" ]]; then
            dpkg-buildpackage -rfakeroot -us -uc
        fi
    else
        echo "Tag build : $TRAVIS_TAG"
        cd "$BUILD_DIR/$series"
        dput ppa:rob-caelers/workrave workrave_*_source.changes
    fi
done

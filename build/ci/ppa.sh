#!/bin/bash -e

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

cd ${SOURCES_DIR}

mkdir -p ~/.gnupg
chmod 600 ~/.gnupg

# Workaround for 'error building skey array: No such file or directory'
mkdir -p ~/.gnupg/private-keys-v1.d

echo allow-loopback-pinentry > ~/.gnupg/gpg-agent.conf

gpg --import ${SECRETS_DIR}/pubring.gpg
gpg --passphrase-file ${SECRETS_DIR}/priv-key --batch --no-tty --yes --pinentry-mode loopback --allow-secret-key-import --import ${SECRETS_DIR}/secring.gpg

if [ -d $WORKSPACE/debian ] ; then
    DEBIAN_PACKAGING_DIR=$WORKSPACE/debian
else
    DEBIAN_PACKAGING_DIR=${SOURCES_DIR}/_dist/debian-packaging
    mkdir -p ${DEBIAN_PACKAGING_DIR}

    git remote set-branches --add origin debian-packaging
    git fetch

    git worktree add -B debian-packaging ${DEBIAN_PACKAGING_DIR} origin/debian-packaging
fi

./autogen.sh
./configure

make dist

apt-get update -q
apt-get -y -q -V --no-install-recommends install python3-paramiko python3-jinja2

SOURCE=`ls workrave-*.tar.gz`
VERSION=`echo $SOURCE | sed -e 's/.*-\(.*\).tar.gz/\1/'`

NATIVE_UBUNTU=`echo $DOCKER_IMAGE | cut -d- -f2`

tar xzfC "$SOURCE" "$BUILD_DIR"
cp -a "${DEBIAN_PACKAGING_DIR}/debian" "$BUILD_DIR/workrave-$VERSION/debian"
cp -a "$SOURCE" "$BUILD_DIR/workrave_$VERSION.orig.tar.gz"

# gpg --list-secret-keys --keyid-format LONG
# gpg --output pubring.gpg --export 9D5F98D3149A28DB
# gpg --output secring.gpg --export-secret-key 9D5F98D3149A28DB
# ssh-keygen -t rsa -f ./deploy_rsa
# travis encrypt-file secrets.tar

#cat > ~/.dput.cf <<EOF
#[workraveppa]
#fqdn        = ppa.launchpad.net
#method      = sftp
#incoming    = ~rob-caelers/ubuntu/workrave/
#login       = rob-caelers
#allow_unsigned_uploads = 0
#EOF
#echo DPUT config:
#cat ~/.dput.cfs
#
#mkdir -p ~/.ssh/
#ssh-keyscan ppa.launchpad.net >> ~/.ssh/known_hosts
#
#cat >> ~/.ssh/config <<EOF
#Host ppa.launchpad.net
#    StrictHostKeyChecking no
#    IdentityFile ${SECRETS_DIR}/deploy_rsa
#EOF
#chmod 400 ~/.ssh/config
#echo SSH config:
#cat ~/.ssh/config

for series in groovy focal eoan disco bionic xenial trusty
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
    if [[ $series == $NATIVE_UBUNTU ]]; then
        cp -a "$BUILD_DIR/workrave-$VERSION/debian/changelog" "${DEBIAN_PACKAGING_DIR}/debian"
    fi

    debuild -p"gpg --passphrase-file ${SECRETS_DIR}/priv-key --batch --no-tty --yes --pinentry-mode loopback" -d -S -sa -kEC02F3CD5A24B1DE -j8 --lintian-opts --suppress-tags bad-distribution-in-changes-file

    if [[ -z "$WORKRAVE_RELEASE_TAG" ]]; then
        echo "No tag build."
        if [[ $series == $NATIVE_UBUNTU ]]; then
            dpkg-buildpackage -rfakeroot -us -uc
        fi
    else
        echo "Tag build : WORKRAVE_RELEASE_TAG"
        cd "$BUILD_DIR/$series"
        echo dput -d $WORKRAVE_PPA workrave_*_source.changes
    fi
done

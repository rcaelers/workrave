#!/bin/bash -ex

BASEDIR=$(dirname "$0")
source ${BASEDIR}/../ci/config.sh

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {
    while getopts "p:dP" o; do
        case "${o}" in
        p)
            PPA="${OPTARG}"
            ;;
        d)
            DRYRUN=1
            ;;
        P)
            PRERELEASE=1
            ;;
        *)
            usage
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

init_builddir() {
    if [ ! -d ${BUILD_DIR} ]; then
        mkdir -p ${BUILD_DIR}
    fi
}

init_gpg() {
    mkdir -p ~/.gnupg
    chmod 600 ~/.gnupg

    # Workaround for 'error building skey array: No such file or directory'
    mkdir -p ~/.gnupg/private-keys-v1.d

    echo allow-loopback-pinentry >~/.gnupg/gpg-agent.conf

    gpg --import ${SECRETS_DIR}/pubring.gpg
    gpg --passphrase-file ${SECRETS_DIR}/priv-key --batch --no-tty --yes --pinentry-mode loopback --allow-secret-key-import --import ${SECRETS_DIR}/secring.gpg
}

init_debian_packaging() {
    if [ -d $WORKSPACE/debian ]; then
        DEBIAN_PACKAGING_DIR=$WORKSPACE/debian
    else
        DEBIAN_PACKAGING_DIR=$WORKSPACE/debian-packaging
        mkdir -p ${DEBIAN_PACKAGING_DIR}

        cd ${SOURCES_DIR}
        git remote set-branches --add origin debian-packaging
        git fetch
        git worktree add -B debian-packaging ${DEBIAN_PACKAGING_DIR} origin/debian-packaging
    fi
}

init_dependencies() {
    apt-get update -q
    apt-get -y -q -V --no-install-recommends install libgnome-panel-dev
}

init_newsgen() {
    cd ${SCRIPTS_DIR}/newsgen
    npm install
}

build_tarball() {
    if [ ! -e "$DEPLOY_TARFILE" ]; then
        cd ${SOURCES_DIR}
        ./autogen.sh
        ./configure
        make dist
        cp ${SOURCE_TARFILE} "${DEPLOY_TARFILE}"
    fi
}

build_sources() {
    tar xzfC "${DEPLOY_TARFILE}" "${BUILD_DIR}"
    cp -a "${DEBIAN_PACKAGING_DIR}/debian" "$BUILD_DIR/workrave-${WORKRAVE_VERSION}/debian"
    cp -a "${DEPLOY_TARFILE}" "${BUILD_DIR}/workrave_${WORKRAVE_VERSION}.orig.tar.gz"
}

build_changelog() {
    series=$1

    cd /
    node ${SCRIPTS_DIR}/newsgen/main.js \
        --input "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}/changes.yaml" \
        --ubuntu $series \
        --increment $PPA \
        --release ${WORKRAVE_VERSION} \
        --template debian-changelog \
        --output "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}/debian/changelog"
}

build_setup() {
    series=$1

    cd ${SOURCES_DIR}

    rm -rf "$BUILD_DIR/$series"
    mkdir -p "$BUILD_DIR/$series"

    cp -a "$BUILD_DIR/workrave-${WORKRAVE_VERSION}" "$BUILD_DIR/$series"
    cp ${DEPLOY_TARFILE} "$BUILD_DIR/$series/workrave_${WORKRAVE_VERSION}.orig.tar.gz"

    if [ -d "${DEBIAN_PACKAGING_DIR}/debian-${series}" ]; then
        cp -a "${DEBIAN_PACKAGING_DIR}/debian-${series}"/* "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}/debian/"
    fi
}

build_single() {
    series=$1

    echo Create $series source package

    build_setup $series
    build_changelog $series

    cd "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}"
    debuild -p"gpg --passphrase-file ${SECRETS_DIR}/priv-key --batch --no-tty --yes --pinentry-mode loopback" -d -S -sa -kEC02F3CD5A24B1DE -j8 --lintian-opts --suppress-tags bad-distribution-in-changes-file

    rm -rf "$DEPLOY_DIR/$series"
    mkdir -p "$DEPLOY_DIR/$series"
    ls -la "$BUILD_DIR/$series"
    cp -a $BUILD_DIR/$series/workrave_${WORKRAVE_VERSION}-ppa* "$DEPLOY_DIR/$series"
    cp -a "${DEPLOY_TARFILE}" "$DEPLOY_DIR/$series/workrave_${WORKRAVE_VERSION}.orig.tar.gz"

    if [[ -z "$WORKRAVE_RELEASE_TAG" ]]; then
        echo "No tag build."
        if [[ $series == `lsb_release -cs` ]]; then
            dpkg-buildpackage -b -rfakeroot -us -uc
        fi
    else
        echo "Tag build : $WORKRAVE_RELEASE_TAG"
        cd "$BUILD_DIR/$series"
        if [ -n "$DRYRUN" ]; then
            echo Dryrun.
        elif [ -n "$PRERELEASE" ]; then
            dput -d $WORKRAVE_TESTING_PPA workrave_*_source.changes
        else
            dput -d $WORKRAVE_PPA workrave_*_source.changes
        fi

    fi
}

build_all() {
    for series in mantic lunar jammy focal; do
        build_single $series
    done
    #build_single hirsute
}

DRYRUN=
PRERELEASE=
SOURCE_TARFILE="${SOURCES_DIR}/workrave-${WORKRAVE_VERSION}.tar.gz"
DEPLOY_TARFILE="${DEPLOY_DIR}/workrave-${WORKRAVE_VERSION}.tar.gz"

parse_arguments $*

init_builddir
init_gpg
init_debian_packaging
init_dependencies
init_newsgen

build_tarball
build_sources
build_all

if [ -n ${WORKRAVE_OVERRIDE_GIT_VERSION} ]; then
    rm -rf ${DEPLOY_TARFILE}
fi

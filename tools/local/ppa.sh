#!/bin/bash -ex

BASEDIR=$(dirname "$0")
source ${BASEDIR}/../ci/config.sh
source ${SCRIPTS_DIR}/ci/ship.sh

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

init_debian_packaging() {
    if [ -d $WORKSPACE/debian ]; then
        DEBIAN_PACKAGING_DIR=$WORKSPACE/debian
    else
        DEBIAN_PACKAGING_DIR=$WORKSPACE/debian-packaging
        mkdir -p ${DEBIAN_PACKAGING_DIR}

        cd ${SOURCES_DIR}
        git clone https://github.com/rcaelers/workrave-debian-packaging.git ${DEBIAN_PACKAGING_DIR}
    fi
}

init_dependencies() {
    apt-get update -q
    apt-get -y -q -V --no-install-recommends install ca-certificates curl dirmngr gnupg libgnome-panel-dev
}

init_newsgen() {
    build_ship
}

build_tarball() {
    if [ ! -e "$DEPLOY_TARFILE" ]; then
        cd ${SOURCES_DIR}
        git archive --prefix=workrave-${WORKRAVE_VERSION}/ HEAD | gzip -9 >"${SOURCE_TARFILE}"
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
    run_ship newsgen \
        --input "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}/changes.yaml" \
        --ubuntu $series \
        --increment $PPA \
        --template debian-changelog \
        --output "$BUILD_DIR/$series/workrave-${WORKRAVE_VERSION}/debian/changelog"
}

ensure_gpg_public_key() {
    local key_id=$WORKRAVE_GPG_KEY_ID
    local public_key_url=${WORKRAVE_GPG_PUBLIC_KEY_URL:-${SIGNING_SERVICE_URL}/sign/gpg/public-key}

    if gpg --batch --list-keys "$key_id" >/dev/null 2>&1; then
        return
    fi

    echo "Importing GPG public key from $public_key_url for dput signature verification"
    curl -skfSL "$public_key_url" | gpg --batch --import

    gpg --batch --list-keys "$key_id" >/dev/null
}

build_setup() {
    series=$1

    cd ${SOURCES_DIR}

    rm -rf "$BUILD_DIR/$series"
    mkdir -p "$BUILD_DIR/$series"

    cp -a "$BUILD_DIR/workrave-${WORKRAVE_VERSION}" "$BUILD_DIR/$series"
    cp ${DEPLOY_TARFILE} "$BUILD_DIR/$series/workrave_${WORKRAVE_DEB_VERSION}.orig.tar.gz"

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
    debuild -p"${BASEDIR}/gpg-sign-client.sh" -d -S -sa -k"$WORKRAVE_GPG_KEY_ID" -j8 --lintian-opts --suppress-tags bad-distribution-in-changes-file

    rm -rf "$DEPLOY_DIR/$series"
    mkdir -p "$DEPLOY_DIR/$series"
    ls -la "$BUILD_DIR/$series"
    cp -a $BUILD_DIR/$series/workrave_${WORKRAVE_DEB_VERSION}-ppa* "$DEPLOY_DIR/$series"
    cp -a "${DEPLOY_TARFILE}" "$DEPLOY_DIR/$series/workrave_${WORKRAVE_DEB_VERSION}.orig.tar.gz"

    if [[ -z "$WORKRAVE_RELEASE_TAG" ]]; then
        echo "No tag build."
        if [[ $series == $(lsb_release -cs) ]]; then
            dpkg-buildpackage -b -rfakeroot -us -uc
        fi
    else
        echo "Tag build : $WORKRAVE_RELEASE_TAG"
        cd "$BUILD_DIR/$series"
        if [ -n "$DRYRUN" ]; then
            echo Dryrun.
        elif [ -n "$PRERELEASE" ]; then
            ensure_gpg_public_key
            dput -d $WORKRAVE_TESTING_PPA workrave_*_source.changes
        else
            ensure_gpg_public_key
            dput -d $WORKRAVE_PPA workrave_*_source.changes
        fi

    fi
}

build_all() {
    for series in resolute questing noble jammy; do
        build_single $series
    done
}

DRYRUN=
PRERELEASE=
SIGNING_SERVICE_URL="${SIGNING_SERVICE_URL:-https://studio.local:50051}"
WORKRAVE_GPG_KEY_ID=${WORKRAVE_GPG_KEY_ID:-009D57DD1AEE3280943BF3E4EC02F3CD5A24B1DE}
WORKRAVE_DEB_VERSION=$(echo ${WORKRAVE_VERSION} | sed -e 's/-/~/g')
SOURCE_TARFILE="${SOURCES_DIR}/workrave-${WORKRAVE_DEB_VERSION}.tar.gz"
DEPLOY_TARFILE="${DEPLOY_DIR}/workrave-${WORKRAVE_DEB_VERSION}.tar.gz"

parse_arguments $*

init_builddir
init_debian_packaging
init_dependencies
init_newsgen

build_tarball
build_sources
build_all

if [ -n ${WORKRAVE_OVERRIDE_GIT_VERSION} ]; then
    rm -rf ${DEPLOY_TARFILE}
fi

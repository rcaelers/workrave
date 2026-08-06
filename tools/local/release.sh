#!/bin/bash -ex

source "$(dirname "${BASH_SOURCE[0]}")/release-common.sh"

run_docker_ppa() {
    if [ -n "$DEBIAN_DIR" ]; then
        DEBVOL="-v $DEBIAN_DIR:/workspace/debian"
    fi
    if [ -n "$PRERELEASE" ] || [ "$CHANNEL" != "stable" ]; then
        PRERELEASE_ARG="-P"
    fi
    docker run --rm \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SCRIPTS_DIR:/workspace/scripts" $DEBVOL \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|SIGNING_SERVICE_URL|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/local/ppa.sh -p $PPA $DRYRUN $PRERELEASE_ARG"
}

run_docker_deb() {
    docker run --rm --privileged \
        -v "$DEPLOY_DIR:/workspace/deploy" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        -e GIT_TAG=$GIT_TAG \
        ghcr.io/rcaelers/workrave-build:ubuntu-pbuilder \
        sh -c "/workspace/scripts/local/pbuild.sh"
}

run_docker_appimage() {
    export CONF_APPIMAGE=1
    mkdir -p $DEPLOY_DIR/$GIT_TAG
    docker run --platform linux/amd64 --rm \
        --cap-add SYS_ADMIN --device /dev/fuse --security-opt apparmor:unconfined \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR/$GIT_TAG:/workspace/deploy/" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/ci/build.sh"

    docker run --platform linux/aarch64 --rm \
        --cap-add SYS_ADMIN --device /dev/fuse --security-opt apparmor:unconfined \
        -v "$SOURCES_DIR:/workspace/source" \
        -v "$DEPLOY_DIR/$GIT_TAG:/workspace/deploy/" \
        -v "$SCRIPTS_DIR:/workspace/scripts" \
        $(printenv | grep -E '^(DOCKER_IMAGE|CONF_.*|WORKRAVE_.*)=' | sed -e 's/^/-e/g') \
        ghcr.io/rcaelers/workrave-build:${DOCKER_IMAGE} \
        sh -c "/workspace/scripts/ci/build.sh"

    unset CONF_APPIMAGE
    unset CONF_SOURCE_TARBALL
}

# Mirrors workrave-dependencies/macos/check-no-homebrew-links.sh: the -M
# dependencies tree is supposed to be self-contained (universal arm64+x86_64,
# buildable on a clean machine), so nothing under it may link against or
# rpath into Homebrew — that would silently make it not actually self-contained.
check_no_homebrew_links() {
    local DEPLOYDIR="$1"
    local BLOCKED='/opt/homebrew|/usr/local/Cellar|/usr/local/opt|/usr/local/lib|/usr/local/bin|/usr/local/share'
    local FOUND=0

    while IFS= read -r -d '' f; do
        if ! file "$f" | grep -q "Mach-O"; then
            continue
        fi

        local deps
        deps=$(otool -L "$f" 2>/dev/null | tail -n +2 | awk '{print $1}' | grep -E "${BLOCKED}" || true)
        if [ -n "${deps}" ]; then
            while IFS= read -r dep; do
                echo "!!! $f links against Homebrew: ${dep}" >&2
            done <<<"${deps}"
            FOUND=1
        fi

        local rpaths
        rpaths=$(otool -l "$f" 2>/dev/null | awk '/LC_RPATH/{getline; getline; print $2}' | grep -E "${BLOCKED}" || true)
        if [ -n "${rpaths}" ]; then
            while IFS= read -r rp; do
                echo "!!! $f has an LC_RPATH into Homebrew: ${rp}" >&2
            done <<<"${rpaths}"
            FOUND=1
        fi
    done < <(find "${DEPLOYDIR}" -type f \( -name "*.dylib" -o -perm -u+x \) -print0)

    if [ "${FOUND}" -eq 1 ]; then
        echo "Homebrew linkage detected in ${DEPLOYDIR} — see above." >&2
        return 1
    fi
}

build_macos_dmg() {
    MACOS_BUILD_DIR=${WORKSPACE_DIR}/build-macos
    MACOS_OUTPUT_DIR=${WORKSPACE_DIR}/output-macos

    # Unlike SOURCES_DIR, these live outside the git worktree, so setup()'s
    # git clean -fdx never touches them. Wipe them here instead: a stale
    # CMakeCache.txt from a previous run (before a script change, or from a
    # failed configure) must never silently survive into this one.
    rm -rf "${MACOS_BUILD_DIR}" "${MACOS_OUTPUT_DIR}"

    if [ -z "${WORKRAVE_SIGN_IDENTITY}" ]; then
        WORKRAVE_SIGN_IDENTITY=$(security find-identity -v -p codesigning | sed -n 's/.*"\(Developer ID Application: [^"]*\)".*/\1/p' | head -n1)
    fi
    if [ -z "${WORKRAVE_SIGN_IDENTITY}" ] || [ -z "${WORKRAVE_NOTARIZE_PROFILE}" ]; then
        echo "WORKRAVE_SIGN_IDENTITY and WORKRAVE_NOTARIZE_PROFILE must be set to build a signed macOS dmg." 1>&2
        echo "See ui/app/toolkits/qt/dist/macos/CMakeLists.txt for the one-time setup." 1>&2
        exit 1
    fi

    # WORKRAVE_DEPENDENCIES_DIR (-M) provides universal (arm64+x86_64) builds of
    # Qt, sqlite3 and other libs. It's optional: without it, fall back to
    # whatever CMake finds on its own (e.g. Homebrew's Qt keg), which is
    # normally arm64-only, so the build is then single-arch.
    #
    # Passed as the dependencies root, not .../lib/cmake: CMake's config-mode
    # search already checks <root>/lib/cmake/<Name>/ (finds Qt6, etc.), but
    # sqlite3 ships no CMake config package here, only headers/libs, so it's
    # found via CMake's bundled module-mode FindSQLite3.cmake, which only
    # searches <root>/include and <root>/lib — narrowing the prefix to
    # .../lib/cmake would hide those from it.
    if [ -n "${WORKRAVE_DEPENDENCIES_DIR}" ]; then
        if [ ! -d "${WORKRAVE_DEPENDENCIES_DIR}" ]; then
            echo "WORKRAVE_DEPENDENCIES_DIR (${WORKRAVE_DEPENDENCIES_DIR}) does not exist." 1>&2
            exit 1
        fi

        MACOS_PATH="${WORKRAVE_DEPENDENCIES_DIR}/bin:${PATH}"
        MACOS_PREFIX_PATH="${WORKRAVE_DEPENDENCIES_DIR}"
        MACOS_ARCHITECTURES="arm64;x86_64"
    else
        MACOS_PATH="${PATH}"
        MACOS_PREFIX_PATH=$(brew --prefix qt 2>/dev/null || true)
        MACOS_ARCHITECTURES=""
    fi

    PATH="${MACOS_PATH}" \
        cmake -S ${SOURCES_DIR} -B ${MACOS_BUILD_DIR} -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${MACOS_OUTPUT_DIR} \
        -DCMAKE_PREFIX_PATH="${MACOS_PREFIX_PATH}" \
        -DCMAKE_OSX_ARCHITECTURES="${MACOS_ARCHITECTURES}" \
        -DWITH_UI=Qt \
        -DWITH_GRPC=ON \
        "-DWORKRAVE_SIGN_IDENTITY=${WORKRAVE_SIGN_IDENTITY}" \
        "-DWORKRAVE_NOTARIZE_PROFILE=${WORKRAVE_NOTARIZE_PROFILE}"

    cmake --build ${MACOS_BUILD_DIR} --target dmg

    # Checked here, not before the build: the dependencies tree itself is
    # already known clean (that's what workrave-dependencies' own CI verifies).
    # What matters is whether the build picked up something from Homebrew along
    # the way (e.g. a tool falling back to it despite -M) — the app bundle is
    # the actual output that ships, so it's the one to check.
    if [ -n "${WORKRAVE_DEPENDENCIES_DIR}" ]; then
        APP_BUNDLE=$(find ${MACOS_OUTPUT_DIR} -maxdepth 1 -name "*.app" | head -n1)
        if [ -z "${APP_BUNDLE}" ]; then
            echo "No .app bundle found in ${MACOS_OUTPUT_DIR}" 1>&2
            exit 1
        fi
        check_no_homebrew_links "${APP_BUNDLE}" || exit 1
    fi

    DMG_FILE=$(find ${MACOS_OUTPUT_DIR} -maxdepth 1 -name "*.dmg" | head -n1)
    if [ -z "${DMG_FILE}" ]; then
        echo "No dmg found in ${MACOS_OUTPUT_DIR}" 1>&2
        exit 1
    fi

    mkdir -p ${DEPLOY_DIR}/${GIT_TAG}
    DEPLOY_DMG=${DEPLOY_DIR}/${GIT_TAG}/workrave-macos-${WORKRAVE_VERSION}.dmg
    cp "${DMG_FILE}" "${DEPLOY_DMG}"
    run_or_echo ${SCRIPTS_DIR}/local/sign-cosign.sh "${DEPLOY_DMG}"
}

init_newsgen() {
    source ${SCRIPTS_DIR}/ci/ship.sh
    build_ship
}

init() {
    if [ ! -d "${WORKSPACE_DIR}" ]; then
        mkdir -p "${WORKSPACE_DIR}"
    fi
    if [ ! -d "$DEPLOY_DIR" ]; then
        mkdir -p "$DEPLOY_DIR"
    fi

    init_workspace
    init_version
    init_newsgen
}

generate_blog() {
    cd ${SOURCES_DIR}
    DIR_DATE=$(date +"%Y_%m_%d")
    if [ -n "$WORKRAVE_OVERRIDE_GIT_VERSION" ]; then
        DIR_VERSION=$(echo $WORKRAVE_OVERRIDE_GIT_VERSION | sed -e 's/^v//g')
    else
        DIR_VERSION=$(git describe --tags --abbrev=10 2>/dev/null | sed -e 's/-g.*//' | sed -e 's/^v//g')
    fi
    DIR="${WEBSITE_DIR}/content/en/blog/${DIR_DATE}_workrave-${DIR_VERSION}-released"
    WORKRAVE_VERSION=$(echo $DIR_VERSION | sed -e 's/_\([0-9]\)/.\1/g' | sed -E -e 's/-[0-9]+//g' | sed -e 's/_/-/g')

    if [ ! -d $DIR ]; then
        mkdir -p ${DIR}
        cd /
        run_ship newsgen \
            --input "${SOURCES_DIR}/changes.yaml" \
            --template blog \
            --release $(echo ${WORKRAVE_VERSION} | sed -e 's/^v//g') \
            --single \
            --output "${DIR}/index.md"
    fi
}

setup() {
    cd $WORKSPACE_DIR/source
    git reset --hard HEAD
    git clean -fdx
    git checkout $COMMIT
}

PLATFORM_OPTIONS="bB:D:M:p:P"

parse_platform_argument() {
    case "${1}" in
    b)
        BUILD_DEB=1
        ;;
    B)
        WEBSITE_DIR="${2}"
        ;;
    D)
        DEBIAN_DIR="${2}"
        ;;
    M)
        WORKRAVE_DEPENDENCIES_DIR="${2}"
        ;;
    p)
        PPA="${2}"
        ;;
    P)
        PRERELEASE=1
        ;;
    *)
        usage
        ;;
    esac
}

upload() {
    upload_github
}

upload_github() {
    fetch_github_token
    github_create_release
    for artifact in ${DEPLOY_DIR}/${GIT_TAG}/*.AppImage ${DEPLOY_DIR}/${GIT_TAG}/*.dmg ${DEPLOY_DIR}/${GIT_TAG}/*.dmg.sigstore; do
        if [ -f "${artifact}" ]; then
            run_or_echo gh release upload ${GIT_TAG} "${artifact}"
        fi
    done
}

init_common_defaults
BUILD_DEB=
PRERELEASE=
DEBIAN_DIR=
WEBSITE_DIR=
WORKRAVE_DEPENDENCIES_DIR=

parse_arguments $*

if [ -z $WEBSITE_DIR ]; then
    echo No website directory specified.
    exit 1
fi

DEPLOY_DIR=$WORKSPACE_DIR/deploy

export CONF_CONFIGURATION=Release
export WORKRAVE_ENV=local
init

export DOCKER_IMAGE="ubuntu-resolute"
setup
run_docker_appimage
env
run_docker_ppa

if [ "$(uname)" = "Darwin" ]; then
    build_macos_dmg
fi

if [ -n "$BUILD_DEB" ]; then
    echo Build all debian packages.
    run_docker_deb
fi

# generate_blog
upload

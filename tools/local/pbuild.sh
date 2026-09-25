#!/bin/bash -ex

usage() {
    echo "Usage: $0 " 1>&2
    exit 1
}

parse_arguments() {
    while getopts "p:d" o; do
        case "${o}" in
        *)
            usage
            ;;
        esac
    done
    shift $((OPTIND - 1))
}

parse_arguments $*

# In a rootless container (podman without root) device nodes cannot be
# created, so the device nodes in the base tarballs cannot be extracted.
# Replace them by empty placeholder files and bind-mount the container's
# devices onto those instead. pbuilder handles /dev/pts and /dev/ptmx itself.
DEVICES="/dev/null /dev/zero /dev/full /dev/random /dev/urandom /dev/tty"
BINDMOUNTS=
if ! mknod /tmp/.mknod-test c 1 3 2>/dev/null; then
    echo "Cannot create device nodes (rootless container); bind-mounting devices into the chroots"
    BINDMOUNTS="$DEVICES"
fi
rm -f /tmp/.mknod-test

prepare_basetgz() {
    tgz=$1
    if [ -z "$BINDMOUNTS" ]; then
        return
    fi
    tmp=$(mktemp -d)
    gzip -dc "$tgz" > "$tmp/base.tar"
    nodes=$(tar -tvf "$tmp/base.tar" | awk '$1 ~ /^[cb]/ {print $NF}')
    if [ -n "$nodes" ]; then
        echo "Replacing the device nodes in $tgz by placeholders"
        for node in $nodes; do
            tar --delete -f "$tmp/base.tar" "$node"
            mkdir -p "$tmp/placeholders/$(dirname "$node")"
            touch "$tmp/placeholders/$node"
        done
        tar -rf "$tmp/base.tar" -C "$tmp/placeholders" $nodes
        gzip -1 "$tmp/base.tar"
        mv "$tmp/base.tar.gz" "$tgz"
    fi
    rm -rf "$tmp"
}

# One directory per Ubuntu series, recognised by the source package in it
# (deploy/ also holds the release directory, newsgen/ and the like).
DIST_DIRS=$(find /workspace/deploy -mindepth 2 -maxdepth 2 -name 'workrave_*.dsc' -print | xargs -n1 dirname | sort -ru)
for dir in $DIST_DIRS; do
    dist=$(basename $dir)
    cd $dir
    prepare_basetgz /var/cache/pbuilder/base-$dist.tgz
    # pbuilder ignores every option that follows the .dsc, and --bindmounts
    # takes all its directories as one argument.
    opts=(--basetgz /var/cache/pbuilder/base-$dist.tgz)
    if [ -n "$BINDMOUNTS" ]; then
        opts+=(--bindmounts "$BINDMOUNTS")
    fi
    echo Updating $dist builder
    DIST=$dist pbuilder --update "${opts[@]}"
    echo Running build for $dist
    DIST=$dist pbuilder --build "${opts[@]}" workrave*.dsc
    mkdir -p /workspace/deploy/$GIT_TAG/$dist
    # The results are owned by pbuilder's build user; keep them owned by the
    # container user, which maps to the host user when running rootless.
    cp --no-preserve=ownership /var/cache/pbuilder/result/*$dist*.deb /var/cache/pbuilder/result/*$dist*.ddeb /workspace/deploy/$GIT_TAG/$dist
done

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

# Ignore old deploy directories for series that are no longer selected.
for dist in ${WORKRAVE_PPA_SERIES:-stonking resolute noble}; do
    dir=/workspace/deploy/$dist
    if ! compgen -G "$dir/workrave_*.dsc" >/dev/null; then
        continue
    fi
    cd "$dir"
    echo Updating $dist builder
    DIST=$dist cowbuilder --update --basepath /var/cache/pbuilder/base-$dist.cow
    echo Running build for $dist
    DIST=$dist cowbuilder --build workrave*.dsc --basepath /var/cache/pbuilder/base-$dist.cow
done

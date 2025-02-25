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

DIST_DIRS=$(find /workspace/deploy -mindepth 1 -type d -print | grep -v "v1_11" | sort -r)
for dir in $DIST_DIRS; do
    dist=$(basename $dir)
    cd $dir
    echo Updating $dist builder
    DIST=$dist cowbuilder --update --basepath /var/cache/pbuilder/base-$dist.cow
    echo Running build for $dist
    DIST=$dist cowbuilder --build workrave*.dsc --basepath /var/cache/pbuilder/base-$dist.cow
done

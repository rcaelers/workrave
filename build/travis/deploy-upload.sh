#!/bin/bash

shopt -s nullglob

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

cd ${DEPLOY_DIR}
for file in *.exe *.tar.gz; do
    github-release upload \
                   --user "${TRAVIS_REPO_SLUG%%/[^/]*}" \
                   --repo "${TRAVIS_REPO_SLUG#[^/]*/}" \
                   --tag "$TRAVIS_TAG" \
                   --name $file \
                   --file $file
done

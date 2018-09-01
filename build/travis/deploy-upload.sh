#!/bin/bash
for file in _deploy/*.exe; do
    github-release upload \
                   --user "${TRAVIS_REPO_SLUG%%/[^/]*}" \
                   --repo "${TRAVIS_REPO_SLUG#[^/]*/}" \
                   --tag "$TRAVIS_TAG" \
                   --file $file 
done

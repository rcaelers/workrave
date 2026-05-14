#!/bin/bash -e

SCRIPTS_DIR=${SCRIPTS_DIR:-$(dirname "$0")/..}

catalogs=$(find $ARTIFACTS -name "job-catalog*")
for catalog in $catalogs; do
  files=$(jq -r '.builds[].artifacts[] | .filename' $catalog | tr -d '\r')
  folder=$(dirname $catalog)
  for file in $files; do
    signature=$(${SCRIPTS_DIR}/local/sign-ed25519.sh $folder/$file)

    jq \
      --arg querykey $file \
      --arg sig "$signature" \
      '( .builds[].artifacts[] | select(.filename == $querykey)).ed25519 |= $sig' $catalog >$catalog.new
    mv -f $catalog.new $catalog
  done
done

#!/bin/bash -e

if [ $(uname) = "Darwin" ]; then
  OPENSSL=${OPENSSL:-/usr/local/opt/openssl@3/bin/openssl}
else
  OPENSSL=${OPENSSL:-openssl}
fi

catalogs=$(find $ARTIFACTS -name "job-catalog*")
for catalog in $catalogs; do
  files=$(jq -r '.builds[].artifacts[] | .filename' $catalog)
  for file in $files; do
    signature=$($OPENSSL pkeyutl -sign -rawin -in $ARTIFACTS/$file -inkey $WORKSPACE/ed25519key.pem | $OPENSSL base64 | tr -d \\n)

    jq \
      --arg querykey $file \
      --arg sig "$signature" \
      '( .builds[].artifacts[] | select(.filename == $querykey)).ed25519 |= $sig' $catalog >$catalog.new
    mv -f $catalog.new $catalog
  done
done

#!/bin/bash -x

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

mkdir -p ${DEPLOY_DIR}

CATALOG_NAME=${DEPLOY_DIR}/job-catalog-root-${WORKRAVE_JOB_NUMBER}.json
export NOTES=""

if [[ -n "$WORKRAVE_RELEASE" ]]; then
  cd ${SCRIPTS_DIR}/newsgen
  npm install
  cd ${SOURCES_DIR}
  node --experimental-modules ${SCRIPTS_DIR}/newsgen/main.js \
    --input "${SOURCE_DIR}/changes.yaml" \
    --template github \
    --single \
    --release $(echo $WORKRAVE_VERSION | sed -e 's/^v//g') \
    --output "release-notes.md"

  NOTES=$(cat $NOTES_FILE)
fi

if [ ! -f $CATALOG_NAME ]; then
  jq -n ' {
              "version": "2",
              "builds": [
                {
                  "id": env.WORKRAVE_BUILD_ID,
                  "tag": env.WORKRAVE_GIT_TAG,
                  "increment": env.WORKRAVE_COMMIT_COUNT,
                  "hash": env.WORKRAVE_COMMIT_HASH,
                  "date": env.WORKRAVE_BUILD_DATETIME,
                  "notes": env.NOTES,
                  "artifacts": []
                }
              ]
            }
' >$CATALOG_NAME
fi

ls -la ${DEPLOY_DIR}
chmod 644 ${DEPLOY_DIR}/*

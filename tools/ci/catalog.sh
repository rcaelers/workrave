#!/bin/bash -ex

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

mkdir -p ${DEPLOY_DIR}

CATALOG_DIR=${DEPLOY_DIR}
CATALOG_NAME=${CATALOG_DIR}/job-catalog-root-${WORKRAVE_JOB_NUMBER}.json

if [[ -n "$WORKRAVE_RELEASE" ]]; then
  GEN_ARGS=-"-single --release $(echo $WORKRAVE_VERSION | sed -e 's/^v//g')"
else
  GEN_ARGS="--single --latest"
fi

cd ${SCRIPTS_DIR}/citool
npm install
npm run build
cd ${SOURCES_DIR}
${SCRIPTS_DIR}/citool/bin/citool.ts newsgen \
  --input "${SOURCES_DIR}/changes.yaml" \
  --template github \
  $GEN_ARGS \
  --output "release-notes.md"

export NOTES=$(cat release-notes.md)

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

ls -la ${CATALOG_DIR}
chmod 644 ${CATALOG_DIR}/*

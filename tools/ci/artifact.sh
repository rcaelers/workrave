#!/bin/bash -e

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

parse_arguments() {
  while getopts "c:f:p:k:" o; do
    case "${o}" in
    c)
      export CONFIG=${OPTARG}
      ;;
    f)
      export FILENAME=${OPTARG}
      ;;
    p)
      export PLATFORM=${OPTARG}
      ;;
    k)
      export KIND=${OPTARG}
      ;;
    esac
  done
  shift $((OPTIND - 1))
}

parse_arguments $*

mkdir -p ${DEPLOY_DIR}

CATALOG_NAME=${DEPLOY_DIR}/job-catalog-${WORKRAVE_JOB_NUMBER}.json

if [ ! -f $CATALOG_NAME ]; then
  jq -n ' {
              "version": "2",
              "builds": [
                {
                  "id": env.WORKRAVE_BUILD_ID,
                  "artifacts": []
                }
              ]
            }
' >$CATALOG_NAME
fi

export SIZE=$(stat --printf="%s" ${DEPLOY_DIR}/$FILENAME)
export SHA256=$(sha256sum ${DEPLOY_DIR}/$FILENAME | cut -d' ' -f1)
export SHA512=$(sha512sum ${DEPLOY_DIR}/$FILENAME | cut -d' ' -f1)
export LASTMOD=$(date -r ${DEPLOY_DIR}/$FILENAME +"%Y-%m-%d %H:%M:%S")
export URL="$WORKRAVE_UPLOAD_DIR/$FILENAME"

tmp=$(mktemp)
cat $CATALOG_NAME | jq '.builds[-1].artifacts +=
    [
        {
            "url": env.URL,
            "size": env.SIZE,
            "sha256": env.SHA256,
            "sha512": env.SHA512,
            "path": env.WORKRAVE_UPLOAD_DIR,
            "lastmod": env.LASTMOD,
            "filename": env.FILENAME,
            "platform": env.PLATFORM,
            "kind": env.KIND,
            "configuration": env.CONFIG
        }
    ]
' >$tmp

mv -f $tmp $CATALOG_NAME

ls -la ${DEPLOY_DIR}
chmod 644 ${DEPLOY_DIR}/*

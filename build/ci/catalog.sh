#!/bin/bash -x

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

parse_arguments()
{
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
  shift $((OPTIND-1))
}

parse_arguments $*

# --arg body "`sed -n "/## \[$TRAVIS_TAG\]/,/## \[/{/## \[/b;p}" CHANGELOG.md`"

mkdir -p ${DEPLOY_DIR}

CATALOG_NAME=${DEPLOY_DIR}/job-catalog-${WORKRAVE_JOB_NUMBER}.json

if [ ! -f $CATALOG_NAME ]; then
    jq -n ' {
              "version": "2",
              "builds": [
                {
                  "id": env.WORKRAVE_BUILD_ID,
                  "tag": env.WORKRAVE_TAG,
                  "increment": env.WORKRAVE_COMMIT_COUNT,
                  "hash": env.WORKRAVE_COMMIT_HASH,
                  "date": env.WORKRAVE_BUILD_DATETIME,
                  "artifacts": []
                }
              ]
            }
' > $CATALOG_NAME
fi

export SIZE=`stat --printf="%s" ${DEPLOY_DIR}/$FILENAME`
export LASTMOD=`date -r ${DEPLOY_DIR}/$FILENAME +"%Y-%m-%d %H:%M:%S"`
export URL="$WORKRAVE_UPLOAD_DIR/$FILENAME"

tmp=`mktemp`
cat $CATALOG_NAME| jq '.builds[-1].artifacts +=
    [
        {
            "url": env.URL,
            "size": env.SIZE,
            "path": env.WORKRAVE_UPLOAD_DIR,
            "lastmod": env.LASTMOD,
            "filename": env.FILENAME,
            "platform": env.PLATFORM,
            "kind": env.KIND,
            "configuration": env.CONFIG
        }
    ]
' > $tmp

mv -f $tmp $CATALOG_NAME
ls -la ${DEPLOY_DIR}
chmod 644 ${DEPLOY_DIR}/*

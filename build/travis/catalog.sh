#!/bin/bash -x

BASEDIR=$(dirname "$0")
source ${BASEDIR}/config.sh

parse_arguments()
{
  while getopts "c:f:p:k:D" o; do
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
          D)
            export TRAVIS_BRANCH=next
            export TRAVIS_BUILD_NUMBER=27
            export WORKRAVE_FULL_TAG=`git describe --tags --abbrev=10`
            export WORKRAVE_TAG=`git describe --abbrev=0`
            export WORKRAVE_COMMIT_COUNT=`git rev-list ${WORKRAVE_TAG}..HEAD --count`
            export WORKRAVE_COMMIT_HASH=`git rev-parse HEAD`
            export WORKRAVE_BUILD_DATE=`date +"%Y%m%d"`
            export WORKRAVE_BUILD_ID="$WORKRAVE_BUILD_DATE-$WORKRAVE_FULL_TAG"
            export WORKRAVE_UPLOAD_DIR="snapshots/next/$WORKRAVE_BUILD_ID"
            ;;
      esac
  done
  shift $((OPTIND-1))
}

parse_arguments $*

# --arg body "`sed -n "/## \[$TRAVIS_TAG\]/,/## \[/{/## \[/b;p}" CHANGELOG.md`"

CATALOG_NAME=${DEPLOY_DIR}/job-catalog-${TRAVIS_BUILD_NUMBER}.json

if [ ! -f $CATALOG_NAME ]; then
    jq -n '
    {
        "builds": {
            (env.WORKRAVE_BUILD_ID): {
                "tag": env.WORKRAVE_TAG,
                "increment": env.WORKRAVE_COMMIT_COUNT,
                "hash": env.WORKRAVE_COMMIT_HASH,
                "date": env.WORKRAVE_BUILD_DATETIME,
                "artifacts": []
            }
        }
    }
    ' > $CATALOG_NAME
fi

export SIZE=`stat --printf="%s" $FILENAME`
export LASTMOD=`date -r $FILENAME +"%Y-%m-%d %H:%M:%S"`
export URL="$WORKRAVE_UPLOAD_DIR/$FILENAME"

tmp=`mktemp`
cat $CATALOG_NAME| jq '.builds[env.WORKRAVE_BUILD_ID].artifacts +=
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

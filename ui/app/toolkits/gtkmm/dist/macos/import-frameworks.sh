#!/bin/sh
TARGETDIR=`pwd`/Workrave.app
EXECS=workrave

for e in $EXECS; do

    F=`otool -L $TARGETDIR/Contents/MacOS/$e | grep compatibility | grep -v /System/ | grep -v /usr/  | awk '{print $1}'`
    FRAMEWORKS="$FRAMEWORKS $F"
done

for f in $FRAMEWORKS ; do

    DIR=`echo $f | sed -e 's/\([^.]*\)\/\([^.]*\)\.framework.*/\1/'`
    FW=`echo $f | sed -e 's/\([^.]*\)\/\([^.]*\)\.framework.*/\2/'`.framework
    VER=`echo $f | sed -e 's/.*\/Versions\/\([^.]*\)/\1/'`

    echo "Installing $FW..."

    mkdir -p $TARGETDIR/Contents/Frameworks
    rm -rf $TARGETDIR/Contents/Frameworks/$FW
    cp -Rp $DIR/$FW $TARGETDIR/Contents/Frameworks
    rm -rf $TARGETDIR/Contents/Frameworks/$FW/Resources/dev/

    HEADER=`readlink $TARGETDIR/Contents/Frameworks/$FW/Headers`
    if [ -d $TARGETDIR/Contents/Frameworks/$FW/$HEADER ]; then
        rm -rf $TARGETDIR/Contents/Frameworks/$FW/$HEADER
    fi
    rm $TARGETDIR/Contents/Frameworks/$FW/Headers
   
    ./relocate-framework.sh $TARGETDIR/Contents/Frameworks/$FW $DIR/$FW @executable_path/../Frameworks/$FW

    CHANGES="$CHANGES -change $f @executable_path/../Frameworks/$FW/Versions/$VER"
done

if [ "x$CHANGES" != "x" ];
then
    for e in $EXECS; do
	if install_name_tool $CHANGES $TARGETDIR/Contents/MacOS/$e ;
        then
	    :
	else
	    exit 1
	fi
    done
fi

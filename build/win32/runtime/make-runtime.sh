#!/bin/bash

RUNTIMEDIR="./"
ALL_LINGUAS=

if [ "x$1" != "x" ]; then
    if [ -e $1 ]; then
        ALL_LINGUAS=$(cat $1/po/LINGUAS |grep -v '^#')
        echo $ALL_LINGUAS
    fi
fi


mkdir -p $RUNTIMEDIR/runtime-base
mkdir -p $RUNTIMEDIR/runtime-gtk
mkdir -p $RUNTIMEDIR/runtime-wimp

## Helper

function copy_dir()
{
    sourcedir=$1;
    source=$2
    dest=$3;

    prefix=`dirname $source`
    mkdir -p $TARGETDIR/$dest/$prefix
    cp -a $SYSROOT/$sourcedir/$source $TARGETDIR/$dest/$prefix
}

## Base runtime

TARGETDIR=$RUNTIMEDIR/runtime-base 

find $TARGETDIR -name "*.dll" -print | xargs $STRIP

for lang in $ALL_LINGUAS; do
    mkdir -p $TARGETDIR/lib/locale/$lang/LC_MESSAGES/
    cp -a /usr/share/locale/$lang/LC_MESSAGES/iso_639.mo $TARGETDIR/lib/locale/$lang/LC_MESSAGES/
    cp -a /usr/share/locale/$lang/LC_MESSAGES/iso_3166.mo $TARGETDIR/lib/locale/$lang/LC_MESSAGES/ 
done

## Dbus runtime

TARGETDIR=$RUNTIMEDIR/runtime-dbus

copy_dir  bin    dbus*.exe					bin
copy_dir  lib    libdbus-1.dll					lib
copy_dir  etc    dbus-1					        etc

## Gtk runtime

TARGETDIR=$RUNTIMEDIR/runtime-gtk

LIBS="libffi-6.dll
      libgcc_s_sjlj-1.dll
      libgio-2.0-0.dll
      libglib-2.0-0.dll
      libgmodule-2.0-0.dll
      libgmp-10.dll
      libgnutls-28.dll
      libgobject-2.0-0.dll
      libgthread-2.0-0.dll
      libhogweed-2-4.dll
      libintl-8.dll
      libnettle-4-6.dll
      libtasn1-6.dll
      zlib1.dll
"

for lib in $LIBS; do
    copy_dir  bin    $lib			   		   lib
done    

copy_dir  lib    gio/modules/*.dll                                 lib

for lang in $ALL_LINGUAS; do
    copy_dir lib locale/$lang lib  
done
for lang in $ALL_LINGUAS; do
    copy_dir share locale/$lang lib  
done

#find $RUNTIMEDIR -name "*.dll" -not -name "iconv.dll" -not -name "intl.dll" -not -name "zlib1.dll" -print | xargs $STRIP

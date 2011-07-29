#!/bin/bash

RUNTIMEDIR="./"
ALL_LINGUAS="nl de eo pl da es zh_TW ru fr pt_BR"

if [ "x$1" != "x" ]; then
    if [ -e $1 ]; then
        ALL_LINGUAS=`grep ^ALL_LING $1/configure.ac | grep -v "for a in" | sed -e 's/^ALL_LINGUAS="\([a-zA-Z_ ]*\)"/\1/g'`
        echo $ALL_LINGUAS
    fi
fi


. `dirname $0`/mingw32

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
    cp -a $CROSSROOT/$sourcedir/$source $TARGETDIR/$dest/$prefix
}

## Base runtime

TARGETDIR=$RUNTIMEDIR/runtime-base 

copy_dir bin     gnet-2.0.dll                                   lib

find $TARGETDIR -name "*.dll" -print | xargs $STRIP

for lang in $ALL_LINGUAS; do
    mkdir -p $TARGETDIR/lib/locale/$lang/LC_MESSAGES/
    cp -a /usr/share/locale/$lang/LC_MESSAGES/iso_639.mo $TARGETDIR/lib/locale/$lang/LC_MESSAGES/
    cp -a /usr/share/locale/$lang/LC_MESSAGES/iso_3166.mo $TARGETDIR/lib/locale/$lang/LC_MESSAGES/ 
done

## Dbus runtime

TARGETDIR=$RUNTIMEDIR/runtime-dbus

copy_dir  bin    dbus*.exe					bin
copy_dir  bin    dbus*.bat					bin
copy_dir  lib    libdbus-1.dll					lib
copy_dir  etc    dbus-1					        etc

## Gtk runtime

if [ -d $CROSSROOT/lib/gtk-2.0/2.4.0 ]; then
    GTKVER=2.4.0
elif [ -d $CROSSROOT/lib/gtk-2.0/2.10.0 ]; then
    GTKVER=2.10.0
fi

if [ -d $CROSSROOT/lib/pango/1.4.0 ]; then
    PANVER=1.4.0
elif [ -d $CROSSROOT/lib/pango/1.5.0 ]; then
    PANVER=1.5.0
fi

TARGETDIR=$RUNTIMEDIR/runtime-gtk

copy_dir  etc    gtk-2.0                                        etc
copy_dir  etc    pango						etc
copy_dir  bin    zlib1.dll					lib
copy_dir  bin    iconv.dll					lib
copy_dir  bin    intl.dll                           		lib
copy_dir  bin    libexpat-1.dll                       		lib
copy_dir  bin    freetype6.dll                       		lib
copy_dir  bin    libpng*.dll                         		lib
copy_dir  bin    jpeg62.dll                         		lib
copy_dir  bin    libjpeg-7.dll                         		lib
copy_dir  bin    libtiff3.dll                         		lib
copy_dir  bin    libtiff-3.dll                         		lib
copy_dir  bin    libfontconfig-1.dll                   		lib
copy_dir  bin    libatk-1.0-0.dll                   		lib
copy_dir  bin    libgdk-win32-2.0-0.dll             		lib
copy_dir  bin    libgdk_pixbuf-2.0-0.dll            		lib
copy_dir  bin    libglib-2.0-0.dll                  		lib
copy_dir  bin    libgio-2.0-0.dll                  		lib
copy_dir  bin    libgmodule-2.0-0.dll               		lib
copy_dir  bin    libgobject-2.0-0.dll               		lib
copy_dir  bin    libgthread-2.0-0.dll               		lib
copy_dir  bin    libgtk-win32-2.0-0.dll             		lib
copy_dir  bin    libpango-1.0-0.dll                 		lib
copy_dir  bin    libpangoft2-1.0-0.dll              		lib
copy_dir  bin    libpangowin32-1.0-0.dll            		lib
copy_dir  bin    libpangocairo-1.0-0.dll                        lib
copy_dir  bin    libcairo-2.dll                                 lib

copy_dir  lib    gtk-2.0/$GTKVER/immodules/*.dll                   lib
copy_dir  lib    gtk-2.0/$GTKVER/loaders/libpixbufloader-ico.dll   lib 
copy_dir  lib    gtk-2.0/$GTKVER/loaders/libpixbufloader-png.dll   lib
copy_dir  lib    gtk-2.0/$GTKVER/loaders/libpixbufloader-pnm.dll   lib

copy_dir  lib    pango/$PANVER/modules      			lib

for lang in $ALL_LINGUAS; do
    copy_dir lib locale/$lang lib  
done
for lang in $ALL_LINGUAS; do
    copy_dir share locale/$lang lib  
done

TARGETDIR=$RUNTIMEDIR/runtime-wimp

if [ -f $CROSSROOT/lib/gtk-2.0/$GTKVER/engines/libwimp.dll ]; then
    copy_dir  lib    gtk-2.0/$GTKVER/engines/libwimp.dll        lib
    copy_dir  share  themes/*   share
fi
cp -a $TARGETDIR/share/themes/MS-Windows/gtk-2.0/gtkrc $TARGETDIR/share/themes/Raleigh/gtk-2.0/

find $RUNTIMEDIR -name "*.dll" -not -name "iconv.dll" -not -name "intl.dll" -not -name "zlib1.dll" -print | xargs $STRIP

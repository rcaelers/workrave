#!/bin/bash
LOCALFILES=lib/gnet-1.1.dll

TARGETFILES="
bin/libpng.dll:lib/x
bin/zlib.dll:lib/x
etc/ 
lib/gtk-2.0/2.2.0/immodules/
lib/gtk-2.0/2.2.0/loaders/libpixbufloader-ico.dll
lib/gtk-2.0/2.2.0/loaders/libpixbufloader-png.dll
lib/gtk-2.0/2.2.0/loaders/libpixbufloader-pnm.dll
lib/iconv.dll
lib/libatk-1.0-0.dll
lib/libgdk-win32-2.0-0.dll
lib/libgdk_pixbuf-2.0-0.dll
lib/libglib-2.0-0.dll
lib/libgmodule-2.0-0.dll
lib/libgobject-2.0-0.dll
lib/libgthread-2.0-0.dll
lib/libgtk-win32-2.0-0.dll
lib/libintl-1.dll
lib/libpango-1.0-0.dll
lib/libpangoft2-1.0-0.dll
lib/libpangowin32-1.0-0.dll
lib/localcharset.dll
lib/locale/nl/
lib/locale/de/
lib/pango/
share/themes/"


rm -rf runtime
mkdir runtime

function copy()
{
    sourcedir=$1
    shift
    
    for a in $*; do

        s=`echo $a | sed 's/^\([^:]*\):.*/\1/'`
        d=`echo $a | sed 's/^[^:]*:\(.*\)/\1/'`
       
        if [ "$dx" != "x" ] ; then
            target=runtime/$d;
        else
            target=runtime/$s;
        fi


        echo processing $s
        source=$sourcedir/$s
        target=`dirname $target`
        
        [ -d $target ] || mkdir -p $target
        cp -a $source $target
    done
}

copy "target" $TARGETFILES
copy "local" $LOCALFILES

find runtime -name "*.dll" -print | xargs strip
cp harpoon.dll runtime/lib/

tar cvfz runtime.tar.gz runtime 
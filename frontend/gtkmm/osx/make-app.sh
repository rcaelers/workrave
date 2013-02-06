#!/bin/bash
#
# Copyright (C) 2008, 2009, 2013 Rob Caelers <robc@krandor.nl>
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Based on code from inkscape, growl, adium, mozilla and probably others.
#

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

LIBPREFIX=$HOME/gtk/inst
BINARY=workrave
PACKAGE=Workrave.app
PACKAGE_VERSION=`grep PACKAGE_VERSION ../../../config.h | cut -d' ' -f 3 | sed "s/\\"//g"`

export PKG_CONFIG_PATH=/Users/robc/gtk/inst/lib/pkgconfig:/Users/robc/gtk/inst/share/pkgconfig:/usr/lib/pkgconfig

# ------------------------------------------------------------
# Print usage message
# ------------------------------------------------------------
usage()
{
echo -e "
Create Workrave application bundle for OS X

Usage: $0 [OPTION...]

  --help, -h                 Display this help message
  --keep-symbols             Keep debugging symbols
  --link                     Create symlinks
  --gtk-dylib                Copy Gtk dynalic libraries
"
}

# ------------------------------------------------------------
# Parse command line arguments
# ------------------------------------------------------------

conf_strip=true
conf_symlink=false
conf_copy_gtk_dylib=false

while [ "$1" != "" ]
do
    case $1 in
	--gtk-dylib)
	    conf_copy_gtk_dylib=true ;;

	--keep-symbols)
	    conf_strip=false ;;

	--link)
	    conf_symlink=true ;;

      	-h|--help)
	    usage
	    exit 0 ;;
	*)
	    usage
	    exit 2 ;;
	esac
	shift 1
done


# ------------------------------------------------------------
# Setup
# ------------------------------------------------------------

osx_minor_version=`/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d'.'`

pkgrootdir=`pwd`/$PACKAGE
pkgcontentsdir=$pkgrootdir/Contents
pkgexecdir=$pkgcontentsdir/MacOS
pkgresourcesdir=$pkgcontentsdir/Resources
pkgdatadir=$pkgresourcesdir/share
pkgetcdir=$pkgresourcesdir/etc
pkglibdir=$pkgresourcesdir/lib
pkgthemedir=$pkgresourcesdir/themes

sed -e "s?@ENV@?$env?g" -e "s?@VERSION@?$PACKAGE_VERSION?g" < Info.plist.in > Info.plist

# ------------------------------------------------------------
# Create directory layout
# ------------------------------------------------------------

echo
echo -e "Creating Workrave application bundle"
echo -e "Workrave version is $PACKAGE_VERSION"

if [ -d $PACKAGE ];
then
    echo "Removing old $PACKAGE tree"
    rm -rf $PACKAGE
fi

echo "Building new app directory structure"

mkdir -p $pkgresourcesdir
mkdir -p $pkgexecdir
mkdir -p $pkglibdir

# ------------------------------------------------------------
# Install Workrave
# ------------------------------------------------------------

echo "Installing Workrave"

if [ $conf_symlink = false ]; then
   INSTALL="cp"
else
   INSTALL="./install_symlink.sh"
fi

make install -C ../../../ \
    prefix=$pkgrootdir bindir=$pkgexecdir pkgdatadir=$pkgresourcesdir \
    datadir=$pkgresourcesdir soundsbasedir=$pkgresourcesdir/sounds \
    DATADIRNAME=Contents/Resources > install.log 2>&1

if [ $conf_symlink = true ]; then
    rm -rf $pkgexecdir/workrave
    ln -s `pwd`/../../../frontend/gtkmm/src/workrave $pkgexecdir
fi
    
rm -rf $pkgrootdir/lib
rm -rf $pkgrootdir/libexec

if [ $conf_copy_gtk_dylib = true ]; then
    mv $pkgexecdir/$BINARY $pkgexecdir/${BINARY}-bin
    $INSTALL workrave $pkgexecdir
fi

cp Info.plist $pkgcontentsdir
$INSTALL ../../../frontend/common/share/images/osx/workrave.icns  $pkgresourcesdir

echo -n "APPL????" > $pkgcontentsdir/PkgInfo

# ------------------------------------------------------------
# Install Pango
# ------------------------------------------------------------

if [ $conf_copy_gtk_dylib = true ]; then

    echo "Installing Pango modules"
    
    pango_version=`pkg-config --variable=pango_module_version pango`
    
    mkdir -p $pkglibdir/pango/$pango_version/modules
    cp $LIBPREFIX/lib/pango/$pango_version/modules/*.so $pkglibdir/pango/$pango_version/modules/
    
    mkdir -p $pkgetcdir/pango
    
    $LIBPREFIX/bin/pango-querymodules \
        | sed "s?$LIBPREFIX/lib/pango/$pango_version/modules/?@executable_path/../Resources/lib/pango/$pango_version/modules/?" \
        > $pkglibdir/pango/$pango_version/modules.cache
    
    #mkdir -p $pkgetcdir/fonts
    #cp $LIBPREFIX/etc/fonts/fonts.dtd $pkgetcdir/fonts/
    #cp -r $LIBPREFIX/etc/fonts/conf.avail $pkgetcdir/fonts/
    #cp -r $LIBPREFIX/etc/fonts/conf.d $pkgetcdir/fonts/
    #cp fonts.conf $pkgetcdir/fonts
    
#    cat > $pkgetcdir/pango/pangorc << END
#[Pango]
#ModuleFiles=./pango.modules
#END

fi

# ------------------------------------------------------------
# Install Gtk
# ------------------------------------------------------------

if [ $conf_copy_gtk_dylib = true ]; then

    gtk_version=`pkg-config --variable=gtk_binary_version gtk+-3.0`
    gdk_pixbuf_version=`pkg-config --variable=gdk_pixbuf_binary_version gdk-pixbuf-2.0`
    
    mkdir -p $pkglibdir/gtk-3.0/$gtk_version/{immodules,printbackends}
    mkdir -p $pkgetcdir/gtk-3.0
    mkdir -p $pkglibdir/gio/modules
    mkdir -p $pkglibdir/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders
    
    cp $LIBPREFIX/lib/gtk-3.0/$gtk_version/immodules/*.so $pkglibdir/gtk-3.0/$gtk_version/immodules/
    cp $LIBPREFIX/lib/gtk-3.0/$gtk_version/printbackends/*.so $pkglibdir/gtk-3.0/$gtk_version/printbackends/
    cp $LIBPREFIX/lib/gio/modules/*.so $pkglibdir/gio/modules
    cp $LIBPREFIX/lib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/*.so $pkglibdir/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/
    
    sed "s?$LIBPREFIX/lib/gtk-3.0/$gtk_version/immodules/?@executable_path/../Resources/lib/gtk-3.0/$gtk_version/immodules/?" \
        < $LIBPREFIX/lib/gtk-3.0/$gtk_version/immodules.cache \
        > $pkglibdir/gtk-3.0/$gtk_version/immodules.cache

    $LIBPREFIX/bin/gdk-pixbuf-query-loaders \
        | sed "s?$LIBPREFIX/lib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/?@executable_path/../Resources/lib/gdk-pixbuf-2.0/$gdk_pixbuf_version/loaders/?" \
        > $pkglibdir/gdk-pixbuf-2.0/$gdk_pixbuf_binary_version/loaders.cache
 
    mkdir -p $pkgthemedir
    cp -r $LIBPREFIX/share/themes/* $pkgthemedir
    mkdir -p $pkgdatadir/glib-2.0/schemas
    cp -r $LIBPREFIX/share/glib-2.0/schemas $pkgdatadir/glib-2.0/
fi

# ------------------------------------------------------------
# Install dependencies
# ------------------------------------------------------------

if [ $conf_copy_gtk_dylib = true ]; then

    echo "Installing dependencies"

    total=0
    stop=false
    while ! $stop; do
        LIBS=`find $pkgrootdir \( -name "*.so*" -or -name "*.dylib" \) -print`
        EXECS=`find $pkgexecdir -type f -print`
        libs="`otool -L $LIBS $EXECS 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $LIBPREFIX | sort | uniq`"
        if [ "x$libs" != "x" ] ; then
            cp -f $libs $pkglibdir
        fi
        num_files=`ls $pkglibdir | wc -l`
        if [ $num_files = $total ];
        then
    	    stop=true
        fi
        total=$num_files
	exit
    done
fi

# ------------------------------------------------------------
# Fix dependencies
# ------------------------------------------------------------

echo "Fixing up library names"

LIBS=`find $pkgrootdir \( -name "*.so*" -or -name "*.dylib" \) -print`
EXECS=`find $pkgexecdir -type f -print`

for f in $EXECS $LIBS; do
    if [ ! -L $f ];
    then
	changes=""
	for lib in `otool -L $f | egrep "($LIBPREFIX|/local/|libs/)" | awk '{print $1}'` ; do
	    base=`basename $lib`
	    changes="$changes -change $lib @executable_path/../Resources/lib/$base"
	done
	if [ "x$changes" != x ];
        then
	    if install_name_tool $changes $f ;
            then
		:
	    else
		exit 1
	    fi
	fi

	base=`basename $f`
        ext=`echo $base | sed -e 's/.*\.\([^.]*\)/\1/'`

        if [ "x$ext" = "xso" -o "x$ext" = "xdylib" ]; 
        then
	    install_name_tool -id @executable_path/../Resources/lib/$base $f
        fi
    fi
done

# ------------------------------------------------------------
# Stripping
# ------------------------------------------------------------

if [ "$conf_strip" = "true" ];
then
    echo "Stripping debugging symbols"
    if [ $conf_copy_gtk_dylib = true ]; then
        chmod +w $pkglibdir/*.dylib
        strip -x $pkglibdir/*.dylib
    fi
    strip -ur `find $pkgexecdir -type f -print`
fi

exit 0


#------------------------------------------------------------

packagemaker=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker
pkgName="Workrave-${version}.pkg"



echo "Prepare for PackageManager"

sudo rm -rf pkgroot
sudo mkdir -p pkgroot

basedir="/Applications/Workrave"
sudo mkdir -p "pkgroot/$basedir"
sudo cp -R Workrave.app "pkgroot/$basedir"

sudo find pkgroot -type d -print0 | xargs -0 sudo chmod 755
sudo find pkgroot -type f -print0 | xargs -0 sudo chmod 644
sudo find pkgroot -name '*.sh' -print0 | xargs -0 sudo chmod 755
sudo chown -R root:wheel pkgroot

sudo chmod 775 pkgroot/Applications
sudo chown root:admin pkgroot/Applications
sudo chmod 1775 pkgroot/Library
sudo chown root:admin pkgroot/Library

echo "Exec PackageMaker"

rm -rf $pkgName
sudo $packagemaker -build \
    -p $pkgName \
    -f pkgroot \
    -ds \
    -r pkginfo/Resources \
    -i pkginfo/Info.plist \
    -d pkginfo/Description.plist

echo "Done."


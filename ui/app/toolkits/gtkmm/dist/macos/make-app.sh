#!/bin/bash
#
# Copyright (C) 2008, 2009 Rob Caelers <robc@krandor.nl>
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
WORKRAVE_VERSION=`grep WORKRAVE_VERSION ../../../config.h | cut -d' ' -f 3 | sed "s/\\"//g"`

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
  --import-frameworks        Import frameworks in Workrave.app
  --gtk-dylib                Copy Gtk dynalic libraries
"
}

# ------------------------------------------------------------
# Parse command line arguments
# ------------------------------------------------------------

conf_strip=true
conf_symlink=false
conf_copy_gtk_dylib=false
conf_import_frameworks=false

while [ "$1" != "" ]
do
    case $1 in
	--gtk-dylib)
	    conf_copy_gtk_dylib=true ;;

	--keep-symbols)
	    conf_strip=false ;;

	--link)
	    conf_symlink=true ;;

        --import-frameworks)
	    conf_import_frameworks=true ;;

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

macos_minor_version=`/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d'.'`

pkgrootdir=`pwd`/$PACKAGE
pkgcontentsdir=$pkgrootdir/Contents
pkgexecdir=$pkgcontentsdir/MacOS
pkgresourcesdir=$pkgcontentsdir/Resources
pkgframeworksdir=$pkgcontentsdir/Frameworks
pkgdatadir=$pkgresourcesdir/share
pkgetcdir=$pkgresourcesdir/etc
pkglibdir=$pkgresourcesdir/lib
pkgthemedir=$pkgresourcesdir/themes

sed -e "s?@ENV@?$env?g" -e "s?@VERSION@?$WORKRAVE_VERSION?g" < Info.plist.in > Info.plist

# ------------------------------------------------------------
# Create directory layout
# ------------------------------------------------------------

echo
echo -e "Creating Workrave application bundle"
echo -e "Workrave version is $WORKRAVE_VERSION"

if [ -d $PACKAGE ];
then
    echo "Removing old $PACKAGE tree"
    rm -rf $PACKAGE
fi

echo "Building new app directory structure"

mkdir -p $pkgresourcesdir
mkdir -p $pkgframeworksdir
mkdir -p $pkgexecdir
mkdir -p $pkglibdir

# ------------------------------------------------------------
# Install Workrave
# ------------------------------------------------------------

echo "Installing Workrave"

if [ $conf_symlink = false ]; then
   INSTALL="cp"
else
   INSTALL="ln -s"
fi

make install -C ../../../ \
    prefix=$pkgrootdir bindir=$pkgexecdir pkgdatadir=$pkgresourcesdir \
    datadir=$pkgresourcesdir soundsbasedir=$pkgresourcesdir/sounds \
    DATADIRNAME=Contents/Resources > install.log 2>&1

if [ $conf_symlink = true ]; then
    rm -rf $pkgexecdir/workrave
    ln -s ../../../ui/app/toolkits/gtkmm/src/workrave $pkgexecdir
fi
    
rm -rf $pkgrootdir/lib
rm -rf $pkgrootdir/libexec

if [ $conf_copy_gtk_dylib = true ]; then
    mv $pkgexecdir/$BINARY $pkgexecdir/${BINARY}-bin
    $INSTALL workrave $pkgexecdir
fi

cp Info.plist $pkgcontentsdir
$INSTALL ../../../u!i/data/images/macos/workrave.icns  $pkgresourcesdir

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
        > $pkgetcdir/pango/pango.modules
    
    mkdir -p $pkgetcdir/fonts
    cp $LIBPREFIX/etc/fonts/fonts.dtd $pkgetcdir/fonts/
    cp -r $LIBPREFIX/etc/fonts/conf.avail $pkgetcdir/fonts/
    cp -r $LIBPREFIX/etc/fonts/conf.d $pkgetcdir/fonts/
    cp fonts.conf $pkgetcdir/fonts
    
    cat > $pkgetcdir/pango/pangorc << END
[Pango]
ModuleFiles=./pango.modules
END

fi

# ------------------------------------------------------------
# Install Gtk
# ------------------------------------------------------------

if [ $conf_copy_gtk_dylib = true ]; then

    gtk_version=`pkg-config --variable=gtk_binary_version gtk+-2.0`
    
    mkdir -p $pkglibdir/gtk-2.0/$gtk_version/{engines,immodules,loaders}
    mkdir -p $pkgetcdir/gtk-2.0
    
    cp -r $LIBPREFIX/lib/gtk-2.0/$gtk_version/engines/* $pkglibdir/gtk-2.0/$gtk_version/engines/
    cp $LIBPREFIX/lib/gtk-2.0/$gtk_version/immodules/*.so $pkglibdir/gtk-2.0/$gtk_version/immodules/
    cp $LIBPREFIX/lib/gtk-2.0/$gtk_version/loaders/*.so $pkglibdir/gtk-2.0/$gtk_version/loaders/
    
    sed "s?$LIBPREFIX/lib/gtk-2.0/$gtk_version/loaders/?@executable_path/../Resources/lib/gtk-2.0/$gtk_version/loaders/?" \
        < $LIBPREFIX/etc/gtk-2.0/gdk-pixbuf.loaders \
        > $pkgetcdir/gtk-2.0/gdk-pixbuf.loaders
    
    sed "s?$LIBPREFIX/lib/gtk-2.0/$gtk_version/immodules/?@executable_path/../Resources/lib/gtk-2.0/$gtk_version/immodules/?" \
        < $LIBPREFIX/etc/gtk-2.0/gtk.immodules \
        > $pkgetcdir/gtk-2.0/gtk.immodules
    
    ## FIXME:
    mkdir -p $pkgthemedir/Leopardish-normal
    cp -r ~/.themes/Leopardish-normal/* $pkgthemedir/Leopardish-normal
    
    mkdir -p $pkgthemedir/Quartz
    cp -r $LIBPREFIX/share/themes/Quartz/gtk-2.0 $pkgthemedir/Quartz
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
        if [ x$libs != x ] ; then
            cp -f $libs $pkglibdir
        fi
        num_files=`ls $pkglibdir | wc -l`
        if [ $num_files = $total ];
        then
    	stop=true
        fi
        total=$num_files
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
# Importing Frameworks
# ------------------------------------------------------------

if [ "$conf_import_frameworks" = "true" ];
then
    echo "Importing Frameworks"

    ./import-frameworks.sh

    rm -rf $pkgframeworksdir/Gtk.framework/Versions/2/Resources/share/icons/gnome
    rm -rf $pkgframeworksdir/Gtk.framework/Versions/2/Resources/share/icons/hicolor
fi

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
    strip -xS `find $pkgframeworksdir -name "*.dylib" -print`

    for framework in $pkgframeworksdir/* ; do
        base=`basename $framework .framework`
        strip -xS $framework/$base
    done;

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


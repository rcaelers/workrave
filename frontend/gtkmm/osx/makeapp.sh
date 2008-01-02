#!/bin/bash
#
#
#

# Default settings.
conf_strip=false
conf_symlink=false

# ------------------------------------------------------------
# Print usage message
# ------------------------------------------------------------
usage()
{
echo -e "
Create Workrave app bundle for OS X

\033[1mUSAGE\033[0m
	$0 [-s]

\033[1mOPTIONS\033[0m
	\033[1m-h,--help\033[0m 
		display this help message
	\033[1m-s\033[0m
		strip the libraries and executables from debugging symbols
"
}

# ------------------------------------------------------------
# Parse command line arguments
# ------------------------------------------------------------
while [ "$1" != "" ]
do
    case $1 in
	-s)
	    conf_strip=true ;;

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

LIBPREFIX=/opt/gtk
pango_version=`pkg-config --variable=pango_module_version pango`
gtk_version=`pkg-config --variable=gtk_binary_version gtk+-2.0`

OSX_MINOR_VERSION=`/usr/bin/sw_vers | grep ProductVersion | cut -f2 -d'.'`

package=Workrave.app
package_version=`grep PACKAGE_VERSION ../../../config.h | cut -d' ' -f 3 | sed "s/\\"//g"`

pkgrootdir=`pwd`/$package
pkgcontentsdir=$pkgrootdir/Contents
pkgexecdir=$pkgcontentsdir/MacOS
pkgresourcesdir=$pkgcontentsdir/Resources
pkgframeworksdir=$pkgcontentsdir/Frameworks
pkgdatadir=$pkgresourcesdir/share
pkgetcdir=$pkgresourcesdir/etc
pkglibdir=$pkgresourcesdir/lib
binary=$pkgexecdir/workrave

sed -e "s?@ENV@?$env?g" -e "s?@VERSION@?$version?g" < Info.plist.in > Info.plist

# ------------------------------------------------------------
# Create directory layout
# ------------------------------------------------------------

echo
echo -e "\033[1mCreating Workrave app bundle\033[0m"
echo -e "\033[1mWorkrave version is $package_version\033[0m\n"

if [ -d $package ];
then
    echo "Removing old $package tree"
    rm -rf $package
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

make install -C ../../../ \
             prefix=$pkgrootdir bindir=$pkgexecdir pkgdatadir=$pkgresourcesdir \
             datadir=$pkgresourcesdir soundsdir=$pkgresourcesdir/sounds \
             DATADIRNAME=Contents/Resources > install.log 2>&1
rmdir $pkgrootdir/lib
rmdir $pkgrootdir/libexec

cp Info.plist $pkgcontentsdir
cp ../../../frontend/common/share/images/osx/workrave.icns  $pkgresourcesdir

echo "APPLWoRa" > $pkgcontentsdir/PkgInfo

# ------------------------------------------------------------
# Install Pango
# ------------------------------------------------------------

echo "Installing Pango modules"

mkdir -p $pkglibdir/pango/$pango_version/modules
cp $LIBPREFIX/lib/pango/$pango_version/modules/*.so $pkglibdir/pango/$pango_version/modules/

mkdir -p $pkgetcdir/pango
$LIBPREFIX/bin/pango-querymodules \
    | sed "s?$LIBPREFIX/lib/pango/$pango_version/modules/?@executable_path/../Resources/lib/pango/$pango_version/modules/?" \
    > $pkgetcdir/pango/pango.modules


# ------------------------------------------------------------
# Install Gtk
# ------------------------------------------------------------

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

# ------------------------------------------------------------
# Install dependencies
# ------------------------------------------------------------

echo "Installing dependencies"

total=0
stop=false
while ! $stop; do
    TARGETS=`find $pkgrootdir \( -name "*.so*" -or -name "*.dylib" \) -print`
    libs="`otool -L $TARGETS $binary 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $LIBPREFIX | sort | uniq`"
    cp -f $libs $pkglibdir
    num_files=`ls $pkglibdir | wc -l`
    if [ $num_files = $total ];
    then
	stop=true
    fi
    total=$num_files
done


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
    chmod +w $pkglibdir/*.dylib
    strip -x $pkglibdir/*.dylib
    strip -ur $binary
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


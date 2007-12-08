#!/bin/sh

# script for pulling together a MacOSX app bundle.
# Taken from Ardour2. TODO: add credit/copyright

GTKQUARTZ_ROOT=/opt/gtk

version=`grep PACKAGE_VERSION config.h | cut -d' ' -f 3 | sed "s/\\"//g"`
echo "Version is $version"

packagemaker=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker
pkgName="Workrave-${version}.pkg"

# setup directory structure

ROOT=`pwd`/Workrave.app
APPROOT=$ROOT/Contents
Frameworks=$APPROOT/Frameworks
Resources=$APPROOT/Resources
Shared=$Resources/share
Etc=$Resources/etc

echo "Removing old Workrave.app tree ..."

rm -rf Workrave.app

echo "Building new app directory structure ..."

# only bother to make the longest paths

mkdir -p $APPROOT/MacOS
mkdir -p $APPROOT/Resources
mkdir -p $Frameworks/modules
mkdir -p $Shared/templates
mkdir -p $Etc

# edit plist
sed -e "s?@ENV@?$env?g" -e "s?@VERSION@?$version?g" < Info.plist.in > Info.plist

cp Info.plist $APPROOT
cp frontend/common/share/images/osx/workrave.icns  $APPROOT/Resources

# copy executable
echo "Installing Workrave ..."

make install prefix=$ROOT bindir=$APPROOT/MacOS pkgdatadir=$Resources datadir=$Resources soundsdir=$Resources/sounds DATADIRNAME=Contents/Resources
rmdir $ROOT/lib
rmdir $ROOT/libexec

## strip $APPROOT/MacOS/workrave

# copy everything related to gtk-quartz
echo "Copying GTK-Quartz tree ..."
cp -R $GTKQUARTZ_ROOT/lib/*.dylib $Frameworks/
cp -R $GTKQUARTZ_ROOT/etc/* $Etc
echo "Copying all Pango modules ..."
cp -R $GTKQUARTZ_ROOT/lib/pango/1.6.0/modules/*.so $Frameworks/modules
echo "Copying all GDK Pixbuf loaders ..."
cp -R $GTKQUARTZ_ROOT/lib/gtk-2.0/2.10.0/loaders/*.so $Frameworks/modules
# charset alias file
cp -R $GTKQUARTZ_ROOT/lib/charset.alias $Resources

echo "Copying libraries ..."
cp -R /opt/gtk/lib/libgnet*.dylib $Frameworks
cp -R /opt/gtk/lib/libdbus*.dylib $Frameworks
cp -R /opt/gtk/lib/libiconv*.dylib $Frameworks

# generate new Pango module file
cat > pangorc <<EOF 
[Pango]
ModulesPath=$GTKQUARTZ_ROOT/lib/pango/1.6.0/modules
EOF
env PANGO_RC_FILE=pangorc $GTKQUARTZ_ROOT/bin/pango-querymodules | sed "s?$GTKQUARTZ_ROOT/lib/pango/1.6.0/modules/?@executable_path/../Frameworks/modules/?" > $Resources/pango.modules
rm pangorc

# generate a new GDK pixbufs loaders file
sed "s?$GTKQUARTZ_ROOT/lib/gtk-2.0/2.10.0/loaders/?@executable_path/../Frameworks/modules/?" < $GTKQUARTZ_ROOT/etc/gtk-2.0/gdk-pixbuf.loaders > $Resources/gdk-pixbuf.loaders

# this one is special - we will set GTK_PATH to $Frameworks/clearlooks
#cp $GTKQUARTZ_ROOT/lib/gtk-2.0/2.10.0/engines/libclearlooks.dylib $Frameworks
#mkdir -p $Frameworks/clearlooks/engines
#(cd $Frameworks/clearlooks/engines && ln -s ../../libclearlooks.dylib libclearlooks.dylib && ln -s ../../libclearlooks.dylib libclearlooks.so)

# now fix up the executables
echo "Fixing up executable dependency names ..."
for exe in Workrave; do
    EXE=$APPROOT/MacOS/$exe
    changes=""
    for lib in `otool -L $EXE | egrep "($GTKQUARTZ_ROOT|/local/|libs/)" | awk '{print $1}'` ; do
	base=`basename $lib`
	changes="$changes -change $lib @executable_path/../Frameworks/$base"
    done
    if test "x$changes" != "x" ; then
	install_name_tool $changes $EXE
    fi
done

echo "Fixing up library names ..."
# now do the same for all the libraries we include
for dylib in $Frameworks/*.dylib $Frameworks/modules/*.so ; do
    # skip symlinks
    if test ! -L $dylib ; then
	
	# change all the dependencies

	changes=""
	for lib in `otool -L $dylib | egrep "($GTKQUARTZ_ROOT|/local/|libs/)" | awk '{print $1}'` ; do
	    base=`basename $lib`
	    changes="$changes -change $lib @executable_path/../Frameworks/$base"
	done

	if test "x$changes" != x ; then
	    if  install_name_tool $changes $dylib ; then
		:
	    else
		exit 1
	    fi
	fi

	# now the change what the library thinks its own name is

	base=`basename $dylib`
	install_name_tool -id @executable_path/../Frameworks/$base $dylib
    fi
done

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


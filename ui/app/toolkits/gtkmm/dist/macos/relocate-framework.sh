#!/bin/bash
#
# Prepares a framework for installation in /Library/Frameworks/.
#
# Copyright (C) 2007, 2008 Imendio AB
#

print_help()
{
    echo "Usage: `basename $0` <path-to-framework-to-relocate> <old-path> <new-path>"
    exit 1
}

update_config_file()
{
    file=$1

    if [ ! -f "$file" ]; then
        return
    fi

    sed -e "s,$old_root,$new_root," "$file" > "$file.tmp" || exit 1
    mv "$file.tmp" "$file"

    if echo "$file" | grep "/dev/bin/" >/dev/null; then
        chmod +x $file
    fi
}

# Verify framework path.
if [ "x$*" = x ]; then
    print_help
    exit 1
fi

framework="$1"

# Check the path, and turn it into an absolute path if not absolute
# already.
if [ x`echo "$framework" | sed -e 's@\(^\/\).*@@'` != x ]; then
    framework=`pwd`/$framework
fi

if [ ! -d $framework ]; then
    echo "The directory $* does not exist"
    exit 1
fi

if [ ! -x $framework ]; then
    echo "The framework in $* is not accessible"
    exit 1
fi

# Drop any trailing slash.
basename=`basename "$framework"`
framework=`dirname "$framework"`/$basename

framework_name=`echo $basename | sed -e 's@\(^.*\)\..*@\1@'`

# Check framework directory for sanity.
if [ ! -d "$framework"/Resources -o ! -f "$framework/$framework_name" ]; then
    echo "$framework does not seem to be a valid framework"
    exit 1
fi

old_root=`dirname "$2"`
new_root=`dirname "$3"`

echo "Update library references in libraries..."
libs=`find "$framework" \( -name "*.dylib" -o -name "*.so" -o -name "$framework_name" \) -a -type f`
for lib in $libs; do
    new=`echo $lib | sed -e s,$old_root,$new_root,`
    install_name_tool -id "$new" "$lib" || exit 1

    deps=`otool -L $lib 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep "$old_root" | sort | uniq`
    for dep in $deps; do
        new=`echo $dep | sed -e s,$old_root,$new_root,`
        install_name_tool -change "$dep" "$new" "$lib" || exit 1
    done
done

#echo "Update dev symlinks..."
#tmp=/Library/Frameworks/$framework_name.framework/$framework_name
#ln -shf $tmp "$framework"/Resources/dev/lib/lib$framework_name.dylib || exit 1
## Handle glib's intl library here as well...
#if [ -f "$framework"/Resources/dev/lib/libintl.dylib ]; then
#    ln -shf $tmp "$framework"/Resources/dev/lib/libintl.dylib 2>/dev/null
#fi

echo "Update library references in executables..."
execs=`find "$framework"/Resources/dev/bin 2>/dev/null`
for exe in $execs; do
    if [ "x`file "$exe" | grep Mach-O\ executable`" != x ]; then
        deps=`otool -L $exe 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep "$old_root" | sort | uniq`
        for dep in $deps; do
            new=`echo $dep | sed -e s,$old_root,$new_root,`
            install_name_tool -change "$dep" "$new" "$exe" || exit 1
        done
    fi
done



# Handle some known locations, if we need to add lots more we can make
# it more generic.
echo "Update config files..."
update_config_file "$framework"/Resources/etc/pango/pango.modules
update_config_file "$framework"/Resources/etc/gtk-2.0/gdk-pixbuf.loaders
update_config_file "$framework"/Resources/etc/gtk-2.0/gtk.immodules
update_config_file "$framework"/Resources/dev/bin/glib-gettextize
update_config_file "$framework"/Resources/dev/bin/intltoolize

echo "Update pkg-config files..."
files=`ls "$framework"/Resources/dev/lib/pkgconfig/*.pc`
for file in $files; do
    update_config_file "$framework"/Resources/dev/lib/pkgconfig/`basename "$file"`
done

echo "Done."

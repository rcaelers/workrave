#!/bin/sh
# Run this to generate all the initial makefiles, etc.

DIE=0

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`autoconf' installed to compile $PKG_NAME."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
}
 

(grep "AC_PROG_INTLTOOL" $srcdir/configure.ac >/dev/null) && {
    (intltoolize --version) < /dev/null > /dev/null 2>&1 || {
        echo 
        echo "**Error**: You must have \`intltoolize' installed to compile $PKG_NAME."
        echo "Get ftp://ftp.gnome.org/pub/GNOME/stable/sources/intltool/intltool-0.10.tar.gz"
        echo "(or a newer version if it is available)"
        DIE=1
    }
}


(grep "^AM_PROG_XML_I18N_TOOLS" $srcdir/configure.ac >/dev/null) && {
    (xml-i18n-toolize --version) < /dev/null > /dev/null 2>&1 || {
        echo 
        echo "**Error**: You must have \`xml-i18n-toolize' installed to compile $PKG_NAME."
        echo "Get ftp://ftp.gnome.org/pub/GNOME/stable/sources/xml-i18n-tools/xml-i18n-tools-0.9.tar.gz"
        echo "(or a newer version of xml-i18n-tools or intltool if it is available)"
        DIE=1
    }
}
 

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
    (libtool --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "**Error**: You must have \`libtool' installed to compile $PKG_NAME."
        echo "Get ftp://ftp.gnu.org/pub/gnu/libtool/libtool-1.4.3.tar.gz"
        echo "(or a newer version if it is available)"
        DIE=1
    }
}


#grep "^AM_GNU_GETTEXT" $srcdir/configure.ac >/dev/null && {
#  grep "sed.*POTFILES" $srcdir/configure.ac >/dev/null || \
#  (gettext --version) < /dev/null > /dev/null 2>&1 || {
#    echo
#    echo "**Error**: You must have \`gettext' installed to compile $PKG_NAME."
#    echo "Get ftp://ftp.gnu.org/pub/gnu/gettext/gettext-0.10.39.tar.gz"
#    echo "(or a newer version if it is available)"
#    DIE=1
#  }
#}



#grep "^AM_GNOME_GETTEXT" $srcdir/configure.ac >/dev/null && {
#  grep "sed.*POTFILES" $srcdir/configure.ac >/dev/null || \
#  (gettext --version) < /dev/null > /dev/null 2>&1 || {
#    echo
#    echo "**Error**: You must have \`gettext' installed to compile $PKG_NAME."
#    echo "Get ftp://ftp.gnu.org/pub/gnu/gettext/gettext-0.10.39.tar.gz"
#    echo "(or a newer version if it is available)"
#    DIE=1
#  }
#}

 

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`automake' installed to compile $PKG_NAME."
    echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.7.1.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
    NO_AUTOMAKE=yes
}
 
 
# if no automake, don't bother testing for aclocal

test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
    echo "installed doesn't appear recent enough."
    echo "Get ftp://ftp.gnu.org/pub/gnu/automake/automake-1.7.1.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
}
 
if test "$DIE" -eq 1; then
    exit 1
fi
 
if test -z "$*"; then
    echo "**Warning**: I am going to run \`configure' with no arguments."
    echo "If you wish to pass any to it, please specify them on the"
    echo \`$0\'" command line."
    echo
fi

case $CC in
    xlc)
        am_opt=--include-deps;;
esac
 

#if grep "^AM_GNU_GETTEXT" configure.ac >/dev/null; then
#    if grep "sed.*POTFILES" configure.ac >/dev/null; then
#        : do nothing -- we still have an old unmodified configure.ac
#    else
#        echo "Creating $srcdir/aclocal.m4 ..."
#        
#        test -r $srcdir/aclocal.m4 || touch $srcdir/aclocal.m4
#        
#        echo "Running gettextize...  Ignore non-fatal messages."
#        echo "no" | gettextize --force --copy
#        echo "Making $srcdir/aclocal.m4 writable ..."
#        
#        test -r $srcdir/aclocal.m4 && chmod u+w $srcdir/aclocal.m4
#    fi
#fi
#          
#if grep "^AM_GNOME_GETTEXT" configure.ac >/dev/null; then
#    echo "Creating $srcdir/aclocal.m4 ..."
#    test -r $srcdir/aclocal.m4 || touch $srcdir/aclocal.m4
#    echo "Running gettextize...  Ignore non-fatal messages."
#    echo "no" | gettextize --force --copy
#    echo "Making $srcdir/aclocal.m4 writable ..."
#    test -r $srcdir/aclocal.m4 && chmod u+w $srcdir/aclocal.m4
#fi

if grep "AC_PROG_INTLTOOL" configure.ac >/dev/null; then
    echo "Running intltoolize..."
    intltoolize --copy --force --automake
fi

if grep "^AM_PROG_XML_I18N_TOOLS" configure.ac >/dev/null; then
    echo "Running xml-i18n-toolize..."
    xml-i18n-toolize --copy --force --automake
fi

if grep "^AM_PROG_LIBTOOL" configure.ac >/dev/null; then
    if test -z "$NO_LIBTOOLIZE" ; then 
        echo "Running libtoolize..."
        libtoolize --force --copy
    fi
fi

echo "Running aclocal $aclocalinclude ..."
aclocal $aclocalinclude -I m4 || {
    echo
    echo "**Error**: aclocal failed. This may mean that you have not"
    echo "installed all of the packages you need, or you may need to"
    echo "set ACLOCAL_FLAGS to include \"-I \$prefix/share/aclocal\""
    echo "for the prefix where you installed the packages whose"
    echo "macros were not found"
    exit 1
}


if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
    echo "Running autoheader..."
    autoheader --force || {
        echo "**Error**: autoheader failed."
        exit 1
    }
fi

echo "Running automake --foreign $am_opt ..."
automake --add-missing --foreign  --copy $am_opt || {
    echo "**Error**: automake failed.";
    exit 1;
}

echo "Running autoconf ..."
autoconf --force || {
    echo "**Error**: autoconf failed."
    exit 1
}

conf_flags=" --target=i386-mingw32msvc --host=i386-mingw32msvc --build=i386-linux  --enable-maintainer-mode --enable-debug --without-x --enable-distribution --enable-exercises"
 
if test x$NOCONFIGURE = x; then
    
    echo Running $srcdir/configure $conf_flags "$@" ...
    $srcdir/configure $conf_flags "$@"  && echo Now type \`make\' to compile $PKG_NAME || exit 1
    
else
    
    echo Skipping configure process.

 fi

#!/bin/sh
tagline=`head -1 $1`
tag=`echo $tagline | grep "\-\-\-" | sed -e 's/^.*\(--- .*\)/\1/'`
copy=`grep "Copyright (C)" $1 | sed -e 's/^.* \(Copyright (C).*\)/\1/'`
time=`grep "Time.stamp: <" $1 | sed -e 's/^.* Time.stamp: <\(.*\)>/\1/'`
id=`grep "\\$Id$" $1 | sed -e 's/^.*\\$Id$/\1/'`

echo tagline = $tagline
echo tag = $tag
echo copy = $copy
echo ts = $time
echo id = $id


sed '/^$/q' < $1 > $1.hdr
sed '1,/^$/d' < $1 > tmp.$$

ext=`echo $1 | sed -e 's/^.*\.\(.*\)/\1/'`

if [ $ext == 'cc' -o $ext == 'icc' ] ; then
    echo C++
cat <<EOF > tmp.$$
// `basename $1` $tag
//
// $copy
// All rights reserved.
//
// Time-stamp: <$time>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

EOF
elif [ $ext == 'hh' ] ; then
    echo H++
cat <<EOF > tmp.$$
// `basename $1` $tag
//
// $copy
// All rights reserved.
//
// Time-stamp: <$time>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// \$Id${id}$
//

EOF
elif [ $ext == 'c'] ; then
    echo C
elif [ $ext == 'h' ] ; then
    echo H
fi    

sed '1,/^$/d' < $1 >> tmp.$$
mv $1 $1.backup
mv tmp.$$ $1


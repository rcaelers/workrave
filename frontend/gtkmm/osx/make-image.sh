# Copyright (C) 2008 Rob Caelers <robc@krandor.nl>
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

DMG_FILENAME=Workrave.dmg
volume_name=Workrave

# ------------------------------------------------------------
# Create image
# ------------------------------------------------------------

echo "Copying disk image content."

dmg_dir=workrave-image

rm -rf "$dmg_dir"
mkdir "$dmg_dir"

cp -rf Workrave.app "$dmg_dir"
ln -sf /Applications "$dmg_dir"

mkdir "$dmg_dir/.background"
cp dmg_background.png "$dmg_dir/.background/background.png"

# ------------------------------------------------------------
# Create image
# ------------------------------------------------------------

echo "Creating disk image"

dmg_temp_filename=RW.${DMG_FILENAME}

rm -f $dmg_temp_filename
/usr/bin/hdiutil create -srcfolder "$dmg_dir" \
                        -volname "$volume_name" \
			-fs HFS+ -fsargs "-c c=64,a=16,e=16" \
			-format UDRW "$dmg_temp_filename"

echo "Mounting disk image"
mount_dir="/Volumes/$volume_name"
device_name=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${dmg_temp_filename}" | egrep '^/dev/' | sed 1q | awk '{print $1}'`

echo "Setting layout"
osascript dmg.applescript

echo "Fixing permissions..."
chmod -Rf go-w "${mount_dir}"

echo "Unmounting disk image..."
hdiutil detach "${device_name}"

# ------------------------------------------------------------
# Compressing
# ------------------------------------------------------------

echo "Compressing disk image..."

rm -f "${DMG_FILENAME}"
hdiutil convert "${dmg_temp_filename}"  \
                -format UDZO            \
		-imagekey zlib-level=9  \
		-o "${DMG_FILENAME}"
rm -f "${dmg_temp_filename}"

# ------------------------------------------------------------
# Adding EULA resource
# ------------------------------------------------------------

echo "adding EULA resources"

echo "
data 'LPic' (5000)
{
  $\"0000\"    		// Default language ID, 0 = English
  $\"0001\"    		// Number of entries
  
  $\"0000\"    		// Language ID
  $\"0000\"    		// Resource ID, 0 = STR#/TEXT/styl 5000
  $\"0000\"    		// Multibyte language, 0 = no
};

data 'styl' (5000, \"English\") {
  
  $\"0001\"             // Number of styles

  // Style 1
  $\"0000 0000\"        // Start character = 0
  $\"0010\"         	// Height
  $\"000C\"         	// Ascent
  $\"0400\"         	// Font family (Lucida Grande)
  $\"00\"           	// Style 0x01=bold 0x02=italic 0x04=underline 0x8=outline
                    	//       0x10=shadow 0x20=condensed 0x40=extended
  $\"02\"               // Style, unused?
  $\"000A\"             // Size in pt
  $\"0000 0000 0000\"   // Color, RGB
};

resource 'STR#' (5000, \"English buttons\") {
    {   /* array StringArray: 9 elements */
        \"English\",
        \"Agree\",
        \"Disagree\",
        \"Print...\",
        \"Save...\",
        \"IMPORTANT - Read this License Agreement carefully before clicking on \"
        \"the \\"Agree\\" button.  By clicking on the \\"Agree\\" button, you agree \"
        \"to be bound by the terms of the License Agreement.\",
        \"Software License Agreement\",
        \"This text cannot be saved. This disk may be full or locked, or the file \"
        \"may be locked.\",
        \"Unable to print. Make sure you've selected a printer.\"
    }
};

data 'TEXT' (5000, \"English\") {
" > tmp.r

sed -e 's/"/\\"/g' -e 's/\(.*\)$/"\1\\n"/g' ../../../COPYING >> tmp.r

echo "
};
" >> tmp.r

hdiutil unflatten "${DMG_FILENAME}"
/Developer/Tools/Rez /Developer/Headers/FlatCarbon/*.r tmp.r -a -o "${DMG_FILENAME}"
hdiutil flatten "${DMG_FILENAME}"

echo "Disk image done"
exit 0
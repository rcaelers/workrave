# ------------------------------------------------------------
# Settings
# ------------------------------------------------------------

dmg_name=Workrave.dmg
dmg_dir=workrave-image
volume_name=Workrave

dmg_tmp_name=RW.${dmg_name}

# ------------------------------------------------------------
# Create image
# ------------------------------------------------------------

echo "Copying disk image content."

rm -rf "$dmg_dir"
mkdir "$dmg_dir"

cp -rf Workrave.app "$dmg_dir"
#cp -f *webloc "$dmg_dir"
ln -sf /Applications "$dmg_dir"

mkdir "$dmg_dir/.background"
cp dmg_background.png "$dmg_dir/.background/background.png"

# ------------------------------------------------------------
# Create image
# ------------------------------------------------------------

echo "Creating disk image"
rm -f $dmg_tmp_name
/usr/bin/hdiutil create -srcfolder "$dmg_dir" -volname "$volume_name" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW "$dmg_tmp_name"

echo "Mounting disk image"
MOUNT_DIR="/Volumes/$volume_name"
DEV_NAME=`/usr/bin/hdiutil attach -readwrite -noverify -noautoopen "${dmg_tmp_name}" | egrep '^/dev/' | sed 1q | awk '{print $1}'`

echo "Setting layout"
osascript dmg.applescript

echo "Fixing permissions..."
chmod -Rf go-w "${MOUNT_DIR}"

echo "Unmounting disk image..."
hdiutil detach "${DEV_NAME}"

# ------------------------------------------------------------
# Compressing
# ------------------------------------------------------------

echo "Compressing disk image..."

rm -f "${dmg_name}"
hdiutil convert "${dmg_tmp_name}" -format UDZO -imagekey zlib-level=9 -o "${dmg_name}"
rm -f "${dmg_tmp_name}"

# ------------------------------------------------------------
# Adding EULA resource
# ------------------------------------------------------------

echo "adding EULA resources"

echo "
data 'LPic' (5000)
{
  // Default language ID, 0 = English
  $\"0000\"
  // Number of entries in list
  $\"0001\"

  // Entry 1
  // Language ID, 0 = English
  $\"0000\"
  // Resource ID, 0 = STR#/TEXT/styl 5000
  $\"0000\"
  // Multibyte language, 0 = no
  $\"0000\"
};

data 'styl' (5000, \"English\") {
  // Number of styles following = 1
  $\"0001\"

  // Style 1.  This is used to display the first two lines in bold text.
  // Start character = 0
  $\"0000 0000\"
  // Height = 16
  $\"0010\"
  // Ascent = 12
  $\"000C\"
  // Font family = 1024 (Lucida Grande)
  $\"0400\"
  // Style bitfield, 0x1=bold 0x2=italic 0x4=underline 0x8=outline
  // 0x10=shadow 0x20=condensed 0x40=extended
  $\"00\"
  // Style, unused?
  $\"02\"
  // Size = 12 point
  $\"000A\"
  // Color, RGB
  $\"0000 0000 0000\"
};

resource 'STR#' (5000, \"English\") {
  {
    // Language (unused?) = English
    \"English\",
    
    // Agree
    \"Agree\",

    // Disagree
    \"Disagree\",

    // Print, ellipsis is 0xC9
    \"Print...\",

    // Save As, ellipsis is 0xC9
    \"Save As...\",

    // Descriptive text, curly quotes are 0xD2 and 0xD3
    \"If you agree to the terms of this license agreement, click \`Agree\' to access the software.  If you \"
    \"do not agree, press \`Disagree.\'\"
  }
};

data 'TEXT' (5000, \"English\") {
" > tmp.r

sed -e 's/"/\\"/g' -e 's/\(.*\)$/"\1\\n"/g' ../../../COPYING >> tmp.r

echo "
};
" >> tmp.r

hdiutil unflatten "${dmg_name}"
/Developer/Tools/Rez /Developer/Headers/FlatCarbon/*.r tmp.r -a -o "${dmg_name}"
hdiutil flatten "${dmg_name}"

echo "Disk image done"
exit 0
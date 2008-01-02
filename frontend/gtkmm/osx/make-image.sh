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
cp -f *webloc "$dmg_dir"
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

hdiutil convert "${dmg_tmp_name}" -format UDZO -imagekey zlib-level=9 -o "${dmg_name}"
rm -f "${dmg_tmp_name}"

# ------------------------------------------------------------
# Adding EULA resource
# ------------------------------------------------------------

echo "adding EULA resources"

hdiutil unflatten "${dmg_name}"
/Developer/Tools/ResMerger -a "${EULA_RSRC}" -o "${dmg_name}"
hdiutil flatten "${dmg_name}"

echo "Disk image done"
exit 0

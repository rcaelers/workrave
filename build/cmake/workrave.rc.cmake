#include <winver.h>

VS_VERSION_INFO VERSIONINFO
  FILEVERSION ${RESOURCE_VERSION}
  PRODUCTVERSION ${RESOURCE_VERSION}
  FILEFLAGSMASK 0
  FILEFLAGS 0
  FILEOS VOS__WINDOWS32
  FILETYPE VFT_APP
  FILESUBTYPE VFT2_UNKNOWN
  BEGIN
    BLOCK "StringFileInfo"
    BEGIN
      BLOCK "040904B0"
      BEGIN
        VALUE "CompanyName", "The Workrave development team"
        VALUE "FileDescription", "Workrave"
        VALUE "FileVersion", "${VERSION}"
        VALUE "InternalName", "workrave"
        VALUE "LegalCopyright", "Copyright (C) 2001-2012 The Workrave development team."
        VALUE "OriginalFilename", "workrave.exe"
        VALUE "ProductName", "Workrave"
        VALUE "ProductVersion", "${VERSION}"
      END
    END
    BLOCK "VarFileInfo"
    BEGIN
      VALUE "Translation", 0x409, 1200
    END
  END

workrave	ICON    DISCARDABLE     "../../common/share/images/win32/workrave.ico"
workravequiet	ICON    DISCARDABLE     "../../common/share/images/win32/workrave-quiet.ico"
workravesusp	ICON    DISCARDABLE     "../../common/share/images/win32/workrave-suspended.ico"

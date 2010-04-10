# -*- cmake -*-
set(DIRECTX_FOUND TRUE)

find_path(DIRECTX_INCLUDES dxdiag.h
            "$ENV{DXSDK_DIR}/Include"
	    "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (February 2010)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2009)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (March 2009)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2008)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2008)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (March 2008)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (November 2007)/Include"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2007)/Include"
            "C:/DX90SDK/Include"
            "$ENV{PROGRAMFILES}/DX90SDK/Include"
            )
if (NOT DIRECTX_INCLUDES)
   set(DIRECTX_FOUND FALSE)
endif (NOT DIRECTX_INCLUDES)


find_path(DIRECTX_LIBS dxguid.lib
            "$ENV{DXSDK_DIR}/Lib/x86"
	    "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (February 2010)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2009)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (March 2009)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2008)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (June 2008)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (March 2008)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (November 2007)/Lib/x86"
            "$ENV{PROGRAMFILES}/Microsoft DirectX SDK (August 2007)/Lib/x86"
            "C:/DX90SDK/Lib"
            "$ENV{PROGRAMFILES}/DX90SDK/Lib"
            )

if (NOT DIRECTX_LIBS)
   set(DIRECTX_FOUND FALSE)
endif (NOT DIRECTX_LIBS)


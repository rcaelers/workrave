PROJECT(Harpoon)

set(SOURCES 
  ../../../common/win32/harpoon/src/harpoon.c
  ../../../common/win32/harpoon/src/harpoon.def
  ../../../common/win32/harpoon/include/harpoon.h
)

include_directories( 
  ../../../common/win32/harpoon/include
)

add_definitions( 
  -DHARPOON_EXPORTS
  -DWIN32
  -DWINVER=0x400
  -D_USRDLL
  -D_WIN32_WINNT=0x500
  -D_WINDOWS
)

add_library(harpoon SHARED ${SOURCES})

target_link_libraries(harpoon kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32)

install(TARGETS harpoon
        RUNTIME DESTINATION lib
)

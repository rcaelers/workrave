# Microsoft Developer Studio Generated NMAKE File, Based on harpoon.dsp
!IF "$(CFG)" == ""
CFG=harpoon - Win32 Debug
!MESSAGE No configuration specified. Defaulting to harpoon - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "harpoon - Win32 Release" && "$(CFG)" != "harpoon - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "harpoon.mak" CFG="harpoon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "harpoon - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "harpoon - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "harpoon - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\harpoon.dll"


CLEAN :
	-@erase "$(INTDIR)\harpoon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\harpoon.dll"
	-@erase "$(OUTDIR)\harpoon.exp"
	-@erase "$(OUTDIR)\harpoon.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HARPOON_EXPORTS" /D _WIN32_WINNT=0x400 /D WINVER=0x400 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\harpoon.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\harpoon.pdb" /machine:I386 /def:".\harpoon.def" /out:"$(OUTDIR)\harpoon.dll" /implib:"$(OUTDIR)\harpoon.lib" 
DEF_FILE= \
	".\harpoon.def"
LINK32_OBJS= \
	"$(INTDIR)\harpoon.obj"

"$(OUTDIR)\harpoon.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "harpoon - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\harpoon.dll"


CLEAN :
	-@erase "$(INTDIR)\harpoon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\harpoon.dll"
	-@erase "$(OUTDIR)\harpoon.exp"
	-@erase "$(OUTDIR)\harpoon.ilk"
	-@erase "$(OUTDIR)\harpoon.lib"
	-@erase "$(OUTDIR)\harpoon.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HARPOON_EXPORTS" /D _WIN32_WINNT=0x400 /D WINVER=0x400 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\harpoon.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\harpoon.pdb" /debug /machine:I386 /def:".\harpoon.def" /out:"$(OUTDIR)\harpoon.dll" /implib:"$(OUTDIR)\harpoon.lib" /pdbtype:sept 
DEF_FILE= \
	".\harpoon.def"
LINK32_OBJS= \
	"$(INTDIR)\harpoon.obj"

"$(OUTDIR)\harpoon.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("harpoon.dep")
!INCLUDE "harpoon.dep"
!ELSE 
!MESSAGE Warning: cannot find "harpoon.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "harpoon - Win32 Release" || "$(CFG)" == "harpoon - Win32 Debug"
SOURCE=.\harpoon.c

"$(INTDIR)\harpoon.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 


# Microsoft Developer Studio Generated NMAKE File, Based on Applet.dsp
!IF "$(CFG)" == ""
CFG=Applet - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Applet - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Applet - Win32 Release" && "$(CFG)" != "Applet - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Applet.mak" CFG="Applet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Applet - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Applet - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Applet - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\workrave-applet.dll"


CLEAN :
	-@erase "$(INTDIR)\Applet.obj"
	-@erase "$(INTDIR)\ClsFact.obj"
	-@erase "$(INTDIR)\DeskBand.obj"
	-@erase "$(INTDIR)\Icon.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\TimeBar.obj"
	-@erase "$(INTDIR)\TimerBox.obj"
	-@erase "$(INTDIR)\Util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\workrave-applet.dll"
	-@erase "$(OUTDIR)\workrave-applet.exp"
	-@erase "$(OUTDIR)\workrave-applet.lib"
	-@erase ".\Release\Register.out"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../include" /I "..\..\..\..\backend\include" /I "..\..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\Applet.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Applet.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  shlwapi.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\workrave-applet.pdb" /machine:I386 /def:".\Applet.def" /out:"$(OUTDIR)\workrave-applet.dll" /implib:"$(OUTDIR)\workrave-applet.lib" 
DEF_FILE= \
	".\Applet.def"
LINK32_OBJS= \
	"$(INTDIR)\Applet.obj" \
	"$(INTDIR)\ClsFact.obj" \
	"$(INTDIR)\DeskBand.obj" \
	"$(INTDIR)\Icon.obj" \
	"$(INTDIR)\TimeBar.obj" \
	"$(INTDIR)\TimerBox.obj" \
	"$(INTDIR)\Util.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\workrave-applet.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
InputPath=.\Release\workrave-applet.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\Register.out" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32.exe /s /c $(OUTDIR)\workrave-applet.dll
<< 
	

!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\workrave-applet.dll" "$(OUTDIR)\Applet.bsc"


CLEAN :
	-@erase "$(INTDIR)\Applet.obj"
	-@erase "$(INTDIR)\Applet.sbr"
	-@erase "$(INTDIR)\ClsFact.obj"
	-@erase "$(INTDIR)\ClsFact.sbr"
	-@erase "$(INTDIR)\DeskBand.obj"
	-@erase "$(INTDIR)\DeskBand.sbr"
	-@erase "$(INTDIR)\Icon.obj"
	-@erase "$(INTDIR)\Icon.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\TimeBar.obj"
	-@erase "$(INTDIR)\TimeBar.sbr"
	-@erase "$(INTDIR)\TimerBox.obj"
	-@erase "$(INTDIR)\TimerBox.sbr"
	-@erase "$(INTDIR)\Util.obj"
	-@erase "$(INTDIR)\Util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Applet.bsc"
	-@erase "$(OUTDIR)\workrave-applet.dll"
	-@erase "$(OUTDIR)\workrave-applet.exp"
	-@erase "$(OUTDIR)\workrave-applet.ilk"
	-@erase "$(OUTDIR)\workrave-applet.lib"
	-@erase "$(OUTDIR)\workrave-applet.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "..\..\..\..\backend\include" /I "..\..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Applet.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Applet.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Applet.sbr" \
	"$(INTDIR)\ClsFact.sbr" \
	"$(INTDIR)\DeskBand.sbr" \
	"$(INTDIR)\Icon.sbr" \
	"$(INTDIR)\TimeBar.sbr" \
	"$(INTDIR)\TimerBox.sbr" \
	"$(INTDIR)\Util.sbr"

"$(OUTDIR)\Applet.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  shlwapi.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\workrave-applet.pdb" /debug /machine:I386 /def:".\Applet.def" /out:"$(OUTDIR)\workrave-applet.dll" /implib:"$(OUTDIR)\workrave-applet.lib" /pdbtype:sept 
DEF_FILE= \
	".\Applet.def"
LINK32_OBJS= \
	"$(INTDIR)\Applet.obj" \
	"$(INTDIR)\ClsFact.obj" \
	"$(INTDIR)\DeskBand.obj" \
	"$(INTDIR)\Icon.obj" \
	"$(INTDIR)\TimeBar.obj" \
	"$(INTDIR)\TimerBox.obj" \
	"$(INTDIR)\Util.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\workrave-applet.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Applet.dep")
!INCLUDE "Applet.dep"
!ELSE 
!MESSAGE Warning: cannot find "Applet.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Applet - Win32 Release" || "$(CFG)" == "Applet - Win32 Debug"
SOURCE=.\Applet.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\Applet.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\Applet.obj"	"$(INTDIR)\Applet.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ClsFact.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\ClsFact.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\ClsFact.obj"	"$(INTDIR)\ClsFact.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\DeskBand.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\DeskBand.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\DeskBand.obj"	"$(INTDIR)\DeskBand.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Icon.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\Icon.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\Icon.obj"	"$(INTDIR)\Icon.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\TimeBar.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\TimeBar.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\TimeBar.obj"	"$(INTDIR)\TimeBar.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TimerBox.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\TimerBox.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\TimerBox.obj"	"$(INTDIR)\TimerBox.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Util.cpp

!IF  "$(CFG)" == "Applet - Win32 Release"


"$(INTDIR)\Util.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Applet - Win32 Debug"


"$(INTDIR)\Util.obj"	"$(INTDIR)\Util.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 


@ECHO OFF

SET MSSdk="C:\Program Files\Microsoft SDKs\Windows\v6.0"

IF "%~1"=="DEBUG64" (
   call %MSSdk%\bin/setenv /x64 /Debug
   msbuild Applet.sln /t:Clean;Rebuild /p:Configuration=Debug64;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="RELEASE64" (
  call %MSSdk%\bin/setenv.cmd /x64 /Release
  msbuild Applet.sln /t:Clean;Rebuild /p:Configuration=Release64;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="DEBUG" (
  call %MSSdk%\bin/setenv.cmd /x86 /Debug
  msbuild Applet.sln /t:Clean;Rebuild /p:Configuration=Debug;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="RELEASE" (
  call %MSSdk%\bin/setenv.cmd /x86 /Release
  msbuild Applet.sln /t:Clean;Rebuild /p:Configuration=Release;VCBuildUseEnvironment=true
)
 

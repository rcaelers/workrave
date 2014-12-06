@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

SET MSSdk="C:\Program Files\Microsoft SDKs\Windows\v7.0"

IF "%~1"=="DEBUG64" (
  call %MSSdk%\bin/setenv /x64 /Debug
  msbuild harpoon.sln /t:Clean;Rebuild /p:Configuration=Debug64;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="RELEASE64" (
  call %MSSdk%\bin/setenv.cmd /x64 /Release
  msbuild harpoon.sln /t:Clean;Rebuild /p:Configuration=Release64;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="DEBUG" (
  call %MSSdk%\bin/setenv.cmd /x86 /Debug
  msbuild harpoon.sln /t:Clean;Rebuild /p:Configuration=Debug;VCBuildUseEnvironment=true
) ELSE IF "%~1"=="RELEASE" (
  call %MSSdk%\bin/setenv.cmd /x86 /Release
  msbuild harpoon.sln /t:Clean;Rebuild /p:Configuration=Release;VCBuildUseEnvironment=true
)
 

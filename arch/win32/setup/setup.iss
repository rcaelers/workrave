; setup.iss --- Inno setup file
;
; Copyright (C) 2002 Raymond Penners <raymond@dotsphinx.com>
; All rights reserved.
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2, or (at your option)
; any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; $Id$

[Setup]
AppName=Workrave
AppVerName=Workrave 0.2.0
AppPublisher=Rob Caelers & Raymond Penners
AppPublisherURL=http://workrave.sourceforge.net
AppSupportURL=http://workrave.sourceforge.net
AppUpdatesURL=http://workrave.sourceforge.net
DefaultDirName={pf}\Workrave
DefaultGroupName=Workrave
LicenseFile=..\..\..\COPYING
AlwaysCreateUninstallIcon=yes

; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional tasks:"; MinVersion: 4,4
Name: "startupmenu"; Description: "Start Workrave when Windows starts"; GroupDescription: "Additional tasks:"; MinVersion: 4,4

[Files]
Source: ".\runtime\*.*"; DestDir: "{app}"; CopyMode: alwaysoverwrite; Flags: recursesubdirs
Source: "..\..\..\images\*.png"; DestDir: "{app}\share\images\"; CopyMode: alwaysoverwrite; Flags: recursesubdirs
Source: "..\..\..\sounds\*.wav"; DestDir: "{app}\share\sounds\"; CopyMode: alwaysoverwrite; Flags: recursesubdirs
Source: "..\..\..\COPYING.txt"; DestDir: "{app}"; DestName: "COPYING.txt"; CopyMode: alwaysoverwrite;
Source: "..\..\..\AUTHORS.txt"; DestDir: "{app}"; DestName: "AUTHORS.txt"; CopyMode: alwaysoverwrite;
Source: "..\..\..\NEWS.txt"; DestDir: "{app}"; DestName: "NEWS.txt"; CopyMode: alwaysoverwrite;
Source: "..\..\..\README.txt"; DestDir: "{app}"; DestName: "README.txt"; CopyMode: alwaysoverwrite;
Source: "..\..\..\src\workrave.exe"; DestDir: "{app}"; DestName: "Workrave.exe"; CopyMode: alwaysoverwrite;

[Registry]
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Workrave.exe"; ValueType: string; ValueData: "{app}\Workrave.exe"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\Workrave.exe"; ValueName: "Path"; ValueType: string; ValueData: "{app}\lib"; Flags: uninsdeletekeyifempty

[Icons]
Name: "{group}\Workrave"; Filename: "{app}\workrave.exe"
Name: "{group}\News"; Filename: "{app}\NEWS.txt"
Name: "{group}\Read me"; Filename: "{app}\README.txt"
Name: "{group}\License"; Filename: "{app}\COPYING.txt"
Name: "{userdesktop}\Workrave"; Filename: "{app}\workrave.exe"; MinVersion: 4,4; Tasks: desktopicon
Name: "{userstartup}\Workrave"; Filename: "{app}\workrave.exe"; MinVersion: 4,4; Tasks: startupmenu


;[Run]
;Filename: "{app}\workrave.exe"; Description: "Launch Workrave"; Flags: nowait postinstall skipifsilent



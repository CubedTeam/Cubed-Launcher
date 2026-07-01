#ifndef CubedLauncherVersion
#define CubedLauncherVersion "dev"
#endif

[Setup]
AppName=Cubed Launcher
AppVersion={#CubedLauncherVersion}
DefaultDirName={autopf}\Cubed Launcher
DefaultGroupName=Cubed Launcher

OutputDir=output
OutputBaseFilename=CubedLauncher-{#CubedLauncherVersion}-windows-x64-setup

Compression=lzma
SolidCompression=yes

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

UninstallDisplayIcon={app}\CubedLauncher.exe

VersionInfoVersion={#CubedLauncherVersion}
VersionInfoProductName=Cubed Launcher
VersionInfoDescription=Cubed Launcher Installer
VersionInfoCompany=CubedTeam

[Files]
Source: "..\build\CubedLauncher\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Cubed Launcher"; Filename: "{app}\CubedLauncher.exe"
Name: "{commondesktop}\Cubed Launcher"; Filename: "{app}\CubedLauncher.exe"

[Run]
Filename: "{app}\CubedLauncher.exe"; Description: "Launch Cubed Launcher"; Flags: nowait postinstall skipifsilent
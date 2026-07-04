#ifndef CubedLauncherVersion
#define CubedLauncherVersion "dev"
#endif

#if CubedLauncherVersion == "dev"
  #define CubedLauncherFileVersion "0.0.0.0"
#else
  #define CubedLauncherFileVersion CubedLauncherVersion + ".0"
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

VersionInfoVersion={#CubedLauncherFileVersion}
VersionInfoProductName=Cubed Launcher
VersionInfoDescription=Cubed Launcher Installer
VersionInfoCompany=CubedTeam

[Files]
Source: "..\build\CubedLauncher\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Cubed Launcher"; Filename: "{app}\CubedLauncher.exe"
Name: "{commondesktop}\Cubed Launcher"; Filename: "{app}\CubedLauncher.exe"

[Code]

function NeedsVCRedist: Boolean;
var
  Installed: Cardinal;
begin
  Result :=
    (not RegQueryDWordValue(
      HKLM64,
      'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
      'Installed',
      Installed))
    or (Installed = 0);
end;

[Run]
Filename: "{app}\vc_redist.x64.exe";
Parameters: "/install /quiet /norestart";
Flags: waituntilterminated;
Check: NeedsVCRedist

Filename: "{app}\CubedLauncher.exe"; Description: "Launch Cubed Launcher"; Flags: nowait postinstall skipifsilent

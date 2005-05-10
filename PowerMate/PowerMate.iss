; PowerMate Inno Setup Script
; $Header$

[Setup] 
AppName=PowerMate
AppVerName=PowerMate 3.0
AppID=PowerMate
AppPublisherURL=http://www.promixis.com/products.php?section=girder
AppSupportURL=http://www.promixis.com/phpBB2/
AppVersion=1.66
DefaultDirName={reg:HKLM\SOFTWARE\Promixis\Girder\4,PlugInDir|{pf}\girder4\plugins}
DirExistsWarning=no 
;LicenseFile=license.txt
OutputBaseFilename=PowerMate
OutputDir=..\..
SetupIconFile=src\plugin.ico

[Types]
Name: "compact"; Description: "Minimal installation"
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "Required Files"; Types: full compact custom; Flags: fixed
Name: "readme"; Description: "Readme File"; Types: full compact
Name: "sample"; Description: "Samples"; Types: full

[Files] 
Source: "..\PowerMate.dll"; DestDir: "{app}"; Components: main; Flags: promptifolder
Source: "PowerMate.xml"; DestDir: "{app}\UI"; Components: main; Flags: promptifolder
Source: "readme.htm"; DestDir: "{app}\PowerMate"; Components: readme; Flags: isreadme
Source: "license.txt"; DestDir: "{app}\PowerMate"; Components: readme
Source: "Test.GML"; DestDir: "{app}\PowerMate"; Components: sample; Flags: promptifolder
Source: "TestMenu.ini"; DestDir: "{app}\PowerMate"; Components: sample; Flags: promptifolder

[Registry] 
Root: HKLM; Subkey: "SOFTWARE\Promixis\Girder\4\Plugins"; ValueName: 44; ValueType: dword; ValueData: 1

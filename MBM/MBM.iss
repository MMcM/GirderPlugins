; MBM Inno Setup Script
; $Header$

[Setup] 
AppName=MBM
AppVerName=MBM 2.0
AppID=MBM
AppPublisherURL=http://www.promixis.com/products.php?section=girder
AppSupportURL=http://www.promixis.com/phpBB2/
AppVersion=1.66
DefaultDirName={reg:HKLM\SOFTWARE\Promixis\Girder\4,PlugInDir|{pf}\girder4\plugins}\MBM
DirExistsWarning=no 
;LicenseFile=license.txt
OutputBaseFilename=MBM
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
Source: "..\MBM.dll"; DestDir: "{app}\.."; Components: main; AFlags: promptifolder
Source: "MBM.xml"; DestDir: "{app}\..\UI"; Components: main; Flags: promptifolder
Source: "readme.htm"; DestDir: "{app}"; Components: readme; Flags: isreadme
Source: "license.txt"; DestDir: "{app}"; Components: readme
Source: "Test.GML"; DestDir: "{app}"; Components: sample; Flags: promptifolder

[Registry] 
Root: HKLM; Subkey: "SOFTWARE\Promixis\Girder\4\Plugins"; ValueName: 46; ValueType: dword; ValueData: 1

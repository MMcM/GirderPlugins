; Girder LCD Inno Setup Script
; $Header$

[Setup] 
AppName=LCD
AppVerName=LCD 3.7
AppID=Girder_LCD
AppPublisherURL=http://www.promixis.com/products.php?section=girder
AppSupportURL=http://www.promixis.com/phpBB2/
AppVersion=3.7
DefaultDirName={reg:HKLM\SOFTWARE\girder3,PlugInDir|{pf}\girder\plugins}\..
DirExistsWarning=no 
OutputBaseFilename=LCD
OutputDir=.
SourceDir=..\..
UninstallDisplayName=Girder LCD 3.7

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Minimal installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "Required Files"; Types: full compact custom; Flags: fixed
Name: "lcdriver"; Description: "LCDriver Driver Library"; Types: full
Name: "crystalfontz"; Description: "Crystalfontz Displays"; Types: full
Name: "matrixorbital"; Description: "Matrix Orbital Displays"; Types: full
Name: "futaba"; Description: "Futaba Displays"; Types: full
Name: "noritake"; Description: "Noritake Displays"; Types: full
Name: "lcdhype"; Description: "LCDHype Driver Library"; Types: full
Name: "vlsyslis"; Description: "VLSystem Displays"; Types: full
Name: "parallel"; Description: "HD44780 Parallel Port Displays"; Types: full
Name: "readme"; Description: "Readme File"; Types: full compact
Name: "sample"; Description: "Samples"; Types: full
Name: "multdev"; Description: "Allow multiple displays"

[Files] 
Source: "simlcd.dll"; DestDir: "{app}"; Components: main; Flags: promptifolder
Source: "lcdrvr1.dll"; DestDir: "{app}"; Components: lcdriver; Flags: promptifolder
Source: "cfontzlcd.dll"; DestDir: "{app}"; Components: crystalfontz; Flags: promptifolder
Source: "molcd.dll"; DestDir: "{app}"; Components: matrixorbital; Flags: promptifolder
Source: "futabavfd.dll"; DestDir: "{app}"; Components: futaba; Flags: promptifolder
Source: "noritakevfd.dll"; DestDir: "{app}"; Components: noritake; Flags: promptifolder
Source: "LcdHype.dll"; DestDir: "{app}"; Components: lcdhype; Flags: promptifolder
Source: "vlsyslis.dll"; DestDir: "{app}"; Components: vlsyslis; Flags: promptifolder
Source: "paralcd.dll"; DestDir: "{app}"; Components: parallel; Flags: promptifolder
Source: "plugins\LCD.dll"; DestDir: "{app}\plugins"; Components: main; Flags: promptifolder
Source: "plugins\LCD\readme.htm"; DestDir: "{app}\plugins\LCD"; Components: readme; Flags: isreadme
Source: "plugins\LCD\license.txt"; DestDir: "{app}\plugins\LCD"; Components: readme
Source: "plugins\LCD\LCDTests.gml"; DestDir: "{app}\plugins\LCD"; Components: sample

[Registry] 
Root: HKLM; Subkey: "SOFTWARE\girder3\SoftPlugins\LCD"; ValueType: string; ValueData: "LCD"; Components: multdev

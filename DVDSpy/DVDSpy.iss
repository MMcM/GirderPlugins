; DVDSpy Inno Setup Script
; $Header$

[Setup] 
AppName=DVDSpy
AppVerName=DVDSpy 1.63
AppID=DVDSpy
AppPublisherURL=http://www.promixis.com/products.php?section=girder
AppSupportURL=http://www.promixis.com/phpBB2/
AppVersion=1.63
DefaultDirName={reg:HKLM\SOFTWARE\girder3,PlugInDir|{pf}\girder\plugins}\..
DirExistsWarning=no 
;LicenseFile=plugins\DVDSpy\license.txt
OutputBaseFilename=DVDSpy
OutputDir=.
SetupIconFile=plugins\DVDSpy\src\dvdspy.ico
SourceDir=..\..

[Types]
Name: "compact"; Description: "Minimal installation"
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "Required Files"; Types: full compact custom; Flags: fixed
Name: "windvd"; Description: "WinDVD"; Types: full
Name: "powerdvd"; Description: "PowerDVD"; Types: full
Name: "adtv"; Description: "accessDTV"; Types: full
Name: "zp"; Description: "ZoomPlayer"; Types: full
Name: "tt"; Description: "TheaterTek"; Types: full
Name: "cineplayer"; Description: "CinePlayer"; Types: full
Name: "ati"; Description: "ATI Player"; Types: full
Name: "wmp"; Description: "Windows Media Player"; Types: full
Name: "real"; Description: "RealPlayer"; Types: full
Name: "winamp"; Description: "Winamp 2 / 5"; Types: full
Name: "ssf"; Description: "ShowShifter"; Types: full
Name: "bsplay"; Description: "BSPlayer"; Types: full
Name: "powerdivx"; Description: "PowerDivX"; Types: full
Name: "mj"; Description: "Media Jukebox / Center"; Types: full
Name: "radlight"; Description: "RadLight"; Types: full
Name: "eugenes"; Description: "Eugene's Player"; Types: full
Name: "dvdstation"; Description: "DVDStation"; Types: full
Name: "foobar"; Description: "Foobar2000"; Types: full
Name: "weather"; Description: "Weather data download"; Types: full
Name: "dvdtitles"; Description: "DVD titles flat file"
Name: "dvddata"; Description: "DVD titles download"; Types: full compact
Name: "readme"; Description: "Readme File"; Types: full compact
Name: "sample"; Description: "Samples"; Types: full

[Files] 
Source: "plugins\DVDSpy.dll"; DestDir: "{app}\plugins"; Components: main; Flags: promptifolder
Source: "DisplaySpyHook.dll"; DestDir: "{app}"; Components: main; Flags: promptifolder
Source: "MediaSpy.dll"; DestDir: "{app}"; Components: zp tt cineplayer ati ssf bsplay powerdivx radlight; Flags: promptifolder regserver
Source: "plugins\DVDSpy\WinDVD3.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: windvd; Flags: promptifolder
Source: "plugins\DVDSpy\WinDVD4.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: windvd; Flags: promptifolder
Source: "plugins\DVDSpy\WinDVD4PR.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: windvd; Flags: promptifolder
Source: "plugins\DVDSpy\WinDVD5.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: windvd; Flags: promptifolder
Source: "plugins\DVDSpy\PowerDVD.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: powerdvd; Flags: promptifolder
Source: "plugins\DVDSpy\ShowShifter.reg"; DestDir: "{app}\plugins\DVDSpy"; Components: ssf; Flags: promptifolder
Source: "WMPSpy.dll"; DestDir: "{app}"; Components: wmp; Flags: promptifolder regserver
Source: "WMPSpy.wmz"; DestDir: "{reg:HKLM\SOFTWARE\Microsoft\MediaPlayer,SkinsDir|{pf}\Windows Media Player\Skins}"; Components: wmp; Flags: promptifolder
Source: "RealSpy.dll"; DestDir: "{cf}\Real\Plugins"; Components: real; Flags: promptifolder
Source: "gen_DVDSpy.dll"; DestDir: "{reg:HKCU\Software\Winamp,|{pf}\Winamp}\Plugins"; Components: winamp; Flags: promptifolder
Source: "ShowShifterSpy.dll"; DestDir: "{pf}\Home Media Networks Limited\ShowShifter"; Components: ssf; Flags: promptifolder regserver
Source: "TVSpy.exe"; DestDir: "{app}"; Components: ssf; Flags: promptifolder
Source: "int_DVDSpy.dll"; DestDir: "{app}"; Components: mj; Flags: promptifolder regserver
Source: "foo_girder_dvdspy.dll"; DestDir: "{reg:HKCU\Software\foobar2000,InstallDir|{pf}\foobar2000}\components"; Components: foobar; Flags: promptifolder
Source: "DVDTitles.csv"; DestDir: "{app}"; Components: dvdtitles; Flags: promptifolder
Source: "DVDData.vbs"; DestDir: "{app}"; Components: dvddata; Flags: promptifolder
Source: "weather.js"; DestDir: "{app}"; Components: weather; Flags: promptifolder
Source: "WeatherSettings.lua"; DestDir: "{app}"; Components: weather; Flags: promptifolder
Source: "plugins\DVDSpy\readme.htm"; DestDir: "{app}\plugins\DVDSpy"; Components: readme; Flags: isreadme
Source: "plugins\DVDSpy\license.txt"; DestDir: "{app}\plugins\DVDSpy"; Components: readme
Source: "DVDChars.lua"; DestDir: "{app}"; Components: sample; Flags: promptifolder
Source: "DVD2LCD2.GML"; DestDir: "{app}"; Components: sample; Flags: promptifolder
Source: "DVD2LCD4.GML"; DestDir: "{app}"; Components: sample; Flags: promptifolder
Source: "DVD2OSD.GML"; DestDir: "{app}"; Components: sample; Flags: promptifolder
Source: "plugins\DVDSpy\DVDInsert.GML"; DestDir: "{app}\plugins\DVDSpy"; Components: sample; Flags: promptifolder

[Dirs]
Name: "DVDData"; Components: dvddata

[Registry] 
Root: HKLM; Subkey: "SOFTWARE\girder3\Plugins"; ValueName: 215; ValueType: dword; ValueData: 1
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy"; Flags: uninsdeletekey
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy"; ValueName: "StartHook"; ValueType: string; ValueData: "False"
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy"; ValueName: "StartHook"; ValueType: string; ValueData: "True"; Components: windvd powerdvd adtv zp tt cineplayer ati ssf bsplay powerdivx radlight eugenes dvdstation
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy"; ValueName: "DVDTitles"; ValueType: string; ValueData: "{app}\DVDTitles.csv"; Components: dvdtitles; Flags: createvalueifdoesntexist
; Events: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.PlayState"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.Status"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.Duration"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.Elapsed"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.URL"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.Title"; ValueType: string; Components: wmp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "WMP.Close"; ValueType: string; Components: wmp
; Events: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Duration"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Elapsed"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.URL"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Title"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Author"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.State"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Buffering"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Contacting"; ValueType: string; Components: real
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Real.Close"; ValueType: string; Components: real
; Events: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Elapsed"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.File"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Length"; ValueData:"of playlist"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Position"; ValueData:"in playlist"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Status"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Title"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Duration"; ValueType: string; Components: winamp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Winamp.Close"; ValueType: string; Components: winamp
; Events: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.State"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.Time"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.OSD"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.OSD.Off"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.Mode"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.DVD.Title"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.DVD.Chapter"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.DVD.Audio"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.DVD.Sub"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.File"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.Position"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.Resolution"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.FrameRate"; ValueType: string; Components: zp
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "ZoomPlayer.LCD.AspectRatio"; ValueType: string; Components: zp
; Events: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.Playlist.Position"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.Playlist.Length"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Filename"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Artist"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Album"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Name"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Custom1"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Custom2"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.Custom3"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.File.FileType"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.Playback.State"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.Playback.Elapsed"; ValueType: string; Components: mj
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "MediaJukebox.Playback.Duration"; ValueType: string; Components: mj
; Events: eugenes
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Eugenes.Name"; ValueType: string; Components: eugenes
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Eugenes.Elapsed"; ValueType: string; Components: eugenes
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Eugenes.Duration"; ValueType: string; Components: eugenes
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Eugenes.TitleNo"; ValueType: string; Components: eugenes
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Eugenes.Chapter"; ValueType: string; Components: eugenes
; Events: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.Volume"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.Mute"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.Mode"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.AudioMode"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.Resolution"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.ColorSpace"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.SourceFormat"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.DestFormat"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.RecordElapsed"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.PlaybackPosition"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.ProgramLength"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.Channel"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.ChannelPSIP"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "accessDTV.SignalMeter"; ValueType: string; Components: adtv
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Foobar.Close"; ValueType: string; Components: foobar
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Foobar.Duration"; ValueType: string; Components: foobar
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Foobar.Elapsed"; ValueType: string; Components: foobar
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Foobar.Status"; ValueType: string; Components: foobar
Root: HKLM; Subkey: "SOFTWARE\girder3\HardPlugins\DVDSpy\Events"; ValueName: "Foobar.Title"; ValueType: string; Components: foobar

[Run]
Filename: "{win}\REGEDIT.EXE"; WorkingDir: "{app}\plugins\DVDSpy"; Parameters: "/s ShowShifter.reg"; Components: ssf
Filename: "{win}\REGEDIT.EXE"; WorkingDir: "{app}\plugins\DVDSpy"; Parameters: "/s PowerDVD.reg"; Components: powerdvd
Filename: "{win}\REGEDIT.EXE"; WorkingDir: "{app}\plugins\DVDSpy"; Parameters: "/s WinDVD5.reg"; Components: windvd

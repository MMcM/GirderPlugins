@echo off
echo This will install and/or register all DLLs.
pause

regedit DVDSpy.reg

regsvr32 MediaSpy.dll

rem Get PATH right and avoid the wrong CONSOLE.DLL.
pushd "..\Home Media Networks Limited\ShowShifter"
regsvr32 ..\..\girder32\ShowShifterSpy.dll
popd

regsvr32 WMPSpy.dll
copy WMPSpy.wmz "..\Windows Media Player\Skins"

copy gen_DVDSpy.dll ..\Winamp\Plugins

regsvr32 int_DVDSpy.dll 

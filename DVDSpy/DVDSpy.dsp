# Microsoft Developer Studio Project File - Name="DVDSpy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DVDSpy - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DVDSpy.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DVDSpy.mak" CFG="DVDSpy - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DVDSpy - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DVDSpy - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DVDSpy - Win32 Trace" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DVDSpy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /I "..\..\include" /I "Hook" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 advapi32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib strmiids.lib winmm.lib DisplaySpyHook.lib /nologo /dll /machine:I386 /out:"../DVDSpy.dll" /libpath:"Release"

!ELSEIF  "$(CFG)" == "DVDSpy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "Hook" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib winspool.lib comdlg32.lib shell32.lib odbc32.lib odbccp32.lib advapi32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib strmiids.lib winmm.lib DisplaySpyHook.lib /nologo /dll /debug /machine:I386 /out:"../DVDSpy.dll" /pdbtype:sept /libpath:"Debug"

!ELSEIF  "$(CFG)" == "DVDSpy - Win32 Trace"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DVDSpy___Win32_Trace"
# PROP BASE Intermediate_Dir "DVDSpy___Win32_Trace"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Trace"
# PROP Intermediate_Dir "Trace"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /MD /W3 /GX /O2 /I "..\..\include" /I "Hook" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /Gz /MD /W3 /GX /O2 /I "..\..\include" /I "Hook" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DVDSpy_EXPORTS" /D "_TRACE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib strmiids.lib DisplaySpyHook.lib /nologo /dll /machine:I386 /out:"../DVDSpy.dll" /libpath:"Release"
# ADD LINK32 advapi32.lib user32.lib gdi32.lib ole32.lib oleaut32.lib uuid.lib strmiids.lib winmm.lib DisplaySpyHook.lib /nologo /dll /machine:I386 /out:"../DVDSpy.dll" /libpath:"Trace"

!ENDIF 

# Begin Target

# Name "DVDSpy - Win32 Release"
# Name "DVDSpy - Win32 Debug"
# Name "DVDSpy - Win32 Trace"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\dvdtitle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\eugenes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\monitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\plugin.def
# End Source File
# Begin Source File

SOURCE=.\src\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui.rc
# End Source File
# Begin Source File

SOURCE=.\src\zplcd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\plugin.h
# End Source File
# Begin Source File

SOURCE=.\src\resource.h
# End Source File
# Begin Source File

SOURCE=.\src\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\src\dvdspy.ico
# End Source File
# Begin Source File

SOURCE=.\src\plugin.ico
# End Source File
# End Group
# End Target
# End Project

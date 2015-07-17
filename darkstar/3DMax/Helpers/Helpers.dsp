# Microsoft Developer Studio Project File - Name="Helpers" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Helpers - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Helpers.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Helpers.mak" CFG="Helpers - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Helpers - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Helpers - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Helpers - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Lib"
# PROP Intermediate_Dir "Obj/RMObj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MD /W3 /GX /O2 /I "inc" /I "C:\Program Files\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MAX_2" /D EXPORT= /D _USERENTRY=__cdecl /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib core.lib mesh.lib geom.lib util.lib rmCore.lib rmML.lib rmTS3.lib rmGFXIO.lib /nologo /subsystem:windows /dll /machine:I386 /out:"C:\Program Files\3DSMax2\Plugins\Helpers.dlo" /libpath:"C:\Program Files\3DSMax2\MaxSDK\Lib" /libpath:"$(PHOENIXLIB)"

!ELSEIF  "$(CFG)" == "Helpers - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Lib"
# PROP Intermediate_Dir "Obj/DMObj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "C:\Program Files\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib core.lib mesh.lib geom.lib util.lib dmCore.lib dmML.lib dmTS3.lib dmGFXIO.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"C:\Program Files\3DSMax2\Plugins\Helpers.DLO" /pdbtype:sept /libpath:"C:\Program Files\3DSMax2\MaxSDK\Lib" /libpath:"$(PHOENIXLIB)"

!ENDIF 

# Begin Target

# Name "Helpers - Win32 Release"
# Name "Helpers - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\Code\Helpers.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\Helpers.def
# End Source File
# Begin Source File

SOURCE=.\Code\Sequence.cpp
# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter "rc"
# Begin Source File

SOURCE=.\Res\Helpers.rc
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Inc\Helpers.h
# End Source File
# Begin Source File

SOURCE=.\Inc\Resource.h
# End Source File
# End Group
# End Target
# End Project

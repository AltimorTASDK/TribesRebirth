# Microsoft Developer Studio Project File - Name="dtsexp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dtsexp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pb3dexp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pb3dexp.mak" CFG="dtsexp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dtsexp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dtsexp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dtsexp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "d:\darkstar\develop\core\classio\inc" /I "c:\3dsmax\MAXSDK\INCLUDE" /I "c:\phenx\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "MAXSDK" /D "SDUMP" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB dmcore.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"LIBC.lib"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /Zi /Od /I ".\inc" /I "c:\3dsmax\MAXSDK\INCLUDE" /D "_DEBUG" /D "DEBUG" /D "DUMP" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "MAXSDK" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB /nologo /subsystem:windows /dll /pdb:none /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"c:\3dsmax\plugins\pb3dexp.dlo"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "dtsexp - Win32 Release"
# Name "dtsexp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\..\..\3dsmax\maxsdk\lib\Core.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\3dsmax\maxsdk\lib\Geom.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\3dsmax\maxsdk\lib\Mesh.lib
# End Source File
# Begin Source File

SOURCE=.\code\pb3dexp.cpp
# End Source File
# Begin Source File

SOURCE=.\pb3dexp.def
# End Source File
# Begin Source File

SOURCE=.\res\pb3dexp.rc
# End Source File
# Begin Source File

SOURCE=.\code\SceneEnum.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\3dsmax\maxsdk\lib\Util.lib
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\inc\pb3dexp.h
# End Source File
# Begin Source File

SOURCE=.\inc\SceneEnum.h
# End Source File
# End Group
# End Target
# End Project

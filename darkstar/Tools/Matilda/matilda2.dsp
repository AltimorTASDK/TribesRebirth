# Microsoft Developer Studio Project File - Name="matilda2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=matilda2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "matilda2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "matilda2.mak" CFG="matilda2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "matilda2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Tools/Matilda", BQBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Exe"
# PROP Intermediate_Dir ".\Obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Zp4 /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\inc" /I ".\res" /I "$(PHOENIXINC)" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "M_BACK" /D "STRICT" /D "EXPORT" /D "_MBCS" /D _USERENTRY=__cdecl /D "_AFXDLL" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i ".\inc res" /d "_DEBUG" /d "MSVC" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 comctl32.lib winmm.lib wsock32.lib rpcrt4.lib dmMl.lib dmcore.lib dmts3.lib dmgfx.lib dmgfxio.lib dmgfx.lib dmitr.lib dmgrd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /libpath:"d:\darkstar\lib\\"
# SUBTRACT LINK32 /verbose /map /nodefaultlib
# Begin Target

# Name "matilda2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Code\ChangeLighting.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\colorPreviewFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\commonProps.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\copyDetailDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\detailLevelsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\gfxPreviewWind.cpp
# ADD CPP /Od
# End Source File
# Begin Source File

SOURCE=.\Code\matilda2.cpp
# End Source File
# Begin Source File

SOURCE=.\res\matilda2.rc
# ADD BASE RSC /l 0x409 /i "res"
# SUBTRACT BASE RSC /i ".\inc res"
# ADD RSC /l 0x409 /i "res" /i ".\res"
# SUBTRACT RSC /i ".\inc res"
# End Source File
# Begin Source File

SOURCE=.\Code\matilda2Dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\Mledit.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\NullEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\palEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\palHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\palSelListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\RGBEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\run.bat
# End Source File
# Begin Source File

SOURCE=.\Code\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\Code\TEXEditDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\inc\ChangeLighting.h
# End Source File
# Begin Source File

SOURCE=.\inc\colorPreviewFrame.h
# End Source File
# Begin Source File

SOURCE=.\inc\commonProps.h
# End Source File
# Begin Source File

SOURCE=.\inc\copyDetailDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\detailLevelsDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\gfxPreviewWind.h
# End Source File
# Begin Source File

SOURCE=.\inc\matilda2.h
# End Source File
# Begin Source File

SOURCE=.\inc\matilda2Dlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\matsuprt.h
# End Source File
# Begin Source File

SOURCE=.\inc\mledit.h
# End Source File
# Begin Source File

SOURCE=.\inc\NullEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\palEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\palHelp.h
# End Source File
# Begin Source File

SOURCE=.\inc\palSelListBox.h
# End Source File
# Begin Source File

SOURCE=.\inc\RGBEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\inc\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\inc\TEXEditDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\matilda2.ico
# End Source File
# Begin Source File

SOURCE=.\res\matilda2.rc2
# End Source File
# End Group
# End Target
# End Project

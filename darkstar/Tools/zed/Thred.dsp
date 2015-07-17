# Microsoft Developer Studio Project File - Name="THRED" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=THRED - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Thred.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Thred.mak" CFG="THRED - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "THRED - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "THRED - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "THRED - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /Gd /Zp4 /MD /W3 /GR /GX /Zi /O2 /Oy- /I "." /I "d:\darkstar\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC" /D "_MBCS" /D "ZED" /D "_AFXDLL" /Fr /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 d:\darkstar\lib\rmconsole.lib d:\darkstar\lib\rmcore.lib d:\darkstar\lib\rmgfx.lib d:\darkstar\lib\rmgfxio.lib d:\darkstar\lib\rmml.lib d:\darkstar\lib\rmts3.lib d:\darkstar\lib\rmitr.lib d:\darkstar\lib\rmzeditr.lib /nologo /subsystem:windows /debug /machine:I386 /out:".\Release\Zed.exe"
# SUBTRACT LINK32 /incremental:yes /map /nodefaultlib

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /I "d:\darkstar\inc" /D "_ZED_BUILD" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG" /D "WIN" /D "MSVC" /D "_MBCS" /D "_AFXDLL" /D "ZED" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 d:\darkstar\lib\dmconsole.lib d:\darkstar\lib\dmcore.lib d:\darkstar\lib\dmgfx.lib d:\darkstar\lib\dmgfxio.lib d:\darkstar\lib\dmml.lib d:\darkstar\lib\dmts3.lib d:\darkstar\lib\dmitr.lib d:\darkstar\lib\dmzeditr.lib /nologo /subsystem:windows /debug /machine:I386 /out:".\Debug\Zed.exe"
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "THRED - Win32 Release"
# Name "THRED - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\3DMWorld.cpp
# End Source File
# Begin Source File

SOURCE=.\AsgnText.cpp
# End Source File
# Begin Source File

SOURCE=.\AsPalMat.cpp
# End Source File
# Begin Source File

SOURCE=.\BOrdrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\BrushAttributesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BrushGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\BrushGroupDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\BspManagerDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\bspnode.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ConstructiveBsp.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateBoxDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateCylDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateSpheroidDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateStaircaseDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CreateTriDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DetailSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\EditSun.cpp
# End Source File
# Begin Source File

SOURCE=.\EntitiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Entity.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalEnvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalSettingsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\GridSizeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupView.cpp
# End Source File
# Begin Source File

SOURCE=.\ITRBuildOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\ITRLightOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\itrmatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\ITRShapeOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\Light.cpp
# End Source File
# Begin Source File

SOURCE=.\LightBlankProp.cpp
# End Source File
# Begin Source File

SOURCE=.\LightDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LightEmitterProp.cpp
# End Source File
# Begin Source File

SOURCE=.\LightProp.cpp
# End Source File
# Begin Source File

SOURCE=.\LightPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\LightStateProp.cpp
# End Source File
# Begin Source File

SOURCE=.\LightView.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Matcanvs.cpp
# End Source File
# Begin Source File

SOURCE=.\MaterialProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\MatVuDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewFDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectView.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\SClrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectClassname.cpp
# End Source File
# Begin Source File

SOURCE=.\Splitter.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TextureView.cpp
# End Source File
# Begin Source File

SOURCE=.\THRED.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\THRED.hpj

!IF  "$(CFG)" == "THRED - Win32 Release"

# Begin Custom Build - Making help file...
OutDir=.\Release
ProjDir=.
TargetName=Zed
InputPath=.\hlp\THRED.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(ProjDir)\makehelp.bat"

# End Custom Build

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\THRED.rc
# End Source File
# Begin Source File

SOURCE=.\ThredBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\THREDDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ThredParser.cpp
# End Source File
# Begin Source File

SOURCE=.\THREDView.cpp
# End Source File
# Begin Source File

SOURCE=.\TransformMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\TSDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\VDescDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\3DMWorld.h
# End Source File
# Begin Source File

SOURCE=.\asgntext.h
# End Source File
# Begin Source File

SOURCE=.\AsPalMat.h
# End Source File
# Begin Source File

SOURCE=.\BOrdrDlg.h
# End Source File
# Begin Source File

SOURCE=.\BrushAttributesDialog.h
# End Source File
# Begin Source File

SOURCE=.\BrushGroup.h
# End Source File
# Begin Source File

SOURCE=.\BrushGroupDialog.h
# End Source File
# Begin Source File

SOURCE=.\BspManagerDialog.h
# End Source File
# Begin Source File

SOURCE=.\bspnode.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ConstructiveBsp.h
# End Source File
# Begin Source File

SOURCE=.\CreateBoxDialog.h
# End Source File
# Begin Source File

SOURCE=.\CreateCylDialog.h
# End Source File
# Begin Source File

SOURCE=.\CreateSpheroidDialog.h
# End Source File
# Begin Source File

SOURCE=.\CreateStaircaseDialog.h
# End Source File
# Begin Source File

SOURCE=.\CreateTriDialog.h
# End Source File
# Begin Source File

SOURCE=.\DetailSettings.h
# End Source File
# Begin Source File

SOURCE=.\EditSun.h
# End Source File
# Begin Source File

SOURCE=.\EntitiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\Entity.h
# End Source File
# Begin Source File

SOURCE=.\GlobalEnvironment.h
# End Source File
# Begin Source File

SOURCE=.\GlobalSettingsDialog.h
# End Source File
# Begin Source File

SOURCE=.\Grid.h
# End Source File
# Begin Source File

SOURCE=.\GridSizeDialog.h
# End Source File
# Begin Source File

SOURCE=.\GroupView.h
# End Source File
# Begin Source File

SOURCE=.\ITRBuildOpt.h
# End Source File
# Begin Source File

SOURCE=.\ITRLightOpt.h
# End Source File
# Begin Source File

SOURCE=.\ITRShapeOpt.h
# End Source File
# Begin Source File

SOURCE=.\Light.h
# End Source File
# Begin Source File

SOURCE=.\LightBlankProp.h
# End Source File
# Begin Source File

SOURCE=.\LightDlg.h
# End Source File
# Begin Source File

SOURCE=.\LightEmitterProp.h
# End Source File
# Begin Source File

SOURCE=.\LightProp.h
# End Source File
# Begin Source File

SOURCE=.\LightPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\LightStateProp.h
# End Source File
# Begin Source File

SOURCE=.\LightView.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Matcanvs.h
# End Source File
# Begin Source File

SOURCE=.\MaterialProperties.h
# End Source File
# Begin Source File

SOURCE=.\matvudlg.h
# End Source File
# Begin Source File

SOURCE=.\newfdlg.h
# End Source File
# Begin Source File

SOURCE=.\ObjectView.h
# End Source File
# Begin Source File

SOURCE=.\P_edge.h
# End Source File
# Begin Source File

SOURCE=.\ProgressBar.h
# End Source File
# Begin Source File

SOURCE=.\RenderCamera.h
# End Source File
# Begin Source File

SOURCE=.\resource.hm
# End Source File
# Begin Source File

SOURCE=.\SClrDlg.h
# End Source File
# Begin Source File

SOURCE=.\SelectClassname.h
# End Source File
# Begin Source File

SOURCE=.\Splitter.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TextureView.h
# End Source File
# Begin Source File

SOURCE=.\THRED.h
# End Source File
# Begin Source File

SOURCE=.\ThredBrush.h
# End Source File
# Begin Source File

SOURCE=.\THREDDoc.h
# End Source File
# Begin Source File

SOURCE=.\ThredParser.h
# End Source File
# Begin Source File

SOURCE=.\thredprimitives.h
# End Source File
# Begin Source File

SOURCE=.\THREDView.h
# End Source File
# Begin Source File

SOURCE=.\Tplane.h
# End Source File
# Begin Source File

SOURCE=.\Tpoly.h
# End Source File
# Begin Source File

SOURCE=.\TransformMatrix.h
# End Source File
# Begin Source File

SOURCE=.\TSDialog.h
# End Source File
# Begin Source File

SOURCE=.\VDescDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\cylinder.bmp
# End Source File
# Begin Source File

SOURCE=.\res\darrow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\detail.bmp
# End Source File
# Begin Source File

SOURCE=.\res\detail2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hollowbo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hollowsp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hollowtr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\light.bmp
# End Source File
# Begin Source File

SOURCE=.\res\light2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\lightbul.bmp
# End Source File
# Begin Source File

SOURCE=.\res\solidbox.bmp
# End Source File
# Begin Source File

SOURCE=.\res\solidsph.bmp
# End Source File
# Begin Source File

SOURCE=.\res\solidtri.bmp
# End Source File
# Begin Source File

SOURCE=.\res\state.bmp
# End Source File
# Begin Source File

SOURCE=.\res\THRED.ico
# End Source File
# Begin Source File

SOURCE=.\res\THRED.rc2
# End Source File
# Begin Source File

SOURCE=.\res\thred_pa.bin
# End Source File
# Begin Source File

SOURCE=.\res\THREDDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# End Target
# End Project
# Section THRED : {37C270D0-4771-11CF-944B-0000E8C4CFB1}
# 	0:8:Splash.h:C:\Thred\Splash.h
# 	0:10:Splash.cpp:C:\Thred\Splash.cpp
# 	1:10:IDB_SPLASH:105
# 	2:10:ResHdrName:resource.h
# 	2:11:ProjHdrName:stdafx.h
# 	2:10:WrapperDef:_SPLASH_SCRN_
# 	2:12:SplClassName:CSplashWnd
# 	2:21:SplashScreenInsertKey:4.0
# 	2:10:HeaderName:Splash.h
# 	2:10:ImplemName:Splash.cpp
# 	2:7:BmpID16:IDB_SPLASH
# End Section

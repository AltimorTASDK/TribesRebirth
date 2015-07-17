# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=THRED - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to THRED - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "THRED - Win32 Release" && "$(CFG)" != "THRED - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Thred.mak" CFG="THRED - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "THRED - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "THRED - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "THRED - Win32 Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "THRED - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Zed.exe" "$(OUTDIR)\Thred.bsc" "$(OUTDIR)\Zed.hlp"

CLEAN : 
	-@erase "$(INTDIR)\3DMWorld.obj"
	-@erase "$(INTDIR)\3DMWorld.sbr"
	-@erase "$(INTDIR)\AsgnText.obj"
	-@erase "$(INTDIR)\AsgnText.sbr"
	-@erase "$(INTDIR)\AsPalMat.obj"
	-@erase "$(INTDIR)\AsPalMat.sbr"
	-@erase "$(INTDIR)\BOrdrDlg.obj"
	-@erase "$(INTDIR)\BOrdrDlg.sbr"
	-@erase "$(INTDIR)\BrushAttributesDialog.obj"
	-@erase "$(INTDIR)\BrushAttributesDialog.sbr"
	-@erase "$(INTDIR)\BrushGroup.obj"
	-@erase "$(INTDIR)\BrushGroup.sbr"
	-@erase "$(INTDIR)\BrushGroupDialog.obj"
	-@erase "$(INTDIR)\BrushGroupDialog.sbr"
	-@erase "$(INTDIR)\BspManagerDialog.obj"
	-@erase "$(INTDIR)\BspManagerDialog.sbr"
	-@erase "$(INTDIR)\bspnode.obj"
	-@erase "$(INTDIR)\bspnode.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\ConstructiveBsp.obj"
	-@erase "$(INTDIR)\ConstructiveBsp.sbr"
	-@erase "$(INTDIR)\CreateBoxDialog.obj"
	-@erase "$(INTDIR)\CreateBoxDialog.sbr"
	-@erase "$(INTDIR)\CreateCylDialog.obj"
	-@erase "$(INTDIR)\CreateCylDialog.sbr"
	-@erase "$(INTDIR)\CreateSpheroidDialog.obj"
	-@erase "$(INTDIR)\CreateSpheroidDialog.sbr"
	-@erase "$(INTDIR)\CreateStaircaseDialog.obj"
	-@erase "$(INTDIR)\CreateStaircaseDialog.sbr"
	-@erase "$(INTDIR)\CreateTriDialog.obj"
	-@erase "$(INTDIR)\CreateTriDialog.sbr"
	-@erase "$(INTDIR)\EntitiesDialog.obj"
	-@erase "$(INTDIR)\EntitiesDialog.sbr"
	-@erase "$(INTDIR)\Entity.obj"
	-@erase "$(INTDIR)\Entity.sbr"
	-@erase "$(INTDIR)\GlobalEnvironment.obj"
	-@erase "$(INTDIR)\GlobalEnvironment.sbr"
	-@erase "$(INTDIR)\GlobalSettingsDialog.obj"
	-@erase "$(INTDIR)\GlobalSettingsDialog.sbr"
	-@erase "$(INTDIR)\GridSizeDialog.obj"
	-@erase "$(INTDIR)\GridSizeDialog.sbr"
	-@erase "$(INTDIR)\LightDlg.obj"
	-@erase "$(INTDIR)\LightDlg.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\Matcanvs.obj"
	-@erase "$(INTDIR)\Matcanvs.sbr"
	-@erase "$(INTDIR)\MatVuDlg.obj"
	-@erase "$(INTDIR)\MatVuDlg.sbr"
	-@erase "$(INTDIR)\NewFDlg.obj"
	-@erase "$(INTDIR)\NewFDlg.sbr"
	-@erase "$(INTDIR)\P_edge.obj"
	-@erase "$(INTDIR)\P_edge.sbr"
	-@erase "$(INTDIR)\RenderCamera.obj"
	-@erase "$(INTDIR)\RenderCamera.sbr"
	-@erase "$(INTDIR)\SClrDlg.obj"
	-@erase "$(INTDIR)\SClrDlg.sbr"
	-@erase "$(INTDIR)\SelectClassname.obj"
	-@erase "$(INTDIR)\SelectClassname.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\THRED.obj"
	-@erase "$(INTDIR)\Thred.pch"
	-@erase "$(INTDIR)\THRED.res"
	-@erase "$(INTDIR)\THRED.sbr"
	-@erase "$(INTDIR)\ThredBrush.obj"
	-@erase "$(INTDIR)\ThredBrush.sbr"
	-@erase "$(INTDIR)\THREDDoc.obj"
	-@erase "$(INTDIR)\THREDDoc.sbr"
	-@erase "$(INTDIR)\ThredParser.obj"
	-@erase "$(INTDIR)\ThredParser.sbr"
	-@erase "$(INTDIR)\THREDView.obj"
	-@erase "$(INTDIR)\THREDView.sbr"
	-@erase "$(INTDIR)\TransformMatrix.obj"
	-@erase "$(INTDIR)\TransformMatrix.sbr"
	-@erase "$(INTDIR)\ts_PointArray.obj"
	-@erase "$(INTDIR)\ts_PointArray.sbr"
	-@erase "$(INTDIR)\TSDialog.obj"
	-@erase "$(INTDIR)\TSDialog.sbr"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\VDescDlg.obj"
	-@erase "$(INTDIR)\VDescDlg.sbr"
	-@erase "$(INTDIR)\Zed.hlp"
	-@erase "$(OUTDIR)\Thred.bsc"
	-@erase "$(OUTDIR)\Zed.exe"
	-@erase "$(OUTDIR)\Zed.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /Gr /Zp4 /MD /W3 /GX /Zi /O2 /Oy- /I "." /I "L:\Dynamix\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC" /D "_AFXDLL" /D "_MBCS" /Fr /Yu"stdafx.h" /c
CPP_PROJ=/nologo /G5 /Gr /Zp4 /MD /W3 /GX /Zi /O2 /Oy- /I "." /I\
 "L:\Dynamix\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC"\
 /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/" /Fp"$(INTDIR)/Thred.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THRED.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Thred.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\3DMWorld.sbr" \
	"$(INTDIR)\AsgnText.sbr" \
	"$(INTDIR)\AsPalMat.sbr" \
	"$(INTDIR)\BOrdrDlg.sbr" \
	"$(INTDIR)\BrushAttributesDialog.sbr" \
	"$(INTDIR)\BrushGroup.sbr" \
	"$(INTDIR)\BrushGroupDialog.sbr" \
	"$(INTDIR)\BspManagerDialog.sbr" \
	"$(INTDIR)\bspnode.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\ConstructiveBsp.sbr" \
	"$(INTDIR)\CreateBoxDialog.sbr" \
	"$(INTDIR)\CreateCylDialog.sbr" \
	"$(INTDIR)\CreateSpheroidDialog.sbr" \
	"$(INTDIR)\CreateStaircaseDialog.sbr" \
	"$(INTDIR)\CreateTriDialog.sbr" \
	"$(INTDIR)\EntitiesDialog.sbr" \
	"$(INTDIR)\Entity.sbr" \
	"$(INTDIR)\GlobalEnvironment.sbr" \
	"$(INTDIR)\GlobalSettingsDialog.sbr" \
	"$(INTDIR)\GridSizeDialog.sbr" \
	"$(INTDIR)\LightDlg.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\Matcanvs.sbr" \
	"$(INTDIR)\MatVuDlg.sbr" \
	"$(INTDIR)\NewFDlg.sbr" \
	"$(INTDIR)\P_edge.sbr" \
	"$(INTDIR)\RenderCamera.sbr" \
	"$(INTDIR)\SClrDlg.sbr" \
	"$(INTDIR)\SelectClassname.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\THRED.sbr" \
	"$(INTDIR)\ThredBrush.sbr" \
	"$(INTDIR)\THREDDoc.sbr" \
	"$(INTDIR)\ThredParser.sbr" \
	"$(INTDIR)\THREDView.sbr" \
	"$(INTDIR)\TransformMatrix.sbr" \
	"$(INTDIR)\ts_PointArray.sbr" \
	"$(INTDIR)\TSDialog.sbr" \
	"$(INTDIR)\VDescDlg.sbr"

"$(OUTDIR)\Thred.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 L:\Dynamix\lib\rmcore.lib L:\Dynamix\lib\rmgfx.lib L:\Dynamix\lib\rmgfxio.lib L:\Dynamix\lib\rmml.lib L:\Dynamix\lib\rmts3.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Release/Zed.exe"
# SUBTRACT LINK32 /incremental:yes
LINK32_FLAGS=L:\Dynamix\lib\rmcore.lib L:\Dynamix\lib\rmgfx.lib\
 L:\Dynamix\lib\rmgfxio.lib L:\Dynamix\lib\rmml.lib L:\Dynamix\lib\rmts3.lib\
 /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/Zed.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/Zed.exe" 
LINK32_OBJS= \
	"$(INTDIR)\3DMWorld.obj" \
	"$(INTDIR)\AsgnText.obj" \
	"$(INTDIR)\AsPalMat.obj" \
	"$(INTDIR)\BOrdrDlg.obj" \
	"$(INTDIR)\BrushAttributesDialog.obj" \
	"$(INTDIR)\BrushGroup.obj" \
	"$(INTDIR)\BrushGroupDialog.obj" \
	"$(INTDIR)\BspManagerDialog.obj" \
	"$(INTDIR)\bspnode.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\ConstructiveBsp.obj" \
	"$(INTDIR)\CreateBoxDialog.obj" \
	"$(INTDIR)\CreateCylDialog.obj" \
	"$(INTDIR)\CreateSpheroidDialog.obj" \
	"$(INTDIR)\CreateStaircaseDialog.obj" \
	"$(INTDIR)\CreateTriDialog.obj" \
	"$(INTDIR)\EntitiesDialog.obj" \
	"$(INTDIR)\Entity.obj" \
	"$(INTDIR)\GlobalEnvironment.obj" \
	"$(INTDIR)\GlobalSettingsDialog.obj" \
	"$(INTDIR)\GridSizeDialog.obj" \
	"$(INTDIR)\LightDlg.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Matcanvs.obj" \
	"$(INTDIR)\MatVuDlg.obj" \
	"$(INTDIR)\NewFDlg.obj" \
	"$(INTDIR)\P_edge.obj" \
	"$(INTDIR)\RenderCamera.obj" \
	"$(INTDIR)\SClrDlg.obj" \
	"$(INTDIR)\SelectClassname.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\THRED.obj" \
	"$(INTDIR)\THRED.res" \
	"$(INTDIR)\ThredBrush.obj" \
	"$(INTDIR)\THREDDoc.obj" \
	"$(INTDIR)\ThredParser.obj" \
	"$(INTDIR)\THREDView.obj" \
	"$(INTDIR)\TransformMatrix.obj" \
	"$(INTDIR)\ts_PointArray.obj" \
	"$(INTDIR)\TSDialog.obj" \
	"$(INTDIR)\VDescDlg.obj"

"$(OUTDIR)\Zed.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Zed.exe" "$(OUTDIR)\Thred.bsc" "$(OUTDIR)\Zed.hlp"

CLEAN : 
	-@erase "$(INTDIR)\3DMWorld.obj"
	-@erase "$(INTDIR)\3DMWorld.sbr"
	-@erase "$(INTDIR)\AsgnText.obj"
	-@erase "$(INTDIR)\AsgnText.sbr"
	-@erase "$(INTDIR)\AsPalMat.obj"
	-@erase "$(INTDIR)\AsPalMat.sbr"
	-@erase "$(INTDIR)\BOrdrDlg.obj"
	-@erase "$(INTDIR)\BOrdrDlg.sbr"
	-@erase "$(INTDIR)\BrushAttributesDialog.obj"
	-@erase "$(INTDIR)\BrushAttributesDialog.sbr"
	-@erase "$(INTDIR)\BrushGroup.obj"
	-@erase "$(INTDIR)\BrushGroup.sbr"
	-@erase "$(INTDIR)\BrushGroupDialog.obj"
	-@erase "$(INTDIR)\BrushGroupDialog.sbr"
	-@erase "$(INTDIR)\BspManagerDialog.obj"
	-@erase "$(INTDIR)\BspManagerDialog.sbr"
	-@erase "$(INTDIR)\bspnode.obj"
	-@erase "$(INTDIR)\bspnode.sbr"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\ConstructiveBsp.obj"
	-@erase "$(INTDIR)\ConstructiveBsp.sbr"
	-@erase "$(INTDIR)\CreateBoxDialog.obj"
	-@erase "$(INTDIR)\CreateBoxDialog.sbr"
	-@erase "$(INTDIR)\CreateCylDialog.obj"
	-@erase "$(INTDIR)\CreateCylDialog.sbr"
	-@erase "$(INTDIR)\CreateSpheroidDialog.obj"
	-@erase "$(INTDIR)\CreateSpheroidDialog.sbr"
	-@erase "$(INTDIR)\CreateStaircaseDialog.obj"
	-@erase "$(INTDIR)\CreateStaircaseDialog.sbr"
	-@erase "$(INTDIR)\CreateTriDialog.obj"
	-@erase "$(INTDIR)\CreateTriDialog.sbr"
	-@erase "$(INTDIR)\EntitiesDialog.obj"
	-@erase "$(INTDIR)\EntitiesDialog.sbr"
	-@erase "$(INTDIR)\Entity.obj"
	-@erase "$(INTDIR)\Entity.sbr"
	-@erase "$(INTDIR)\GlobalEnvironment.obj"
	-@erase "$(INTDIR)\GlobalEnvironment.sbr"
	-@erase "$(INTDIR)\GlobalSettingsDialog.obj"
	-@erase "$(INTDIR)\GlobalSettingsDialog.sbr"
	-@erase "$(INTDIR)\GridSizeDialog.obj"
	-@erase "$(INTDIR)\GridSizeDialog.sbr"
	-@erase "$(INTDIR)\LightDlg.obj"
	-@erase "$(INTDIR)\LightDlg.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\Matcanvs.obj"
	-@erase "$(INTDIR)\Matcanvs.sbr"
	-@erase "$(INTDIR)\MatVuDlg.obj"
	-@erase "$(INTDIR)\MatVuDlg.sbr"
	-@erase "$(INTDIR)\NewFDlg.obj"
	-@erase "$(INTDIR)\NewFDlg.sbr"
	-@erase "$(INTDIR)\P_edge.obj"
	-@erase "$(INTDIR)\P_edge.sbr"
	-@erase "$(INTDIR)\RenderCamera.obj"
	-@erase "$(INTDIR)\RenderCamera.sbr"
	-@erase "$(INTDIR)\SClrDlg.obj"
	-@erase "$(INTDIR)\SClrDlg.sbr"
	-@erase "$(INTDIR)\SelectClassname.obj"
	-@erase "$(INTDIR)\SelectClassname.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\THRED.obj"
	-@erase "$(INTDIR)\Thred.pch"
	-@erase "$(INTDIR)\THRED.res"
	-@erase "$(INTDIR)\THRED.sbr"
	-@erase "$(INTDIR)\ThredBrush.obj"
	-@erase "$(INTDIR)\ThredBrush.sbr"
	-@erase "$(INTDIR)\THREDDoc.obj"
	-@erase "$(INTDIR)\THREDDoc.sbr"
	-@erase "$(INTDIR)\ThredParser.obj"
	-@erase "$(INTDIR)\ThredParser.sbr"
	-@erase "$(INTDIR)\THREDView.obj"
	-@erase "$(INTDIR)\THREDView.sbr"
	-@erase "$(INTDIR)\TransformMatrix.obj"
	-@erase "$(INTDIR)\TransformMatrix.sbr"
	-@erase "$(INTDIR)\ts_PointArray.obj"
	-@erase "$(INTDIR)\ts_PointArray.sbr"
	-@erase "$(INTDIR)\TSDialog.obj"
	-@erase "$(INTDIR)\TSDialog.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\VDescDlg.obj"
	-@erase "$(INTDIR)\VDescDlg.sbr"
	-@erase "$(INTDIR)\Zed.hlp"
	-@erase "$(OUTDIR)\Thred.bsc"
	-@erase "$(OUTDIR)\Zed.exe"
	-@erase "$(OUTDIR)\Zed.ilk"
	-@erase "$(OUTDIR)\Zed.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GX /Zi /Od /I "." /I "L:\Dynamix\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG" /D "WIN" /D "MSVC" /D "_AFXDLL" /D COREAPI=__cdecl /D "_MBCS" /Fr /Yu"stdafx.h" /c
CPP_PROJ=/nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GX /Zi /Od /I "." /I\
 "L:\Dynamix\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG"\
 /D "WIN" /D "MSVC" /D "_AFXDLL" /D COREAPI=__cdecl /D "_MBCS" /Fr"$(INTDIR)/"\
 /Fp"$(INTDIR)/Thred.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/THRED.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Thred.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\3DMWorld.sbr" \
	"$(INTDIR)\AsgnText.sbr" \
	"$(INTDIR)\AsPalMat.sbr" \
	"$(INTDIR)\BOrdrDlg.sbr" \
	"$(INTDIR)\BrushAttributesDialog.sbr" \
	"$(INTDIR)\BrushGroup.sbr" \
	"$(INTDIR)\BrushGroupDialog.sbr" \
	"$(INTDIR)\BspManagerDialog.sbr" \
	"$(INTDIR)\bspnode.sbr" \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\ConstructiveBsp.sbr" \
	"$(INTDIR)\CreateBoxDialog.sbr" \
	"$(INTDIR)\CreateCylDialog.sbr" \
	"$(INTDIR)\CreateSpheroidDialog.sbr" \
	"$(INTDIR)\CreateStaircaseDialog.sbr" \
	"$(INTDIR)\CreateTriDialog.sbr" \
	"$(INTDIR)\EntitiesDialog.sbr" \
	"$(INTDIR)\Entity.sbr" \
	"$(INTDIR)\GlobalEnvironment.sbr" \
	"$(INTDIR)\GlobalSettingsDialog.sbr" \
	"$(INTDIR)\GridSizeDialog.sbr" \
	"$(INTDIR)\LightDlg.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\Matcanvs.sbr" \
	"$(INTDIR)\MatVuDlg.sbr" \
	"$(INTDIR)\NewFDlg.sbr" \
	"$(INTDIR)\P_edge.sbr" \
	"$(INTDIR)\RenderCamera.sbr" \
	"$(INTDIR)\SClrDlg.sbr" \
	"$(INTDIR)\SelectClassname.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\THRED.sbr" \
	"$(INTDIR)\ThredBrush.sbr" \
	"$(INTDIR)\THREDDoc.sbr" \
	"$(INTDIR)\ThredParser.sbr" \
	"$(INTDIR)\THREDView.sbr" \
	"$(INTDIR)\TransformMatrix.sbr" \
	"$(INTDIR)\ts_PointArray.sbr" \
	"$(INTDIR)\TSDialog.sbr" \
	"$(INTDIR)\VDescDlg.sbr"

"$(OUTDIR)\Thred.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 L:\Dynamix\lib\dmcore.lib L:\Dynamix\lib\dmgfx.lib L:\Dynamix\lib\dmgfxio.lib L:\Dynamix\lib\dmml.lib L:\Dynamix\lib\dmts3.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/Zed.exe"
# SUBTRACT LINK32 /incremental:no /nodefaultlib
LINK32_FLAGS=L:\Dynamix\lib\dmcore.lib L:\Dynamix\lib\dmgfx.lib\
 L:\Dynamix\lib\dmgfxio.lib L:\Dynamix\lib\dmml.lib L:\Dynamix\lib\dmts3.lib\
 /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/Zed.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/Zed.exe" 
LINK32_OBJS= \
	"$(INTDIR)\3DMWorld.obj" \
	"$(INTDIR)\AsgnText.obj" \
	"$(INTDIR)\AsPalMat.obj" \
	"$(INTDIR)\BOrdrDlg.obj" \
	"$(INTDIR)\BrushAttributesDialog.obj" \
	"$(INTDIR)\BrushGroup.obj" \
	"$(INTDIR)\BrushGroupDialog.obj" \
	"$(INTDIR)\BspManagerDialog.obj" \
	"$(INTDIR)\bspnode.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\ConstructiveBsp.obj" \
	"$(INTDIR)\CreateBoxDialog.obj" \
	"$(INTDIR)\CreateCylDialog.obj" \
	"$(INTDIR)\CreateSpheroidDialog.obj" \
	"$(INTDIR)\CreateStaircaseDialog.obj" \
	"$(INTDIR)\CreateTriDialog.obj" \
	"$(INTDIR)\EntitiesDialog.obj" \
	"$(INTDIR)\Entity.obj" \
	"$(INTDIR)\GlobalEnvironment.obj" \
	"$(INTDIR)\GlobalSettingsDialog.obj" \
	"$(INTDIR)\GridSizeDialog.obj" \
	"$(INTDIR)\LightDlg.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Matcanvs.obj" \
	"$(INTDIR)\MatVuDlg.obj" \
	"$(INTDIR)\NewFDlg.obj" \
	"$(INTDIR)\P_edge.obj" \
	"$(INTDIR)\RenderCamera.obj" \
	"$(INTDIR)\SClrDlg.obj" \
	"$(INTDIR)\SelectClassname.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\THRED.obj" \
	"$(INTDIR)\THRED.res" \
	"$(INTDIR)\ThredBrush.obj" \
	"$(INTDIR)\THREDDoc.obj" \
	"$(INTDIR)\ThredParser.obj" \
	"$(INTDIR)\THREDView.obj" \
	"$(INTDIR)\TransformMatrix.obj" \
	"$(INTDIR)\ts_PointArray.obj" \
	"$(INTDIR)\TSDialog.obj" \
	"$(INTDIR)\VDescDlg.obj"

"$(OUTDIR)\Zed.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "THRED - Win32 Release"
# Name "THRED - Win32 Debug"

!IF  "$(CFG)" == "THRED - Win32 Release"

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\THRED.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_THRED=\
	".\ChildFrm.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THRED.obj" : $(SOURCE) $(DEP_CPP_THRED) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THRED.sbr" : $(SOURCE) $(DEP_CPP_THRED) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_THRED=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ChildFrm.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THRED.obj" : $(SOURCE) $(DEP_CPP_THRED) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THRED.sbr" : $(SOURCE) $(DEP_CPP_THRED) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "THRED - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /Gr /Zp4 /MD /W3 /GX /Zi /O2 /Oy- /I "." /I\
 "L:\Dynamix\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC"\
 /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/" /Fp"$(INTDIR)/Thred.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thred.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GX /Zi /Od /I "." /I\
 "L:\Dynamix\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG"\
 /D "WIN" /D "MSVC" /D "_AFXDLL" /D COREAPI=__cdecl /D "_MBCS" /Fr"$(INTDIR)/"\
 /Fp"$(INTDIR)/Thred.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Thred.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_MAINF=\
	".\ChildFrm.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\Matcanvs.h"\
	".\matvudlg.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_MAINF=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ChildFrm.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\Matcanvs.h"\
	".\matvudlg.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChildFrm.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CHILD=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ChildFrm.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THREDDoc.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_THREDD=\
	".\AsPalMat.h"\
	".\BOrdrDlg.h"\
	".\BrushGroup.h"\
	".\BrushGroupDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\EntitiesDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\GridSizeDialog.h"\
	".\LightDlg.h"\
	".\MainFrm.h"\
	".\newfdlg.h"\
	".\RenderCamera.h"\
	".\SClrDlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\TSDialog.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THREDDoc.obj" : $(SOURCE) $(DEP_CPP_THREDD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THREDDoc.sbr" : $(SOURCE) $(DEP_CPP_THREDD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_THREDD=\
	".\AsPalMat.h"\
	".\BOrdrDlg.h"\
	".\BrushGroup.h"\
	".\BrushGroupDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\EntitiesDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\GridSizeDialog.h"\
	".\LightDlg.h"\
	".\MainFrm.h"\
	".\Matcanvs.h"\
	".\newfdlg.h"\
	".\RenderCamera.h"\
	".\SClrDlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\TSDialog.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THREDDoc.obj" : $(SOURCE) $(DEP_CPP_THREDD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THREDDoc.sbr" : $(SOURCE) $(DEP_CPP_THREDD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THREDView.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_THREDV=\
	".\asgntext.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\fn_all.h"\
	"L:\Dynamix\inc\fn_table.h"\
	"L:\Dynamix\inc\g_cds.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THREDView.obj" : $(SOURCE) $(DEP_CPP_THREDV) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THREDView.sbr" : $(SOURCE) $(DEP_CPP_THREDV) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_THREDV=\
	".\asgntext.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\THREDView.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\fn_all.h"\
	"L:\Dynamix\inc\fn_table.h"\
	"L:\Dynamix\inc\g_cds.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\THREDView.obj" : $(SOURCE) $(DEP_CPP_THREDV) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\THREDView.sbr" : $(SOURCE) $(DEP_CPP_THREDV) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\THRED.rc
DEP_RSC_THRED_=\
	".\res\bitmap1.bmp"\
	".\res\cursor1.cur"\
	".\res\cylinder.bmp"\
	".\res\darrow.bmp"\
	".\res\hollowbo.bmp"\
	".\res\hollowsp.bmp"\
	".\res\hollowtr.bmp"\
	".\res\lightbul.bmp"\
	".\res\solidbox.bmp"\
	".\res\solidsph.bmp"\
	".\res\solidtri.bmp"\
	".\res\THRED.ico"\
	".\res\THRED.rc2"\
	".\res\thred_pa.bin"\
	".\res\THREDDoc.ico"\
	".\res\Toolbar.bmp"\
	".\res\toolbar1.bmp"\
	".\resource.hm"\
	

"$(INTDIR)\THRED.res" : $(SOURCE) $(DEP_RSC_THRED_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
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

# Begin Custom Build - Making help file...
OutDir=.\Debug
ProjDir=.
TargetName=Zed
InputPath=.\hlp\THRED.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   "$(ProjDir)\makehelp.bat"

# End Custom Build

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BrushAttributesDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BRUSH=\
	".\BrushAttributesDialog.h"\
	".\Matcanvs.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushAttributesDialog.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushAttributesDialog.sbr" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BRUSH=\
	".\BrushAttributesDialog.h"\
	".\Matcanvs.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushAttributesDialog.obj" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushAttributesDialog.sbr" : $(SOURCE) $(DEP_CPP_BRUSH) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GlobalEnvironment.cpp
DEP_CPP_GLOBA=\
	".\GlobalEnvironment.h"\
	".\GlobalSettingsDialog.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\GlobalEnvironment.obj" : $(SOURCE) $(DEP_CPP_GLOBA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\GlobalEnvironment.sbr" : $(SOURCE) $(DEP_CPP_GLOBA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\RenderCamera.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_RENDE=\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\RenderCamera.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\RenderCamera.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_RENDE=\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\RenderCamera.obj" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\RenderCamera.sbr" : $(SOURCE) $(DEP_CPP_RENDE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ThredBrush.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_THREDB=\
	".\BrushAttributesDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ThredBrush.obj" : $(SOURCE) $(DEP_CPP_THREDB) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ThredBrush.sbr" : $(SOURCE) $(DEP_CPP_THREDB) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_THREDB=\
	".\BrushAttributesDialog.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ThredBrush.obj" : $(SOURCE) $(DEP_CPP_THREDB) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ThredBrush.sbr" : $(SOURCE) $(DEP_CPP_THREDB) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TransformMatrix.cpp
DEP_CPP_TRANS=\
	".\GlobalEnvironment.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\TransformMatrix.obj" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\TransformMatrix.sbr" : $(SOURCE) $(DEP_CPP_TRANS) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ConstructiveBsp.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CONST=\
	".\3DMWorld.h"\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\p_txcach.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ConstructiveBsp.obj" : $(SOURCE) $(DEP_CPP_CONST) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ConstructiveBsp.sbr" : $(SOURCE) $(DEP_CPP_CONST) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CONST=\
	".\3DMWorld.h"\
	".\BrushGroup.h"\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\p_txcach.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ConstructiveBsp.obj" : $(SOURCE) $(DEP_CPP_CONST) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ConstructiveBsp.sbr" : $(SOURCE) $(DEP_CPP_CONST) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateBoxDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CREAT=\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateBoxDialog.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateBoxDialog.sbr" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CREAT=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateBoxDialog.obj" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateBoxDialog.sbr" : $(SOURCE) $(DEP_CPP_CREAT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateSpheroidDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CREATE=\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateSpheroidDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateSpheroidDialog.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateSpheroidDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CREATE=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateSpheroidDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateSpheroidDialog.obj" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateSpheroidDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATE) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateCylDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CREATEC=\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateCylDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateCylDialog.obj" : $(SOURCE) $(DEP_CPP_CREATEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateCylDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CREATEC=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateCylDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateCylDialog.obj" : $(SOURCE) $(DEP_CPP_CREATEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateCylDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\3DMWorld.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_3DMWO=\
	".\3DMWorld.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\3DMWorld.obj" : $(SOURCE) $(DEP_CPP_3DMWO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\3DMWorld.sbr" : $(SOURCE) $(DEP_CPP_3DMWO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_3DMWO=\
	".\3DMWorld.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\MainFrm.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\3DMWorld.obj" : $(SOURCE) $(DEP_CPP_3DMWO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\3DMWorld.sbr" : $(SOURCE) $(DEP_CPP_3DMWO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GridSizeDialog.cpp
DEP_CPP_GRIDS=\
	".\Grid.h"\
	".\GridSizeDialog.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\GridSizeDialog.obj" : $(SOURCE) $(DEP_CPP_GRIDS) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\GridSizeDialog.sbr" : $(SOURCE) $(DEP_CPP_GRIDS) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\BspManagerDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BSPMA=\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BspManagerDialog.obj" : $(SOURCE) $(DEP_CPP_BSPMA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BspManagerDialog.sbr" : $(SOURCE) $(DEP_CPP_BSPMA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BSPMA=\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BspManagerDialog.obj" : $(SOURCE) $(DEP_CPP_BSPMA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BspManagerDialog.sbr" : $(SOURCE) $(DEP_CPP_BSPMA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\EntitiesDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_ENTIT=\
	".\EntitiesDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\SelectClassname.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\EntitiesDialog.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\EntitiesDialog.sbr" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_ENTIT=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\EntitiesDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\SelectClassname.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\EntitiesDialog.obj" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\EntitiesDialog.sbr" : $(SOURCE) $(DEP_CPP_ENTIT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Entity.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_ENTITY=\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\Entity.obj" : $(SOURCE) $(DEP_CPP_ENTITY) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\Entity.sbr" : $(SOURCE) $(DEP_CPP_ENTITY) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_ENTITY=\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\Entity.obj" : $(SOURCE) $(DEP_CPP_ENTITY) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\Entity.sbr" : $(SOURCE) $(DEP_CPP_ENTITY) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SelectClassname.cpp
DEP_CPP_SELEC=\
	".\SelectClassname.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\SelectClassname.obj" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\SelectClassname.sbr" : $(SOURCE) $(DEP_CPP_SELEC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\BrushGroup.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BRUSHG=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushGroup.obj" : $(SOURCE) $(DEP_CPP_BRUSHG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushGroup.sbr" : $(SOURCE) $(DEP_CPP_BRUSHG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BRUSHG=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushGroup.obj" : $(SOURCE) $(DEP_CPP_BRUSHG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushGroup.sbr" : $(SOURCE) $(DEP_CPP_BRUSHG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BrushGroupDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BRUSHGR=\
	".\BrushGroup.h"\
	".\BrushGroupDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushGroupDialog.obj" : $(SOURCE) $(DEP_CPP_BRUSHGR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushGroupDialog.sbr" : $(SOURCE) $(DEP_CPP_BRUSHGR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BRUSHGR=\
	".\BrushGroup.h"\
	".\BrushGroupDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BrushGroupDialog.obj" : $(SOURCE) $(DEP_CPP_BRUSHGR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BrushGroupDialog.sbr" : $(SOURCE) $(DEP_CPP_BRUSHGR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ThredParser.cpp
DEP_CPP_THREDP=\
	".\GlobalEnvironment.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\ThredParser.obj" : $(SOURCE) $(DEP_CPP_THREDP) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\ThredParser.sbr" : $(SOURCE) $(DEP_CPP_THREDP) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bspnode.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BSPNO=\
	".\3DMWorld.h"\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\bspnode.obj" : $(SOURCE) $(DEP_CPP_BSPNO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\bspnode.sbr" : $(SOURCE) $(DEP_CPP_BSPNO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BSPNO=\
	".\3DMWorld.h"\
	".\BrushGroup.h"\
	".\BspManagerDialog.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\bspnode.obj" : $(SOURCE) $(DEP_CPP_BSPNO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\bspnode.sbr" : $(SOURCE) $(DEP_CPP_BSPNO) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateStaircaseDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CREATES=\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateStaircaseDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateStaircaseDialog.obj" : $(SOURCE) $(DEP_CPP_CREATES)\
 "$(INTDIR)" "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateStaircaseDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATES)\
 "$(INTDIR)" "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CREATES=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateStaircaseDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateStaircaseDialog.obj" : $(SOURCE) $(DEP_CPP_CREATES)\
 "$(INTDIR)" "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateStaircaseDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATES)\
 "$(INTDIR)" "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GlobalSettingsDialog.cpp
DEP_CPP_GLOBAL=\
	".\GlobalSettingsDialog.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\GlobalSettingsDialog.obj" : $(SOURCE) $(DEP_CPP_GLOBAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\GlobalSettingsDialog.sbr" : $(SOURCE) $(DEP_CPP_GLOBAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\NewFDlg.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_NEWFD=\
	".\newfdlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\NewFDlg.obj" : $(SOURCE) $(DEP_CPP_NEWFD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\NewFDlg.sbr" : $(SOURCE) $(DEP_CPP_NEWFD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_NEWFD=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\newfdlg.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\NewFDlg.obj" : $(SOURCE) $(DEP_CPP_NEWFD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\NewFDlg.sbr" : $(SOURCE) $(DEP_CPP_NEWFD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MatVuDlg.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_MATVU=\
	".\Matcanvs.h"\
	".\matvudlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\MatVuDlg.obj" : $(SOURCE) $(DEP_CPP_MATVU) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\MatVuDlg.sbr" : $(SOURCE) $(DEP_CPP_MATVU) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_MATVU=\
	".\Matcanvs.h"\
	".\matvudlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\MatVuDlg.obj" : $(SOURCE) $(DEP_CPP_MATVU) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\MatVuDlg.sbr" : $(SOURCE) $(DEP_CPP_MATVU) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Matcanvs.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_MATCA=\
	".\Matcanvs.h"\
	".\StdAfx.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\g_cds.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	

"$(INTDIR)\Matcanvs.obj" : $(SOURCE) $(DEP_CPP_MATCA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\Matcanvs.sbr" : $(SOURCE) $(DEP_CPP_MATCA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_MATCA=\
	".\Matcanvs.h"\
	".\StdAfx.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_cds.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\Matcanvs.obj" : $(SOURCE) $(DEP_CPP_MATCA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\Matcanvs.sbr" : $(SOURCE) $(DEP_CPP_MATCA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AsPalMat.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_ASPAL=\
	".\AsPalMat.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\THREDDoc.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\AsPalMat.obj" : $(SOURCE) $(DEP_CPP_ASPAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\AsPalMat.sbr" : $(SOURCE) $(DEP_CPP_ASPAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_ASPAL=\
	".\AsPalMat.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\AsPalMat.obj" : $(SOURCE) $(DEP_CPP_ASPAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\AsPalMat.sbr" : $(SOURCE) $(DEP_CPP_ASPAL) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\AsgnText.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_ASGNT=\
	".\asgntext.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\AsgnText.obj" : $(SOURCE) $(DEP_CPP_ASGNT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\AsgnText.sbr" : $(SOURCE) $(DEP_CPP_ASGNT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_ASGNT=\
	".\asgntext.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\Matcanvs.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_bitmap.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\AsgnText.obj" : $(SOURCE) $(DEP_CPP_ASGNT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\AsgnText.sbr" : $(SOURCE) $(DEP_CPP_ASGNT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CreateTriDialog.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_CREATET=\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateTriDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateTriDialog.obj" : $(SOURCE) $(DEP_CPP_CREATET) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateTriDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATET) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_CREATET=\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateTriDialog.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\CreateTriDialog.obj" : $(SOURCE) $(DEP_CPP_CREATET) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\CreateTriDialog.sbr" : $(SOURCE) $(DEP_CPP_CREATET) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TSDialog.cpp
DEP_CPP_TSDIA=\
	".\StdAfx.h"\
	".\THRED.h"\
	".\TSDialog.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\TSDialog.obj" : $(SOURCE) $(DEP_CPP_TSDIA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\TSDialog.sbr" : $(SOURCE) $(DEP_CPP_TSDIA) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\LightDlg.cpp
DEP_CPP_LIGHT=\
	".\LightDlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\LightDlg.obj" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\LightDlg.sbr" : $(SOURCE) $(DEP_CPP_LIGHT) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ts_PointArray.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_TS_PO=\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

BuildCmds= \
	$(CPP) /nologo /G5 /Gr /Zp4 /MD /W3 /GX /Zi /O2 /Oy- /I "." /I\
 "L:\Dynamix\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC"\
 /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\ts_PointArray.obj" : $(SOURCE) $(DEP_CPP_TS_PO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ts_PointArray.sbr" : $(SOURCE) $(DEP_CPP_TS_PO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_TS_PO=\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

BuildCmds= \
	$(CPP) /nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GX /Zi /Od /I "." /I\
 "L:\Dynamix\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG"\
 /D "WIN" /D "MSVC" /D "_AFXDLL" /D COREAPI=__cdecl /D "_MBCS" /Fr"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\ts_PointArray.obj" : $(SOURCE) $(DEP_CPP_TS_PO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ts_PointArray.sbr" : $(SOURCE) $(DEP_CPP_TS_PO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\SClrDlg.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_SCLRD=\
	".\SClrDlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\SClrDlg.obj" : $(SOURCE) $(DEP_CPP_SCLRD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\SClrDlg.sbr" : $(SOURCE) $(DEP_CPP_SCLRD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_SCLRD=\
	".\Matcanvs.h"\
	".\SClrDlg.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_PointArray.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\SClrDlg.obj" : $(SOURCE) $(DEP_CPP_SCLRD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\SClrDlg.sbr" : $(SOURCE) $(DEP_CPP_SCLRD) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BOrdrDlg.cpp

!IF  "$(CFG)" == "THRED - Win32 Release"

DEP_CPP_BORDR=\
	".\BOrdrDlg.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BOrdrDlg.obj" : $(SOURCE) $(DEP_CPP_BORDR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BOrdrDlg.sbr" : $(SOURCE) $(DEP_CPP_BORDR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

DEP_CPP_BORDR=\
	".\BOrdrDlg.h"\
	".\BrushGroup.h"\
	".\bspnode.h"\
	".\ConstructiveBsp.h"\
	".\CreateBoxDialog.h"\
	".\CreateCylDialog.h"\
	".\CreateSpheroidDialog.h"\
	".\CreateStaircaseDialog.h"\
	".\CreateTriDialog.h"\
	".\Entity.h"\
	".\GlobalEnvironment.h"\
	".\Grid.h"\
	".\RenderCamera.h"\
	".\StdAfx.h"\
	".\THRED.h"\
	".\ThredBrush.h"\
	".\THREDDoc.h"\
	".\ThredParser.h"\
	".\thredprimitives.h"\
	".\Tplane.h"\
	".\Tpoly.h"\
	".\TransformMatrix.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_box.h"\
	"L:\Dynamix\inc\m_dist.h"\
	"L:\Dynamix\inc\m_dot.h"\
	"L:\Dynamix\inc\m_euler.h"\
	"L:\Dynamix\inc\m_lseg.h"\
	"L:\dynamix\inc\M_mat2.h"\
	"L:\Dynamix\inc\m_mat3.h"\
	"L:\Dynamix\inc\m_mul.h"\
	"L:\Dynamix\inc\m_plane.h"\
	"L:\Dynamix\inc\m_plist.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_qsort.h"\
	"L:\Dynamix\inc\m_quat.h"\
	"L:\Dynamix\inc\m_random.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_sphere.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\ml.h"\
	"L:\Dynamix\inc\persist.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\talgorithm.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\ts_camera.h"\
	"L:\Dynamix\inc\ts_light.h"\
	"L:\Dynamix\inc\ts_material.h"\
	"L:\Dynamix\inc\ts_RenderContext.h"\
	"L:\Dynamix\inc\ts_transform.h"\
	"L:\Dynamix\inc\ts_types.h"\
	"L:\Dynamix\inc\ts_vertex.h"\
	"L:\Dynamix\inc\tsorted.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\BOrdrDlg.obj" : $(SOURCE) $(DEP_CPP_BORDR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\BOrdrDlg.sbr" : $(SOURCE) $(DEP_CPP_BORDR) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VDescDlg.cpp
DEP_CPP_VDESC=\
	".\StdAfx.h"\
	".\THRED.h"\
	".\types.h"\
	".\VDescDlg.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\blkstrm.h"\
	"L:\Dynamix\inc\filedefs.h"\
	"L:\Dynamix\inc\filstrm.h"\
	"L:\Dynamix\inc\lzhstrm.h"\
	"L:\Dynamix\inc\memstrm.h"\
	"L:\Dynamix\inc\resManager.h"\
	"L:\Dynamix\inc\rlestrm.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\streams.h"\
	"L:\Dynamix\inc\threadBase.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\Dynamix\inc\volstrm.h"\
	

"$(INTDIR)\VDescDlg.obj" : $(SOURCE) $(DEP_CPP_VDESC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"

"$(INTDIR)\VDescDlg.sbr" : $(SOURCE) $(DEP_CPP_VDESC) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\P_edge.cpp
DEP_CPP_P_EDG=\
	".\P_edge.h"\
	".\types.h"\
	"L:\Dynamix\inc\base.h"\
	"L:\Dynamix\inc\d_defs.h"\
	"L:\dynamix\inc\D_funcs.h"\
	"L:\Dynamix\inc\fn_all.h"\
	"L:\Dynamix\inc\g_contxt.h"\
	"L:\Dynamix\inc\g_pal.h"\
	"L:\dynamix\inc\G_raster.h"\
	"L:\Dynamix\inc\g_surfac.h"\
	"L:\dynamix\inc\G_types.h"\
	"L:\Dynamix\inc\gfxmetrics.h"\
	"L:\Dynamix\inc\m_base.h"\
	"L:\Dynamix\inc\m_point.h"\
	"L:\Dynamix\inc\m_rect.h"\
	"L:\Dynamix\inc\m_trig.h"\
	"L:\Dynamix\inc\p_funcs.h"\
	"L:\Dynamix\inc\p_txcach.h"\
	"L:\Dynamix\inc\rn_manag.h"\
	"L:\Dynamix\inc\streamio.h"\
	"L:\Dynamix\inc\tBitVector.h"\
	"L:\Dynamix\inc\tvector.h"\
	"L:\dynamix\inc\types.h"\
	

!IF  "$(CFG)" == "THRED - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

BuildCmds= \
	$(CPP) /nologo /G5 /Gr /Zp4 /MD /W3 /GX /Zi /O2 /Oy- /I "." /I\
 "L:\Dynamix\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "MSVC"\
 /D "_AFXDLL" /D "_MBCS" /Fr"$(INTDIR)/" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\P_edge.obj" : $(SOURCE) $(DEP_CPP_P_EDG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\P_edge.sbr" : $(SOURCE) $(DEP_CPP_P_EDG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "THRED - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

BuildCmds= \
	$(CPP) /nologo /G5 /Zp4 /MDd /W3 /Gm /Gi /GX /Zi /Od /I "." /I\
 "L:\Dynamix\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GRIDTOOL" /D "DEBUG"\
 /D "WIN" /D "MSVC" /D "_AFXDLL" /D COREAPI=__cdecl /D "_MBCS" /Fr"$(INTDIR)/"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\P_edge.obj" : $(SOURCE) $(DEP_CPP_P_EDG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"
   $(BuildCmds)

"$(INTDIR)\P_edge.sbr" : $(SOURCE) $(DEP_CPP_P_EDG) "$(INTDIR)"\
 "$(INTDIR)\Thred.pch"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
################################################################################
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
################################################################################

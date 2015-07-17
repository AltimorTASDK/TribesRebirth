# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=matilda2 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to matilda2 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "matilda2 - Win32 Release" && "$(CFG)" !=\
 "matilda2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "matilda2.mak" CFG="matilda2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "matilda2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "matilda2 - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "matilda2 - Win32 Debug"
RSC=rc.exe
CPP=cl.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "matilda2 - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Exe"
# PROP Intermediate_Dir "Obj"
# PROP Target_Dir ""
OUTDIR=.\Exe
INTDIR=.\Obj

ALL : "$(OUTDIR)\matilda2.exe" "$(OUTDIR)\matilda2.bsc"

CLEAN : 
	-@erase "$(INTDIR)\ChangeLighting.obj"
	-@erase "$(INTDIR)\ChangeLighting.sbr"
	-@erase "$(INTDIR)\colorPreviewFrame.obj"
	-@erase "$(INTDIR)\colorPreviewFrame.sbr"
	-@erase "$(INTDIR)\copyDetailDlg.obj"
	-@erase "$(INTDIR)\copyDetailDlg.sbr"
	-@erase "$(INTDIR)\detailLevelsDlg.obj"
	-@erase "$(INTDIR)\detailLevelsDlg.sbr"
	-@erase "$(INTDIR)\gfxPreviewWind.obj"
	-@erase "$(INTDIR)\gfxPreviewWind.sbr"
	-@erase "$(INTDIR)\matilda2.obj"
	-@erase "$(INTDIR)\matilda2.res"
	-@erase "$(INTDIR)\matilda2.sbr"
	-@erase "$(INTDIR)\matilda2Dlg.obj"
	-@erase "$(INTDIR)\matilda2Dlg.sbr"
	-@erase "$(INTDIR)\Mledit.obj"
	-@erase "$(INTDIR)\Mledit.sbr"
	-@erase "$(INTDIR)\palEditDlg.obj"
	-@erase "$(INTDIR)\palEditDlg.sbr"
	-@erase "$(INTDIR)\palHelp.obj"
	-@erase "$(INTDIR)\palHelp.sbr"
	-@erase "$(INTDIR)\palSelListBox.obj"
	-@erase "$(INTDIR)\palSelListBox.sbr"
	-@erase "$(INTDIR)\RGBEditDlg.obj"
	-@erase "$(INTDIR)\RGBEditDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TEXEditDlg.obj"
	-@erase "$(INTDIR)\TEXEditDlg.sbr"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\matilda2.bsc"
	-@erase "$(OUTDIR)\matilda2.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Gr /Zp1 /MTd /W3 /WX /Gi /GX /Zi /Od /I "inc" /I "res" /I "..\..\core96\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "M_BACK" /D "STRICT" /D "EXPORT" /D "_MBCS" /D "DEBUG" /FR /YX"stdafx.h" /c
CPP_PROJ=/nologo /Gr /Zp1 /MTd /W3 /WX /Gi /GX /Zi /Od /I "inc" /I "res" /I\
 "..\..\core96\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "M_BACK"\
 /D "STRICT" /D "EXPORT" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/matilda2.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Obj/
CPP_SBRS=.\Obj/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "inc res ." /d "NDEBUG" /d "MSVC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/matilda2.res" /i "inc res ." /d "NDEBUG" /d\
 "MSVC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/matilda2.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ChangeLighting.sbr" \
	"$(INTDIR)\colorPreviewFrame.sbr" \
	"$(INTDIR)\copyDetailDlg.sbr" \
	"$(INTDIR)\detailLevelsDlg.sbr" \
	"$(INTDIR)\gfxPreviewWind.sbr" \
	"$(INTDIR)\matilda2.sbr" \
	"$(INTDIR)\matilda2Dlg.sbr" \
	"$(INTDIR)\Mledit.sbr" \
	"$(INTDIR)\palEditDlg.sbr" \
	"$(INTDIR)\palHelp.sbr" \
	"$(INTDIR)\palSelListBox.sbr" \
	"$(INTDIR)\RGBEditDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TEXEditDlg.sbr"

"$(OUTDIR)\matilda2.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 \darkstar\develop\core96\lib\dmcore.lib \darkstar\develop\core96\ddrawlib\ddrawm.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=\darkstar\develop\core96\lib\dmcore.lib\
 \darkstar\develop\core96\ddrawlib\ddrawm.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/matilda2.pdb" /machine:I386\
 /out:"$(OUTDIR)/matilda2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChangeLighting.obj" \
	"$(INTDIR)\colorPreviewFrame.obj" \
	"$(INTDIR)\copyDetailDlg.obj" \
	"$(INTDIR)\detailLevelsDlg.obj" \
	"$(INTDIR)\gfxPreviewWind.obj" \
	"$(INTDIR)\matilda2.obj" \
	"$(INTDIR)\matilda2.res" \
	"$(INTDIR)\matilda2Dlg.obj" \
	"$(INTDIR)\Mledit.obj" \
	"$(INTDIR)\palEditDlg.obj" \
	"$(INTDIR)\palHelp.obj" \
	"$(INTDIR)\palSelListBox.obj" \
	"$(INTDIR)\RGBEditDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TEXEditDlg.obj"

"$(OUTDIR)\matilda2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "matilda2 - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Exe"
# PROP Intermediate_Dir "Obj"
# PROP Target_Dir ""
OUTDIR=.\Exe
INTDIR=.\Obj

ALL : "$(OUTDIR)\matilda2.exe" "$(OUTDIR)\matilda2.bsc"

CLEAN : 
	-@erase "$(INTDIR)\ChangeLighting.obj"
	-@erase "$(INTDIR)\ChangeLighting.sbr"
	-@erase "$(INTDIR)\colorPreviewFrame.obj"
	-@erase "$(INTDIR)\colorPreviewFrame.sbr"
	-@erase "$(INTDIR)\copyDetailDlg.obj"
	-@erase "$(INTDIR)\copyDetailDlg.sbr"
	-@erase "$(INTDIR)\detailLevelsDlg.obj"
	-@erase "$(INTDIR)\detailLevelsDlg.sbr"
	-@erase "$(INTDIR)\gfxPreviewWind.obj"
	-@erase "$(INTDIR)\gfxPreviewWind.sbr"
	-@erase "$(INTDIR)\matilda2.obj"
	-@erase "$(INTDIR)\matilda2.res"
	-@erase "$(INTDIR)\matilda2.sbr"
	-@erase "$(INTDIR)\matilda2Dlg.obj"
	-@erase "$(INTDIR)\matilda2Dlg.sbr"
	-@erase "$(INTDIR)\Mledit.obj"
	-@erase "$(INTDIR)\Mledit.sbr"
	-@erase "$(INTDIR)\palEditDlg.obj"
	-@erase "$(INTDIR)\palEditDlg.sbr"
	-@erase "$(INTDIR)\palHelp.obj"
	-@erase "$(INTDIR)\palHelp.sbr"
	-@erase "$(INTDIR)\palSelListBox.obj"
	-@erase "$(INTDIR)\palSelListBox.sbr"
	-@erase "$(INTDIR)\RGBEditDlg.obj"
	-@erase "$(INTDIR)\RGBEditDlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TEXEditDlg.obj"
	-@erase "$(INTDIR)\TEXEditDlg.sbr"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\matilda2.bsc"
	-@erase "$(OUTDIR)\matilda2.exe"
	-@erase "$(OUTDIR)\matilda2.ilk"
	-@erase "$(OUTDIR)\matilda2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Gr /Zp1 /MTd /W3 /WX /Gi /GX /Zi /Od /I "inc" /I "res" /I "..\..\core96\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "M_BACK" /D "STRICT" /D "EXPORT" /D "_MBCS" /D "DEBUG" /FR /YX"stdafx.h" /c
CPP_PROJ=/nologo /Gr /Zp1 /MTd /W3 /WX /Gi /GX /Zi /Od /I "inc" /I "res" /I\
 "..\..\core96\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "M_BACK"\
 /D "STRICT" /D "EXPORT" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/matilda2.pch" /YX"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Obj/
CPP_SBRS=.\Obj/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc res ." /d "_DEBUG" /d "MSVC"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/matilda2.res" /i "inc res ." /d "_DEBUG" /d\
 "MSVC" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/matilda2.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ChangeLighting.sbr" \
	"$(INTDIR)\colorPreviewFrame.sbr" \
	"$(INTDIR)\copyDetailDlg.sbr" \
	"$(INTDIR)\detailLevelsDlg.sbr" \
	"$(INTDIR)\gfxPreviewWind.sbr" \
	"$(INTDIR)\matilda2.sbr" \
	"$(INTDIR)\matilda2Dlg.sbr" \
	"$(INTDIR)\Mledit.sbr" \
	"$(INTDIR)\palEditDlg.sbr" \
	"$(INTDIR)\palHelp.sbr" \
	"$(INTDIR)\palSelListBox.sbr" \
	"$(INTDIR)\RGBEditDlg.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TEXEditDlg.sbr"

"$(OUTDIR)\matilda2.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 \darkstar\develop\core96\lib\dmcore.lib \darkstar\develop\core96\ddrawlib\ddrawm.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=\darkstar\develop\core96\lib\dmcore.lib\
 \darkstar\develop\core96\ddrawlib\ddrawm.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/matilda2.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/matilda2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChangeLighting.obj" \
	"$(INTDIR)\colorPreviewFrame.obj" \
	"$(INTDIR)\copyDetailDlg.obj" \
	"$(INTDIR)\detailLevelsDlg.obj" \
	"$(INTDIR)\gfxPreviewWind.obj" \
	"$(INTDIR)\matilda2.obj" \
	"$(INTDIR)\matilda2.res" \
	"$(INTDIR)\matilda2Dlg.obj" \
	"$(INTDIR)\Mledit.obj" \
	"$(INTDIR)\palEditDlg.obj" \
	"$(INTDIR)\palHelp.obj" \
	"$(INTDIR)\palSelListBox.obj" \
	"$(INTDIR)\RGBEditDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TEXEditDlg.obj"

"$(OUTDIR)\matilda2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "matilda2 - Win32 Release"
# Name "matilda2 - Win32 Debug"

!IF  "$(CFG)" == "matilda2 - Win32 Release"

!ELSEIF  "$(CFG)" == "matilda2 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Code\colorPreviewFrame.cpp
DEP_CPP_COLOR=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\colorPreviewFrame.obj" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\colorPreviewFrame.sbr" : $(SOURCE) $(DEP_CPP_COLOR) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\StdAfx.cpp
DEP_CPP_STDAF=\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\RGBEditDlg.cpp
DEP_CPP_RGBED=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\RGBEditDlg.obj" : $(SOURCE) $(DEP_CPP_RGBED) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\RGBEditDlg.sbr" : $(SOURCE) $(DEP_CPP_RGBED) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\palSelListBox.cpp
DEP_CPP_PALSE=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palhelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\palSelListBox.obj" : $(SOURCE) $(DEP_CPP_PALSE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\palSelListBox.sbr" : $(SOURCE) $(DEP_CPP_PALSE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\palHelp.cpp
DEP_CPP_PALHE=\
	".\inc\palhelp.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\palHelp.obj" : $(SOURCE) $(DEP_CPP_PALHE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\palHelp.sbr" : $(SOURCE) $(DEP_CPP_PALHE) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\palEditDlg.cpp
DEP_CPP_PALED=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palhelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\palEditDlg.obj" : $(SOURCE) $(DEP_CPP_PALED) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\palEditDlg.sbr" : $(SOURCE) $(DEP_CPP_PALED) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\Mledit.cpp
DEP_CPP_MLEDI=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_init.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\Mledit.obj" : $(SOURCE) $(DEP_CPP_MLEDI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Mledit.sbr" : $(SOURCE) $(DEP_CPP_MLEDI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\matilda2Dlg.cpp

!IF  "$(CFG)" == "matilda2 - Win32 Release"

DEP_CPP_MATIL=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d3d.h"\
	"..\..\core96\inc\d3dcaps.h"\
	"..\..\core96\inc\d3dmacs.h"\
	"..\..\core96\inc\d3dtypes.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\d_funcs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\direct3d.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\fn_all.h"\
	"..\..\core96\inc\fn_table.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_cds.h"\
	"..\..\core96\inc\g_d3d.h"\
	"..\..\core96\inc\g_d3dtca.h"\
	"..\..\core96\inc\g_ddraw.h"\
	"..\..\core96\inc\g_font.h"\
	"..\..\core96\inc\g_mem.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\g_poly.h"\
	"..\..\core96\inc\g_raster.h"\
	"..\..\core96\inc\g_smanag.h"\
	"..\..\core96\inc\g_surfac.h"\
	"..\..\core96\inc\gfx.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_cam.h"\
	"..\..\core96\inc\ts_cell.h"\
	"..\..\core96\inc\ts_cont.h"\
	"..\..\core96\inc\ts_cstat.h"\
	"..\..\core96\inc\ts_det.h"\
	"..\..\core96\inc\ts_gfx.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_init.h"\
	"..\..\core96\inc\ts_light.h"\
	"..\..\core96\inc\ts_limit.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_null.h"\
	"..\..\core96\inc\ts_part.h"\
	"..\..\core96\inc\ts_persp.h"\
	"..\..\core96\inc\ts_plist.h"\
	"..\..\core96\inc\ts_rendr.h"\
	"..\..\core96\inc\ts_rintr.h"\
	"..\..\core96\inc\ts_scenl.h"\
	"..\..\core96\inc\ts_shape.h"\
	"..\..\core96\inc\ts_sinst.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\ChangeLighting.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matilda2Dlg.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palhelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	
NODEP_CPP_MATIL=\
	"..\..\core96\inc\d3dcom.h"\
	"..\..\core96\inc\subwtype.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\matilda2Dlg.obj" : $(SOURCE) $(DEP_CPP_MATIL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\matilda2Dlg.sbr" : $(SOURCE) $(DEP_CPP_MATIL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "matilda2 - Win32 Debug"

DEP_CPP_MATIL=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_mat.h"\
	".\inc\ChangeLighting.h"\
	".\inc\matilda2.h"\
	".\inc\matilda2Dlg.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palhelp.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\matilda2Dlg.obj" : $(SOURCE) $(DEP_CPP_MATIL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\matilda2Dlg.sbr" : $(SOURCE) $(DEP_CPP_MATIL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\matilda2.cpp
DEP_CPP_MATILD=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d3d.h"\
	"..\..\core96\inc\d3dcaps.h"\
	"..\..\core96\inc\d3dmacs.h"\
	"..\..\core96\inc\d3dtypes.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\d_funcs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\direct3d.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\fn_all.h"\
	"..\..\core96\inc\fn_table.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_cds.h"\
	"..\..\core96\inc\g_d3d.h"\
	"..\..\core96\inc\g_d3dtca.h"\
	"..\..\core96\inc\g_ddraw.h"\
	"..\..\core96\inc\g_font.h"\
	"..\..\core96\inc\g_mem.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\g_poly.h"\
	"..\..\core96\inc\g_raster.h"\
	"..\..\core96\inc\g_smanag.h"\
	"..\..\core96\inc\g_surfac.h"\
	"..\..\core96\inc\gfx.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_cam.h"\
	"..\..\core96\inc\ts_cell.h"\
	"..\..\core96\inc\ts_cont.h"\
	"..\..\core96\inc\ts_cstat.h"\
	"..\..\core96\inc\ts_det.h"\
	"..\..\core96\inc\ts_gfx.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_init.h"\
	"..\..\core96\inc\ts_light.h"\
	"..\..\core96\inc\ts_limit.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_null.h"\
	"..\..\core96\inc\ts_part.h"\
	"..\..\core96\inc\ts_persp.h"\
	"..\..\core96\inc\ts_plist.h"\
	"..\..\core96\inc\ts_rendr.h"\
	"..\..\core96\inc\ts_rintr.h"\
	"..\..\core96\inc\ts_scenl.h"\
	"..\..\core96\inc\ts_shape.h"\
	"..\..\core96\inc\ts_sinst.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matilda2Dlg.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palhelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	
NODEP_CPP_MATILD=\
	"..\..\core96\inc\d3dcom.h"\
	"..\..\core96\inc\subwtype.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\matilda2.obj" : $(SOURCE) $(DEP_CPP_MATILD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\matilda2.sbr" : $(SOURCE) $(DEP_CPP_MATILD) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\gfxPreviewWind.cpp

!IF  "$(CFG)" == "matilda2 - Win32 Release"

DEP_CPP_GFXPR=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d3d.h"\
	"..\..\core96\inc\d3dcaps.h"\
	"..\..\core96\inc\d3dmacs.h"\
	"..\..\core96\inc\d3dtypes.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\d_funcs.h"\
	"..\..\core96\inc\direct3d.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\fn_all.h"\
	"..\..\core96\inc\fn_table.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_cds.h"\
	"..\..\core96\inc\g_d3d.h"\
	"..\..\core96\inc\g_d3dtca.h"\
	"..\..\core96\inc\g_ddraw.h"\
	"..\..\core96\inc\g_font.h"\
	"..\..\core96\inc\g_mem.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\g_poly.h"\
	"..\..\core96\inc\g_raster.h"\
	"..\..\core96\inc\g_smanag.h"\
	"..\..\core96\inc\g_surfac.h"\
	"..\..\core96\inc\gfx.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts.h"\
	"..\..\core96\inc\ts_cam.h"\
	"..\..\core96\inc\ts_cell.h"\
	"..\..\core96\inc\ts_cont.h"\
	"..\..\core96\inc\ts_cstat.h"\
	"..\..\core96\inc\ts_det.h"\
	"..\..\core96\inc\ts_gfx.h"\
	"..\..\core96\inc\ts_init.h"\
	"..\..\core96\inc\ts_light.h"\
	"..\..\core96\inc\ts_limit.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_null.h"\
	"..\..\core96\inc\ts_part.h"\
	"..\..\core96\inc\ts_persp.h"\
	"..\..\core96\inc\ts_plist.h"\
	"..\..\core96\inc\ts_rendr.h"\
	"..\..\core96\inc\ts_rintr.h"\
	"..\..\core96\inc\ts_scenl.h"\
	"..\..\core96\inc\ts_shape.h"\
	"..\..\core96\inc\ts_sinst.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	
NODEP_CPP_GFXPR=\
	"..\..\core96\inc\d3dcom.h"\
	"..\..\core96\inc\subwtype.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\gfxPreviewWind.obj" : $(SOURCE) $(DEP_CPP_GFXPR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gfxPreviewWind.sbr" : $(SOURCE) $(DEP_CPP_GFXPR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "matilda2 - Win32 Debug"

DEP_CPP_GFXPR=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\ts_mat.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\gfxPreviewWind.obj" : $(SOURCE) $(DEP_CPP_GFXPR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gfxPreviewWind.sbr" : $(SOURCE) $(DEP_CPP_GFXPR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\detailLevelsDlg.cpp
DEP_CPP_DETAI=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\detailLevelsDlg.obj" : $(SOURCE) $(DEP_CPP_DETAI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\detailLevelsDlg.sbr" : $(SOURCE) $(DEP_CPP_DETAI) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\TEXEditDlg.cpp
DEP_CPP_TEXED=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d3d.h"\
	"..\..\core96\inc\d3dcaps.h"\
	"..\..\core96\inc\d3dmacs.h"\
	"..\..\core96\inc\d3dtypes.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\d_funcs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\direct3d.h"\
	"..\..\core96\inc\filedefs.h"\
	"..\..\core96\inc\filstrm.h"\
	"..\..\core96\inc\fn_all.h"\
	"..\..\core96\inc\fn_table.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_cds.h"\
	"..\..\core96\inc\g_d3d.h"\
	"..\..\core96\inc\g_d3dtca.h"\
	"..\..\core96\inc\g_ddraw.h"\
	"..\..\core96\inc\g_font.h"\
	"..\..\core96\inc\g_mem.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\g_poly.h"\
	"..\..\core96\inc\g_raster.h"\
	"..\..\core96\inc\g_smanag.h"\
	"..\..\core96\inc\g_surfac.h"\
	"..\..\core96\inc\gfx.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_cam.h"\
	"..\..\core96\inc\ts_cell.h"\
	"..\..\core96\inc\ts_cont.h"\
	"..\..\core96\inc\ts_cstat.h"\
	"..\..\core96\inc\ts_det.h"\
	"..\..\core96\inc\ts_gfx.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_init.h"\
	"..\..\core96\inc\ts_light.h"\
	"..\..\core96\inc\ts_limit.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_null.h"\
	"..\..\core96\inc\ts_part.h"\
	"..\..\core96\inc\ts_persp.h"\
	"..\..\core96\inc\ts_plist.h"\
	"..\..\core96\inc\ts_rendr.h"\
	"..\..\core96\inc\ts_rintr.h"\
	"..\..\core96\inc\ts_scenl.h"\
	"..\..\core96\inc\ts_shape.h"\
	"..\..\core96\inc\ts_sinst.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palhelp.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	
NODEP_CPP_TEXED=\
	"..\..\core96\inc\d3dcom.h"\
	"..\..\core96\inc\subwtype.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\TEXEditDlg.obj" : $(SOURCE) $(DEP_CPP_TEXED) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\TEXEditDlg.sbr" : $(SOURCE) $(DEP_CPP_TEXED) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\res\matilda2.rc
DEP_RSC_MATILDA=\
	".\res\icon2.ico"\
	".\res\matilda2.ico"\
	".\res\matilda2.rc2"\
	

!IF  "$(CFG)" == "matilda2 - Win32 Release"


"$(INTDIR)\matilda2.res" : $(SOURCE) $(DEP_RSC_MATILDA) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/matilda2.res" /i "inc res ." /i "res" /d\
 "NDEBUG" /d "MSVC" $(SOURCE)


!ELSEIF  "$(CFG)" == "matilda2 - Win32 Debug"


"$(INTDIR)\matilda2.res" : $(SOURCE) $(DEP_RSC_MATILDA) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/matilda2.res" /i "inc res ." /i "res" /d\
 "_DEBUG" /d "MSVC" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\copyDetailDlg.cpp
DEP_CPP_COPYD=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\copyDetailDlg.obj" : $(SOURCE) $(DEP_CPP_COPYD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\copyDetailDlg.sbr" : $(SOURCE) $(DEP_CPP_COPYD) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\ChangeLighting.cpp
DEP_CPP_CHANG=\
	"..\..\core96\inc\base.h"\
	"..\..\core96\inc\classio.h"\
	"..\..\core96\inc\classnum.h"\
	"..\..\core96\inc\d_defs.h"\
	"..\..\core96\inc\diction.h"\
	"..\..\core96\inc\g_barray.h"\
	"..\..\core96\inc\g_bitmap.h"\
	"..\..\core96\inc\g_pal.h"\
	"..\..\core96\inc\m_base.h"\
	"..\..\core96\inc\m_box.h"\
	"..\..\core96\inc\m_dist.h"\
	"..\..\core96\inc\m_dot.h"\
	"..\..\core96\inc\m_euler.h"\
	"..\..\core96\inc\m_lseg.h"\
	"..\..\core96\inc\m_mat.h"\
	"..\..\core96\inc\m_mul.h"\
	"..\..\core96\inc\m_plane.h"\
	"..\..\core96\inc\m_plist.h"\
	"..\..\core96\inc\m_point.h"\
	"..\..\core96\inc\m_quat.h"\
	"..\..\core96\inc\m_rect.h"\
	"..\..\core96\inc\m_sphere.h"\
	"..\..\core96\inc\m_trig.h"\
	"..\..\core96\inc\ml.h"\
	"..\..\core96\inc\streamio.h"\
	"..\..\core96\inc\ts_bmarr.h"\
	"..\..\core96\inc\ts_gfxbm.h"\
	"..\..\core96\inc\ts_mat.h"\
	"..\..\core96\inc\ts_txman.h"\
	"..\..\core96\inc\ts_types.h"\
	".\inc\ChangeLighting.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\ChangeLighting.obj" : $(SOURCE) $(DEP_CPP_CHANG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\ChangeLighting.sbr" : $(SOURCE) $(DEP_CPP_CHANG) "$(INTDIR)"
   $(BuildCmds)

# End Source File
# End Target
# End Project
################################################################################

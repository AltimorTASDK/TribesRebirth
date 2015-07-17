# Microsoft Developer Studio Generated NMAKE File, Based on matilda2.dsp
!IF "$(CFG)" == ""
CFG=matilda2 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to matilda2 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "matilda2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "matilda2.mak" CFG="matilda2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "matilda2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
OUTDIR=.\Exe
INTDIR=.\Obj
# Begin Custom Macros
OutDir=.\.\Exe
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\matilda2.exe" "$(OUTDIR)\matilda2.bsc"

!ELSE 

ALL : "$(OUTDIR)\matilda2.exe" "$(OUTDIR)\matilda2.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\matilda2.bsc"
	-@erase "$(OUTDIR)\matilda2.exe"
	-@erase "$(OUTDIR)\matilda2.ilk"
	-@erase "$(OUTDIR)\matilda2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\matilda2.bsc" 
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
LINK32_FLAGS=\darkstar\lib\dmcore.lib \darkstar\lib\dmts3.lib\
 \darkstar\lib\dmgfx.lib \darkstar\lib\dmgfxio.lib \darkstar\lib\dmdgfx.lib\
 \darkstar\lib\dmitr.lib \darkstar\lib\dmgrd.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)\matilda2.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\matilda2.exe" 
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

CPP_PROJ=/nologo /GB /Gz /Zp4 /MTd /W3 /Gm /Gi /GR /GX /Zi /Od /I ".\inc" /I\
 ".\res" /I "D:\darkstar\inc" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D\
 "M_BACK" /D "STRICT" /D "EXPORT" /D "_MBCS" /D _USERENTRY=__cdecl\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\matilda2.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Obj/
CPP_SBRS=.\Obj/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\matilda2.res" /i ".\inc res" /d "_DEBUG" /d\
 "MSVC" 

!IF "$(CFG)" == "matilda2 - Win32 Debug"
SOURCE=.\Code\ChangeLighting.cpp
DEP_CPP_CHANG=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\ChangeLighting.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\ChangeLighting.obj"	"$(INTDIR)\ChangeLighting.sbr" : $(SOURCE)\
 $(DEP_CPP_CHANG) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\colorPreviewFrame.cpp
DEP_CPP_COLOR=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\colorPreviewFrame.obj"	"$(INTDIR)\colorPreviewFrame.sbr" : $(SOURCE)\
 $(DEP_CPP_COLOR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\copyDetailDlg.cpp
DEP_CPP_COPYD=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\copyDetailDlg.obj"	"$(INTDIR)\copyDetailDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_COPYD) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\detailLevelsDlg.cpp
DEP_CPP_DETAI=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\detailLevelsDlg.obj"	"$(INTDIR)\detailLevelsDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_DETAI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\gfxPreviewWind.cpp
DEP_CPP_GFXPR=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\d_defs.h"\
	"..\..\inc\d_funcs.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\fn_all.h"\
	"..\..\inc\fn_table.h"\
	"..\..\inc\g_cds.h"\
	"..\..\inc\g_ddraw.h"\
	"..\..\inc\g_mem.h"\
	"..\..\inc\g_pal.h"\
	"..\..\inc\g_raster.h"\
	"..\..\inc\g_surfac.h"\
	"..\..\inc\g_types.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_collision.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\talgorithm.h"\
	"..\..\inc\tbitvector.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts.h"\
	"..\..\inc\ts_camera.h"\
	"..\..\inc\ts_celanimmesh.h"\
	"..\..\inc\ts_light.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\ts_pointarray.h"\
	"..\..\inc\ts_rendercontext.h"\
	"..\..\inc\ts_renderitem.h"\
	"..\..\inc\ts_shape.h"\
	"..\..\inc\ts_shapeinst.h"\
	"..\..\inc\ts_transform.h"\
	"..\..\inc\ts_types.h"\
	"..\..\inc\ts_vertex.h"\
	"..\..\inc\tsorted.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	
CPP_SWITCHES=/nologo /GB /Gz /Zp4 /MTd /W3 /Gm /Gi /GR /GX /Zi /Od /I ".\inc"\
 /I ".\res" /I "D:\darkstar\inc" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC"\
 /D "M_BACK" /D "STRICT" /D "EXPORT" /D "_MBCS" /D _USERENTRY=__cdecl\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\matilda2.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\gfxPreviewWind.obj"	"$(INTDIR)\gfxPreviewWind.sbr" : $(SOURCE)\
 $(DEP_CPP_GFXPR) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


SOURCE=.\Code\matilda2.cpp
DEP_CPP_MATIL=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\d_defs.h"\
	"..\..\inc\d_funcs.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\fn_all.h"\
	"..\..\inc\fn_table.h"\
	"..\..\inc\g_cds.h"\
	"..\..\inc\g_ddraw.h"\
	"..\..\inc\g_mem.h"\
	"..\..\inc\g_pal.h"\
	"..\..\inc\g_raster.h"\
	"..\..\inc\g_surfac.h"\
	"..\..\inc\g_types.h"\
	"..\..\inc\gfxres.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_collision.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\talgorithm.h"\
	"..\..\inc\tbitvector.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts.h"\
	"..\..\inc\ts_camera.h"\
	"..\..\inc\ts_celanimmesh.h"\
	"..\..\inc\ts_light.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\ts_pointarray.h"\
	"..\..\inc\ts_rendercontext.h"\
	"..\..\inc\ts_renderitem.h"\
	"..\..\inc\ts_shape.h"\
	"..\..\inc\ts_shapeinst.h"\
	"..\..\inc\ts_transform.h"\
	"..\..\inc\ts_types.h"\
	"..\..\inc\ts_vertex.h"\
	"..\..\inc\tsorted.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matilda2Dlg.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palHelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	

"$(INTDIR)\matilda2.obj"	"$(INTDIR)\matilda2.sbr" : $(SOURCE) $(DEP_CPP_MATIL)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\res\matilda2.rc
DEP_RSC_MATILD=\
	".\res\icon2.ico"\
	".\res\matilda2.ico"\
	".\res\matilda2.rc2"\
	

"$(INTDIR)\matilda2.res" : $(SOURCE) $(DEP_RSC_MATILD) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\matilda2.res" /i "res" /i ".\res" /d "_DEBUG" /d\
 "MSVC" $(SOURCE)


SOURCE=.\Code\matilda2Dlg.cpp
DEP_CPP_MATILDA=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\d_defs.h"\
	"..\..\inc\d_funcs.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\fn_all.h"\
	"..\..\inc\fn_table.h"\
	"..\..\inc\g_cds.h"\
	"..\..\inc\g_ddraw.h"\
	"..\..\inc\g_mem.h"\
	"..\..\inc\g_pal.h"\
	"..\..\inc\g_raster.h"\
	"..\..\inc\g_surfac.h"\
	"..\..\inc\g_types.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_collision.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\talgorithm.h"\
	"..\..\inc\tbitvector.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts.h"\
	"..\..\inc\ts_camera.h"\
	"..\..\inc\ts_celanimmesh.h"\
	"..\..\inc\ts_light.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\ts_pointarray.h"\
	"..\..\inc\ts_rendercontext.h"\
	"..\..\inc\ts_renderitem.h"\
	"..\..\inc\ts_shape.h"\
	"..\..\inc\ts_shapeinst.h"\
	"..\..\inc\ts_transform.h"\
	"..\..\inc\ts_types.h"\
	"..\..\inc\ts_vertex.h"\
	"..\..\inc\tsorted.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\ChangeLighting.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\copyDetailDlg.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matilda2Dlg.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palHelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	

"$(INTDIR)\matilda2Dlg.obj"	"$(INTDIR)\matilda2Dlg.sbr" : $(SOURCE)\
 $(DEP_CPP_MATILDA) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\Mledit.cpp
DEP_CPP_MLEDI=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Mledit.obj"	"$(INTDIR)\Mledit.sbr" : $(SOURCE) $(DEP_CPP_MLEDI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\palEditDlg.cpp
DEP_CPP_PALED=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palEditDlg.h"\
	".\inc\palHelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\palEditDlg.obj"	"$(INTDIR)\palEditDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_PALED) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\palHelp.cpp
DEP_CPP_PALHE=\
	".\inc\palHelp.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\palHelp.obj"	"$(INTDIR)\palHelp.sbr" : $(SOURCE) $(DEP_CPP_PALHE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\palSelListBox.cpp
DEP_CPP_PALSE=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palHelp.h"\
	".\inc\palSelListBox.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\palSelListBox.obj"	"$(INTDIR)\palSelListBox.sbr" : $(SOURCE)\
 $(DEP_CPP_PALSE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\RGBEditDlg.cpp
DEP_CPP_RGBED=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\colorPreviewFrame.h"\
	".\inc\RGBEditDlg.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\RGBEditDlg.obj"	"$(INTDIR)\RGBEditDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_RGBED) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\StdAfx.cpp
DEP_CPP_STDAF=\
	".\inc\stdafx.h"\
	

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Code\TEXEditDlg.cpp
DEP_CPP_TEXED=\
	"..\..\inc\base.h"\
	"..\..\inc\bitstream.h"\
	"..\..\inc\blkstrm.h"\
	"..\..\inc\d_defs.h"\
	"..\..\inc\d_funcs.h"\
	"..\..\inc\filedefs.h"\
	"..\..\inc\filstrm.h"\
	"..\..\inc\fn_all.h"\
	"..\..\inc\fn_table.h"\
	"..\..\inc\g_cds.h"\
	"..\..\inc\g_ddraw.h"\
	"..\..\inc\g_mem.h"\
	"..\..\inc\g_pal.h"\
	"..\..\inc\g_raster.h"\
	"..\..\inc\g_surfac.h"\
	"..\..\inc\g_types.h"\
	"..\..\inc\gfxres.h"\
	"..\..\inc\lock.h"\
	"..\..\inc\lzhstrm.h"\
	"..\..\inc\m_base.h"\
	"..\..\inc\m_box.h"\
	"..\..\inc\m_collision.h"\
	"..\..\inc\m_dist.h"\
	"..\..\inc\m_dot.h"\
	"..\..\inc\m_euler.h"\
	"..\..\inc\m_lseg.h"\
	"..\..\inc\m_mat2.h"\
	"..\..\inc\m_mat3.h"\
	"..\..\inc\m_mul.h"\
	"..\..\inc\m_plane.h"\
	"..\..\inc\m_plist.h"\
	"..\..\inc\m_point.h"\
	"..\..\inc\m_qsort.h"\
	"..\..\inc\m_quat.h"\
	"..\..\inc\m_random.h"\
	"..\..\inc\m_rect.h"\
	"..\..\inc\m_sphere.h"\
	"..\..\inc\m_trig.h"\
	"..\..\inc\memstrm.h"\
	"..\..\inc\ml.h"\
	"..\..\inc\persist.h"\
	"..\..\inc\resmanager.h"\
	"..\..\inc\rlestrm.h"\
	"..\..\inc\streamio.h"\
	"..\..\inc\streams.h"\
	"..\..\inc\talgorithm.h"\
	"..\..\inc\tbitvector.h"\
	"..\..\inc\threadbase.h"\
	"..\..\inc\ts.h"\
	"..\..\inc\ts_camera.h"\
	"..\..\inc\ts_celanimmesh.h"\
	"..\..\inc\ts_light.h"\
	"..\..\inc\ts_material.h"\
	"..\..\inc\ts_pointarray.h"\
	"..\..\inc\ts_rendercontext.h"\
	"..\..\inc\ts_renderitem.h"\
	"..\..\inc\ts_shape.h"\
	"..\..\inc\ts_shapeinst.h"\
	"..\..\inc\ts_transform.h"\
	"..\..\inc\ts_types.h"\
	"..\..\inc\ts_vertex.h"\
	"..\..\inc\tsorted.h"\
	"..\..\inc\tvector.h"\
	"..\..\inc\volstrm.h"\
	".\inc\detailLevelsDlg.h"\
	".\inc\gfxPreviewWind.h"\
	".\inc\matilda2.h"\
	".\inc\matsuprt.h"\
	".\inc\mledit.h"\
	".\inc\palHelp.h"\
	".\inc\stdafx.h"\
	".\inc\TEXEditDlg.h"\
	

"$(INTDIR)\TEXEditDlg.obj"	"$(INTDIR)\TEXEditDlg.sbr" : $(SOURCE)\
 $(DEP_CPP_TEXED) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


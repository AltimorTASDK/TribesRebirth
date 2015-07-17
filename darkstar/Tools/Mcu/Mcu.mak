# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Mcu - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Mcu - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Mcu - Win32 Release" && "$(CFG)" != "Mcu - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mcu.mak" CFG="Mcu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mcu - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Mcu - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "Mcu - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "Mcu - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "obj"
# PROP Target_Dir ""
OUTDIR=.\exe
INTDIR=.\obj

ALL : "$(OUTDIR)\Mcu.exe"

CLEAN : 
	-@erase "$(INTDIR)\3ds.obj"
	-@erase "$(INTDIR)\Couple.obj"
	-@erase "$(INTDIR)\Detail.obj"
	-@erase "$(INTDIR)\detailed.obj"
	-@erase "$(INTDIR)\Detio.obj"
	-@erase "$(INTDIR)\mainfrm.obj"
	-@erase "$(INTDIR)\Mats.obj"
	-@erase "$(INTDIR)\mcu.obj"
	-@erase "$(INTDIR)\mcu.res"
	-@erase "$(INTDIR)\mcudoc.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\Part.obj"
	-@erase "$(INTDIR)\Polyhedr.obj"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(OUTDIR)\Mcu.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC" /Yu"stdafx.h" /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I "..\..\core\base\inc"\
 /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I "..\..\core\ml\inc"\
 /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC" /Fp"$(INTDIR)/Mcu.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\obj/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "inc" /i "..\inc" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/mcu.res" /i "inc" /i "..\inc" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mcu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\..\core\lib\dmcore.lib lib\3dftkvc1.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libc"
LINK32_FLAGS=..\..\core\lib\dmcore.lib lib\3dftkvc1.lib /nologo\
 /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libc"\
 /out:"$(OUTDIR)/Mcu.exe" 
LINK32_OBJS= \
	"$(INTDIR)\3ds.obj" \
	"$(INTDIR)\Couple.obj" \
	"$(INTDIR)\Detail.obj" \
	"$(INTDIR)\detailed.obj" \
	"$(INTDIR)\Detio.obj" \
	"$(INTDIR)\mainfrm.obj" \
	"$(INTDIR)\Mats.obj" \
	"$(INTDIR)\mcu.obj" \
	"$(INTDIR)\mcu.res" \
	"$(INTDIR)\mcudoc.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\Part.obj" \
	"$(INTDIR)\Polyhedr.obj" \
	"$(INTDIR)\stdafx.obj"

"$(OUTDIR)\Mcu.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "obj"
# PROP Target_Dir ""
OUTDIR=.\exe
INTDIR=.\obj

ALL : "$(OUTDIR)\Mcu.exe"

CLEAN : 
	-@erase "$(INTDIR)\3ds.obj"
	-@erase "$(INTDIR)\Couple.obj"
	-@erase "$(INTDIR)\Detail.obj"
	-@erase "$(INTDIR)\detailed.obj"
	-@erase "$(INTDIR)\Detio.obj"
	-@erase "$(INTDIR)\mainfrm.obj"
	-@erase "$(INTDIR)\Mats.obj"
	-@erase "$(INTDIR)\mcu.obj"
	-@erase "$(INTDIR)\Mcu.pch"
	-@erase "$(INTDIR)\mcu.res"
	-@erase "$(INTDIR)\mcudoc.obj"
	-@erase "$(INTDIR)\Mesh.obj"
	-@erase "$(INTDIR)\Part.obj"
	-@erase "$(INTDIR)\Polyhedr.obj"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Mcu.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC" /Yu"stdafx.h" /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\obj/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "..\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/mcu.res" /i "inc" /i "..\inc" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Mcu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ..\..\core\lib\dmcore.lib lib\3dftkvc1.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libc"
LINK32_FLAGS=..\..\core\lib\dmcore.lib lib\3dftkvc1.lib /nologo\
 /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib:"libc"\
 /out:"$(OUTDIR)/Mcu.exe" 
LINK32_OBJS= \
	"$(INTDIR)\3ds.obj" \
	"$(INTDIR)\Couple.obj" \
	"$(INTDIR)\Detail.obj" \
	"$(INTDIR)\detailed.obj" \
	"$(INTDIR)\Detio.obj" \
	"$(INTDIR)\mainfrm.obj" \
	"$(INTDIR)\Mats.obj" \
	"$(INTDIR)\mcu.obj" \
	"$(INTDIR)\mcu.res" \
	"$(INTDIR)\mcudoc.obj" \
	"$(INTDIR)\Mesh.obj" \
	"$(INTDIR)\Part.obj" \
	"$(INTDIR)\Polyhedr.obj" \
	"$(INTDIR)\stdafx.obj"

"$(OUTDIR)\Mcu.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "Mcu - Win32 Release"
# Name "Mcu - Win32 Debug"

!IF  "$(CFG)" == "Mcu - Win32 Release"

!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\code\stdafx.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_STDAF=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\mcu.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_STDAF=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\mcu.h"\
	".\inc\stdafx.h"\
	
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fp"$(INTDIR)/Mcu.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\stdafx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Mcu.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Couple.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_COUPL=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Couple.obj" : $(SOURCE) $(DEP_CPP_COUPL) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_COUPL=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Couple.obj" : $(SOURCE) $(DEP_CPP_COUPL) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Detail.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_DETAI=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Detail.obj" : $(SOURCE) $(DEP_CPP_DETAI) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_DETAI=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Detail.obj" : $(SOURCE) $(DEP_CPP_DETAI) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\detailed.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_DETAIL=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\detailed.h"\
	".\inc\Except.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\detailed.obj" : $(SOURCE) $(DEP_CPP_DETAIL) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_DETAIL=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\detailed.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\detailed.obj" : $(SOURCE) $(DEP_CPP_DETAIL) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Detio.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_DETIO=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Detio.obj" : $(SOURCE) $(DEP_CPP_DETIO) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_DETIO=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Detio.obj" : $(SOURCE) $(DEP_CPP_DETIO) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\mainfrm.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_MAINF=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mainfrm.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mainfrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_MAINF=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mainfrm.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mainfrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Mats.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_MATS_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Mats.obj" : $(SOURCE) $(DEP_CPP_MATS_) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_MATS_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Mats.obj" : $(SOURCE) $(DEP_CPP_MATS_) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\mcu.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_MCU_C=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\detailed.h"\
	".\inc\Except.h"\
	".\inc\mainfrm.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mcu.obj" : $(SOURCE) $(DEP_CPP_MCU_C) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_MCU_C=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\detailed.h"\
	".\inc\Except.h"\
	".\inc\mainfrm.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mcu.obj" : $(SOURCE) $(DEP_CPP_MCU_C) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\mcudoc.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_MCUDO=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mcudoc.obj" : $(SOURCE) $(DEP_CPP_MCUDO) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_MCUDO=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\mcudoc.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\mcudoc.obj" : $(SOURCE) $(DEP_CPP_MCUDO) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Mesh.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_MESH_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Mesh.obj" : $(SOURCE) $(DEP_CPP_MESH_) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_MESH_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Mesh.obj" : $(SOURCE) $(DEP_CPP_MESH_) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Part.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_PART_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Part.obj" : $(SOURCE) $(DEP_CPP_PART_) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_PART_=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\core\ts\inc\ts_bsp.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_mesh.h"\
	"..\..\core\ts\inc\ts_mesh2.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\couple.h"\
	".\inc\detail.h"\
	".\inc\Except.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Part.obj" : $(SOURCE) $(DEP_CPP_PART_) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Polyhedr.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_POLYH=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_matimp.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\Polyhedr.obj" : $(SOURCE) $(DEP_CPP_POLYH) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_POLYH=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\mcu.h"\
	".\inc\polyhedr.h"\
	".\inc\Status.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\Polyhedr.obj" : $(SOURCE) $(DEP_CPP_POLYH) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\3ds.cpp

!IF  "$(CFG)" == "Mcu - Win32 Release"

DEP_CPP_3DS_C=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\stdafx.h"\
	

"$(INTDIR)\3ds.obj" : $(SOURCE) $(DEP_CPP_3DS_C) "$(INTDIR)"\
 "$(INTDIR)\Mcu.pch"
   $(CPP) /nologo /G5 /Gr /Zp1 /MT /W3 /GX /O2 /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "NDEBUG"\
 /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D "MSVC"\
 /Fp"$(INTDIR)/Mcu.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"

DEP_CPP_3DS_C=\
	"..\..\core\base\inc\base.h"\
	"..\..\core\base\inc\diction.h"\
	"..\..\core\classio\inc\classio.h"\
	"..\..\core\classio\inc\classnum.h"\
	"..\..\core\fileio\inc\filedefs.h"\
	"..\..\core\fileio\inc\filstrm.h"\
	"..\..\core\fileio\inc\streamio.h"\
	"..\..\Core\Gfx\Inc\d_defs.h"\
	"..\..\Core\Gfx\Inc\d_funcs.h"\
	"..\..\core\gfx\inc\g_barray.h"\
	"..\..\Core\Gfx\Inc\g_bitmap.h"\
	"..\..\Core\Gfx\Inc\g_font.h"\
	"..\..\Core\Gfx\Inc\g_pal.h"\
	"..\..\Core\Gfx\Inc\g_poly.h"\
	"..\..\Core\Gfx\Inc\g_raster.h"\
	"..\..\Core\Gfx\Inc\g_surfac.h"\
	"..\..\core\gfx\inc\gfx.h"\
	"..\..\core\ml\inc\m_base.h"\
	"..\..\core\ml\inc\m_box.h"\
	"..\..\core\ml\inc\m_dist.h"\
	"..\..\core\ml\inc\m_dot.h"\
	"..\..\core\ml\inc\m_euler.h"\
	"..\..\core\ml\inc\m_lseg.h"\
	"..\..\core\ml\inc\m_mat.h"\
	"..\..\core\ml\inc\m_mul.h"\
	"..\..\core\ml\inc\m_plane.h"\
	"..\..\core\ml\inc\m_plist.h"\
	"..\..\core\ml\inc\m_point.h"\
	"..\..\core\ml\inc\m_quat.h"\
	"..\..\core\ml\inc\m_rect.h"\
	"..\..\core\ml\inc\m_sphere.h"\
	"..\..\core\ml\inc\m_trig.h"\
	"..\..\core\ml\inc\ml.h"\
	"..\..\core\ts\inc\ts.h"\
	"..\..\core\ts\inc\ts_bmarr.h"\
	"..\..\Core\Ts\Inc\ts_cam.h"\
	"..\..\core\ts\inc\ts_cell.h"\
	"..\..\core\ts\inc\ts_cont.h"\
	"..\..\Core\Ts\Inc\ts_cstat.h"\
	"..\..\core\ts\inc\ts_det.h"\
	"..\..\Core\Ts\Inc\ts_gfx.h"\
	"..\..\core\ts\inc\ts_gfxbm.h"\
	"..\..\core\ts\inc\ts_init.h"\
	"..\..\core\ts\inc\ts_light.h"\
	"..\..\Core\Ts\Inc\ts_limit.h"\
	"..\..\core\ts\inc\ts_mat.h"\
	"..\..\core\ts\inc\ts_null.h"\
	"..\..\Core\Ts\Inc\ts_part.h"\
	"..\..\core\ts\inc\ts_persp.h"\
	"..\..\core\ts\inc\ts_plist.h"\
	"..\..\Core\Ts\Inc\ts_rendr.h"\
	"..\..\core\ts\inc\ts_rintr.h"\
	"..\..\core\ts\inc\ts_scenl.h"\
	"..\..\core\ts\inc\ts_shape.h"\
	"..\..\core\ts\inc\ts_sinst.h"\
	"..\..\Core\Ts\Inc\ts_txman.h"\
	"..\..\Core\Ts\Inc\ts_types.h"\
	".\inc\3ds.h"\
	".\inc\3dsftk.h"\
	".\inc\mcu.h"\
	".\inc\stdafx.h"\
	
# SUBTRACT CPP /YX /Yc /Yu

"$(INTDIR)\3ds.obj" : $(SOURCE) $(DEP_CPP_3DS_C) "$(INTDIR)"
   $(CPP) /nologo /G5 /Gr /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I\
 "..\..\core\base\inc" /I "..\..\core\fileio\inc" /I "..\..\core\classio\inc" /I\
 "..\..\core\ml\inc" /I "..\..\core\gfx\inc" /I "..\..\core\ts\inc" /D "_DEBUG"\
 /D "DEBUG" /D "WIN32" /D "_WINDOWS" /D "M_BACK" /D "TS_REAL" /D "_MBCS" /D\
 "MSVC" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\res\mcu.rc
DEP_RSC_MCU_R=\
	".\res\idr_main.ico"\
	".\res\toolbar.bmp"\
	

!IF  "$(CFG)" == "Mcu - Win32 Release"


"$(INTDIR)\mcu.res" : $(SOURCE) $(DEP_RSC_MCU_R) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/mcu.res" /i "inc" /i "..\inc" /i "res" /d\
 "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "Mcu - Win32 Debug"


"$(INTDIR)\mcu.res" : $(SOURCE) $(DEP_RSC_MCU_R) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/mcu.res" /i "inc" /i "..\inc" /i "res" /d\
 "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

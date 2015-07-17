# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=dtsexp - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to dtsexp - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dtsexp - Win32 Release" && "$(CFG)" != "dtsexp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "dtsexp.mak" CFG="dtsexp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dtsexp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dtsexp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "dtsexp - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "dtsexp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\dtsexp.dll"

CLEAN : 
	-@erase "$(INTDIR)\Dtsexp.obj"
	-@erase "$(INTDIR)\dtsexp.res"
	-@erase "$(INTDIR)\MeshBuilder.obj"
	-@erase "$(INTDIR)\PERSIST.OBJ"
	-@erase "$(INTDIR)\SceneEnum.obj"
	-@erase "$(INTDIR)\ShapeBuilder.obj"
	-@erase "$(OUTDIR)\dtsexp.dll"
	-@erase "$(OUTDIR)\dtsexp.exp"
	-@erase "$(OUTDIR)\dtsexp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "d:\darkstar\develop\core\classio\inc" /I "c:\3dsmax\MAXSDK\INCLUDE" /I "c:\phenx\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "MAXSDK" /D "SDUMP" /YX /c
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /I "d:\darkstar\develop\core\classio\inc"\
 /I "c:\3dsmax\MAXSDK\INCLUDE" /I "c:\phenx\inc" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "MSVC" /D "MAXSDK" /D "SDUMP" /Fp"$(INTDIR)/dtsexp.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/dtsexp.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dtsexp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB dmcore.lib /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib:"LIBC.lib"
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib COMCTL32.LIB dmcore.lib /nologo /subsystem:windows /dll /pdb:none\
 /machine:I386 /nodefaultlib:"LIBC.lib" /def:".\dtsexp.def"\
 /out:"$(OUTDIR)/dtsexp.dll" /implib:"$(OUTDIR)/dtsexp.lib" 
DEF_FILE= \
	".\dtsexp.def"
LINK32_OBJS= \
	"$(INTDIR)\Dtsexp.obj" \
	"$(INTDIR)\dtsexp.res" \
	"$(INTDIR)\MeshBuilder.obj" \
	"$(INTDIR)\PERSIST.OBJ" \
	"$(INTDIR)\SceneEnum.obj" \
	"$(INTDIR)\ShapeBuilder.obj" \
	"..\..\..\..\3dsmax\maxsdk\lib\Core.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Geom.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Mesh.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Util.lib"

"$(OUTDIR)\dtsexp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\..\..\3dsmax\plugins\dtsexp.dlo"

CLEAN : 
	-@erase "$(INTDIR)\Dtsexp.obj"
	-@erase "$(INTDIR)\dtsexp.res"
	-@erase "$(INTDIR)\MeshBuilder.obj"
	-@erase "$(INTDIR)\PERSIST.OBJ"
	-@erase "$(INTDIR)\SceneEnum.obj"
	-@erase "$(INTDIR)\ShapeBuilder.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\dtsexp.exp"
	-@erase "$(OUTDIR)\dtsexp.lib"
	-@erase "..\..\..\..\3dsmax\plugins\dtsexp.dlo"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /Zi /Od /I "inc" /I "c:\3dsmax\MAXSDK\INCLUDE" /I "c:\phenx\inc" /D "_DEBUG" /D "DEBUG" /D "DUMP" /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "MAXSDK" /D "SDUMP" /YX /c
CPP_PROJ=/nologo /MD /W3 /Gm /GR /GX /Zi /Od /I "inc" /I\
 "c:\3dsmax\MAXSDK\INCLUDE" /I "c:\phenx\inc" /D "_DEBUG" /D "DEBUG" /D "DUMP"\
 /D "WIN32" /D "_WINDOWS" /D "MSVC" /D "MAXSDK" /D "SDUMP"\
 /Fp"$(INTDIR)/dtsexp.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/dtsexp.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dtsexp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB dmcore.lib /nologo /subsystem:windows /dll /pdb:none /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"c:\3dsmax\plugins\dtsexp.dlo"
# SUBTRACT LINK32 /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib COMCTL32.LIB dmcore.lib /nologo /subsystem:windows /dll /pdb:none\
 /debug /machine:I386 /nodefaultlib:"LIBC.lib" /def:".\dtsexp.def"\
 /out:"c:\3dsmax\plugins\dtsexp.dlo" /implib:"$(OUTDIR)/dtsexp.lib" 
DEF_FILE= \
	".\dtsexp.def"
LINK32_OBJS= \
	"$(INTDIR)\Dtsexp.obj" \
	"$(INTDIR)\dtsexp.res" \
	"$(INTDIR)\MeshBuilder.obj" \
	"$(INTDIR)\PERSIST.OBJ" \
	"$(INTDIR)\SceneEnum.obj" \
	"$(INTDIR)\ShapeBuilder.obj" \
	"..\..\..\..\3dsmax\maxsdk\lib\Core.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Geom.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Mesh.lib" \
	"..\..\..\..\3dsmax\maxsdk\lib\Util.lib"

"..\..\..\..\3dsmax\plugins\dtsexp.dlo" : "$(OUTDIR)" $(DEF_FILE)\
 $(LINK32_OBJS)
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

# Name "dtsexp - Win32 Release"
# Name "dtsexp - Win32 Debug"

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\dtsexp.def

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\Dtsexp.cpp

!IF  "$(CFG)" == "dtsexp - Win32 Release"

DEP_CPP_DTSEX=\
	"\3DSMAX\maxsdk\include\acolor.h"\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3DSMAX\maxsdk\include\assert1.h"\
	"\3DSMAX\maxsdk\include\bitarray.h"\
	"\3DSMAX\maxsdk\include\box2.h"\
	"\3DSMAX\maxsdk\include\box3.h"\
	"\3DSMAX\maxsdk\include\channels.h"\
	"\3DSMAX\maxsdk\include\cmdmode.h"\
	"\3DSMAX\maxsdk\include\color.h"\
	"\3DSMAX\maxsdk\include\control.h"\
	"\3DSMAX\maxsdk\include\coreexp.h"\
	"\3DSMAX\maxsdk\include\custcont.h"\
	"\3DSMAX\maxsdk\include\dbgprint.h"\
	"\3DSMAX\maxsdk\include\dpoint3.h"\
	"\3DSMAX\maxsdk\include\evuser.h"\
	"\3DSMAX\maxsdk\include\export.h"\
	"\3DSMAX\maxsdk\include\gencam.h"\
	"\3DSMAX\maxsdk\include\genhier.h"\
	"\3DSMAX\maxsdk\include\genlight.h"\
	"\3DSMAX\maxsdk\include\genshape.h"\
	"\3DSMAX\maxsdk\include\geom.h"\
	"\3DSMAX\maxsdk\include\geomlib.h"\
	"\3DSMAX\maxsdk\include\gfx.h"\
	"\3DSMAX\maxsdk\include\gfxlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\hitdata.h"\
	"\3DSMAX\maxsdk\include\hold.h"\
	"\3DSMAX\maxsdk\include\impapi.h"\
	"\3DSMAX\maxsdk\include\impexp.h"\
	"\3DSMAX\maxsdk\include\imtl.h"\
	"\3DSMAX\maxsdk\include\inode.h"\
	"\3DSMAX\maxsdk\include\interval.h"\
	"\3dsmax\MAXSDK\INCLUDE\ioapi.h"\
	"\3DSMAX\maxsdk\include\iparamb.h"\
	"\3DSMAX\maxsdk\include\ipoint2.h"\
	"\3DSMAX\maxsdk\include\ipoint3.h"\
	"\3DSMAX\maxsdk\include\lockid.h"\
	"\3DSMAX\maxsdk\include\matrix2.h"\
	"\3DSMAX\maxsdk\include\matrix3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Max.h"\
	"\3DSMAX\maxsdk\include\maxapi.h"\
	"\3DSMAX\maxsdk\include\maxcom.h"\
	"\3DSMAX\maxsdk\include\maxtypes.h"\
	"\3DSMAX\maxsdk\include\mesh.h"\
	"\3DSMAX\maxsdk\include\meshlib.h"\
	"\3DSMAX\maxsdk\include\mouseman.h"\
	"\3DSMAX\maxsdk\include\mtl.h"\
	"\3DSMAX\maxsdk\include\nametab.h"\
	"\3DSMAX\maxsdk\include\nurbs.h"\
	"\3DSMAX\maxsdk\include\nurbslib.h"\
	"\3DSMAX\maxsdk\include\nurbsobj.h"\
	"\3DSMAX\maxsdk\include\object.h"\
	"\3DSMAX\maxsdk\include\objmode.h"\
	"\3DSMAX\maxsdk\include\patch.h"\
	"\3DSMAX\maxsdk\include\patchlib.h"\
	"\3DSMAX\maxsdk\include\patchobj.h"\
	"\3DSMAX\maxsdk\include\plugapi.h"\
	"\3DSMAX\maxsdk\include\plugin.h"\
	"\3DSMAX\maxsdk\include\point2.h"\
	"\3DSMAX\maxsdk\include\point3.h"\
	"\3DSMAX\maxsdk\include\ptrvec.h"\
	"\3DSMAX\maxsdk\include\quat.h"\
	"\3DSMAX\maxsdk\include\ref.h"\
	"\3DSMAX\maxsdk\include\render.h"\
	"\3DSMAX\maxsdk\include\rtclick.h"\
	"\3DSMAX\maxsdk\include\sceneapi.h"\
	"\3DSMAX\maxsdk\include\snap.h"\
	"\3DSMAX\maxsdk\include\soundobj.h"\
	"\3DSMAX\maxsdk\include\stack.h"\
	"\3DSMAX\maxsdk\include\stack3.h"\
	"\3dsmax\MAXSDK\INCLUDE\strbasic.h"\
	"\3DSMAX\maxsdk\include\strclass.h"\
	"\3DSMAX\maxsdk\include\tab.h"\
	"\3DSMAX\maxsdk\include\trig.h"\
	"\3DSMAX\maxsdk\include\triobj.h"\
	"\3DSMAX\maxsdk\include\units.h"\
	"\3DSMAX\maxsdk\include\utilexp.h"\
	"\3DSMAX\maxsdk\include\utillib.h"\
	"\3DSMAX\maxsdk\include\vedge.h"\
	"\3DSMAX\maxsdk\include\winutil.h"\
	
NODEP_CPP_DTSEX=\
	".\code\dtseres.h"\
	".\code\dtsexp.h"\
	".\code\SceneEnum.h"\
	

"$(INTDIR)\Dtsexp.obj" : $(SOURCE) $(DEP_CPP_DTSEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

DEP_CPP_DTSEX=\
	".\inc\dtsexp.h"\
	".\Inc\MeshBuilder.h"\
	".\inc\SceneEnum.h"\
	".\inc\ShapeBuilder.h"\
	"\3DSMAX\maxsdk\include\acolor.h"\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3DSMAX\maxsdk\include\assert1.h"\
	"\3DSMAX\maxsdk\include\bitarray.h"\
	"\3DSMAX\maxsdk\include\box2.h"\
	"\3DSMAX\maxsdk\include\box3.h"\
	"\3DSMAX\maxsdk\include\channels.h"\
	"\3DSMAX\maxsdk\include\cmdmode.h"\
	"\3DSMAX\maxsdk\include\color.h"\
	"\3DSMAX\maxsdk\include\control.h"\
	"\3DSMAX\maxsdk\include\coreexp.h"\
	"\3DSMAX\maxsdk\include\custcont.h"\
	"\3DSMAX\maxsdk\include\dbgprint.h"\
	"\3DSMAX\maxsdk\include\dpoint3.h"\
	"\3DSMAX\maxsdk\include\evuser.h"\
	"\3DSMAX\maxsdk\include\export.h"\
	"\3DSMAX\maxsdk\include\gencam.h"\
	"\3DSMAX\maxsdk\include\genhier.h"\
	"\3DSMAX\maxsdk\include\genlight.h"\
	"\3DSMAX\maxsdk\include\genshape.h"\
	"\3DSMAX\maxsdk\include\geom.h"\
	"\3DSMAX\maxsdk\include\geomlib.h"\
	"\3DSMAX\maxsdk\include\gfx.h"\
	"\3DSMAX\maxsdk\include\gfxlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\hitdata.h"\
	"\3DSMAX\maxsdk\include\hold.h"\
	"\3DSMAX\maxsdk\include\impapi.h"\
	"\3DSMAX\maxsdk\include\impexp.h"\
	"\3DSMAX\maxsdk\include\imtl.h"\
	"\3DSMAX\maxsdk\include\inode.h"\
	"\3DSMAX\maxsdk\include\interval.h"\
	"\3dsmax\MAXSDK\INCLUDE\ioapi.h"\
	"\3DSMAX\maxsdk\include\iparamb.h"\
	"\3DSMAX\maxsdk\include\ipoint2.h"\
	"\3DSMAX\maxsdk\include\ipoint3.h"\
	"\3DSMAX\maxsdk\include\lockid.h"\
	"\3DSMAX\maxsdk\include\matrix2.h"\
	"\3DSMAX\maxsdk\include\matrix3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Max.h"\
	"\3DSMAX\maxsdk\include\maxapi.h"\
	"\3DSMAX\maxsdk\include\maxcom.h"\
	"\3DSMAX\maxsdk\include\maxtypes.h"\
	"\3DSMAX\maxsdk\include\mesh.h"\
	"\3DSMAX\maxsdk\include\meshlib.h"\
	"\3DSMAX\maxsdk\include\mouseman.h"\
	"\3DSMAX\maxsdk\include\mtl.h"\
	"\3DSMAX\maxsdk\include\nametab.h"\
	"\3DSMAX\maxsdk\include\nurbs.h"\
	"\3DSMAX\maxsdk\include\nurbslib.h"\
	"\3DSMAX\maxsdk\include\nurbsobj.h"\
	"\3DSMAX\maxsdk\include\object.h"\
	"\3DSMAX\maxsdk\include\objmode.h"\
	"\3DSMAX\maxsdk\include\patch.h"\
	"\3DSMAX\maxsdk\include\patchlib.h"\
	"\3DSMAX\maxsdk\include\patchobj.h"\
	"\3DSMAX\maxsdk\include\plugapi.h"\
	"\3DSMAX\maxsdk\include\plugin.h"\
	"\3DSMAX\maxsdk\include\point2.h"\
	"\3DSMAX\maxsdk\include\point3.h"\
	"\3DSMAX\maxsdk\include\ptrvec.h"\
	"\3DSMAX\maxsdk\include\quat.h"\
	"\3DSMAX\maxsdk\include\ref.h"\
	"\3DSMAX\maxsdk\include\render.h"\
	"\3DSMAX\maxsdk\include\rtclick.h"\
	"\3DSMAX\maxsdk\include\sceneapi.h"\
	"\3DSMAX\maxsdk\include\snap.h"\
	"\3DSMAX\maxsdk\include\soundobj.h"\
	"\3DSMAX\maxsdk\include\stack.h"\
	"\3DSMAX\maxsdk\include\stack3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Stdmat.h"\
	"\3dsmax\MAXSDK\INCLUDE\strbasic.h"\
	"\3DSMAX\maxsdk\include\strclass.h"\
	"\3DSMAX\maxsdk\include\tab.h"\
	"\3DSMAX\maxsdk\include\trig.h"\
	"\3DSMAX\maxsdk\include\triobj.h"\
	"\3DSMAX\maxsdk\include\units.h"\
	"\3DSMAX\maxsdk\include\utilexp.h"\
	"\3DSMAX\maxsdk\include\utillib.h"\
	"\3DSMAX\maxsdk\include\vedge.h"\
	"\3DSMAX\maxsdk\include\winutil.h"\
	"\phenx\inc\base.h"\
	"\phenx\inc\blkStrm.h"\
	"\phenx\inc\d_defs.h"\
	"\phenx\inc\d_funcs.h"\
	"\phenx\inc\filedefs.h"\
	"\phenx\inc\filstrm.h"\
	"\phenx\inc\g_barray.h"\
	"\phenx\inc\g_bitmap.h"\
	"\phenx\inc\g_font.h"\
	"\phenx\inc\g_pal.h"\
	"\phenx\inc\g_poly.h"\
	"\phenx\inc\g_raster.h"\
	"\phenx\inc\g_surfac.h"\
	"\phenx\inc\lzhstrm.h"\
	"\phenx\inc\m_base.h"\
	"\phenx\inc\m_box.h"\
	"\phenx\inc\m_dist.h"\
	"\phenx\inc\m_dot.h"\
	"\phenx\inc\m_euler.h"\
	"\phenx\inc\m_lseg.h"\
	"\phenx\inc\m_mat2.h"\
	"\phenx\inc\m_mat3.h"\
	"\phenx\inc\m_mul.h"\
	"\phenx\inc\m_plane.h"\
	"\phenx\inc\m_plist.h"\
	"\phenx\inc\m_point.h"\
	"\phenx\inc\m_quat.h"\
	"\phenx\inc\m_random.h"\
	"\phenx\inc\m_rect.h"\
	"\phenx\inc\m_sphere.h"\
	"\phenx\inc\m_trig.h"\
	"\phenx\inc\memStrm.h"\
	"\phenx\inc\ml.h"\
	"\phenx\inc\p_txcach.h"\
	"\phenx\inc\persist.h"\
	"\phenx\inc\resManager.h"\
	"\phenx\inc\rlestrm.h"\
	"\phenx\inc\streamio.h"\
	"\phenx\inc\streams.h"\
	"\phenx\inc\talgorithm.h"\
	"\phenx\inc\tBitVector.h"\
	"\phenx\inc\threadBase.h"\
	"\phenx\inc\ts_camera.h"\
	"\phenx\inc\ts_CelAnimMesh.h"\
	"\phenx\inc\ts_light.h"\
	"\phenx\inc\ts_Material.h"\
	"\phenx\inc\ts_PointArray.h"\
	"\phenx\inc\ts_RenderContext.h"\
	"\phenx\inc\ts_RenderItem.h"\
	"\phenx\inc\ts_shape.h"\
	"\phenx\inc\ts_transform.h"\
	"\phenx\inc\ts_types.h"\
	"\phenx\inc\ts_vertex.h"\
	"\phenx\inc\tsorted.h"\
	"\phenx\inc\tVector.h"\
	"\phenx\inc\volstrm.h"\
	

"$(INTDIR)\Dtsexp.obj" : $(SOURCE) $(DEP_CPP_DTSEX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\res\dtsexp.rc

!IF  "$(CFG)" == "dtsexp - Win32 Release"


"$(INTDIR)\dtsexp.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/dtsexp.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"


"$(INTDIR)\dtsexp.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/dtsexp.res" /i "res" /d "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\3dsmax\maxsdk\lib\Util.lib

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\3dsmax\maxsdk\lib\Mesh.lib

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\3dsmax\maxsdk\lib\Geom.lib

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\3dsmax\maxsdk\lib\Core.lib

!IF  "$(CFG)" == "dtsexp - Win32 Release"

!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\MeshBuilder.cpp

!IF  "$(CFG)" == "dtsexp - Win32 Release"

NODEP_CPP_MESHB=\
	".\code\ShapeBuilder.h"\
	

"$(INTDIR)\MeshBuilder.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

DEP_CPP_MESHB=\
	".\Inc\MeshBuilder.h"\
	".\inc\ShapeBuilder.h"\
	"\3DSMAX\maxsdk\include\acolor.h"\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3DSMAX\maxsdk\include\assert1.h"\
	"\3DSMAX\maxsdk\include\bitarray.h"\
	"\3DSMAX\maxsdk\include\box2.h"\
	"\3DSMAX\maxsdk\include\box3.h"\
	"\3DSMAX\maxsdk\include\channels.h"\
	"\3DSMAX\maxsdk\include\cmdmode.h"\
	"\3DSMAX\maxsdk\include\color.h"\
	"\3DSMAX\maxsdk\include\control.h"\
	"\3DSMAX\maxsdk\include\coreexp.h"\
	"\3DSMAX\maxsdk\include\custcont.h"\
	"\3DSMAX\maxsdk\include\dbgprint.h"\
	"\3DSMAX\maxsdk\include\dpoint3.h"\
	"\3DSMAX\maxsdk\include\evuser.h"\
	"\3DSMAX\maxsdk\include\export.h"\
	"\3DSMAX\maxsdk\include\gencam.h"\
	"\3DSMAX\maxsdk\include\genhier.h"\
	"\3DSMAX\maxsdk\include\genlight.h"\
	"\3DSMAX\maxsdk\include\genshape.h"\
	"\3DSMAX\maxsdk\include\geom.h"\
	"\3DSMAX\maxsdk\include\geomlib.h"\
	"\3DSMAX\maxsdk\include\gfx.h"\
	"\3DSMAX\maxsdk\include\gfxlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\hitdata.h"\
	"\3DSMAX\maxsdk\include\hold.h"\
	"\3DSMAX\maxsdk\include\impapi.h"\
	"\3DSMAX\maxsdk\include\impexp.h"\
	"\3DSMAX\maxsdk\include\imtl.h"\
	"\3DSMAX\maxsdk\include\inode.h"\
	"\3DSMAX\maxsdk\include\interval.h"\
	"\3dsmax\MAXSDK\INCLUDE\ioapi.h"\
	"\3DSMAX\maxsdk\include\iparamb.h"\
	"\3DSMAX\maxsdk\include\ipoint2.h"\
	"\3DSMAX\maxsdk\include\ipoint3.h"\
	"\3DSMAX\maxsdk\include\lockid.h"\
	"\3DSMAX\maxsdk\include\matrix2.h"\
	"\3DSMAX\maxsdk\include\matrix3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Max.h"\
	"\3DSMAX\maxsdk\include\maxapi.h"\
	"\3DSMAX\maxsdk\include\maxcom.h"\
	"\3DSMAX\maxsdk\include\maxtypes.h"\
	"\3DSMAX\maxsdk\include\mesh.h"\
	"\3DSMAX\maxsdk\include\meshlib.h"\
	"\3DSMAX\maxsdk\include\mouseman.h"\
	"\3DSMAX\maxsdk\include\mtl.h"\
	"\3DSMAX\maxsdk\include\nametab.h"\
	"\3DSMAX\maxsdk\include\nurbs.h"\
	"\3DSMAX\maxsdk\include\nurbslib.h"\
	"\3DSMAX\maxsdk\include\nurbsobj.h"\
	"\3DSMAX\maxsdk\include\object.h"\
	"\3DSMAX\maxsdk\include\objmode.h"\
	"\3DSMAX\maxsdk\include\patch.h"\
	"\3DSMAX\maxsdk\include\patchlib.h"\
	"\3DSMAX\maxsdk\include\patchobj.h"\
	"\3DSMAX\maxsdk\include\plugapi.h"\
	"\3DSMAX\maxsdk\include\plugin.h"\
	"\3DSMAX\maxsdk\include\point2.h"\
	"\3DSMAX\maxsdk\include\point3.h"\
	"\3DSMAX\maxsdk\include\ptrvec.h"\
	"\3DSMAX\maxsdk\include\quat.h"\
	"\3DSMAX\maxsdk\include\ref.h"\
	"\3DSMAX\maxsdk\include\render.h"\
	"\3DSMAX\maxsdk\include\rtclick.h"\
	"\3DSMAX\maxsdk\include\sceneapi.h"\
	"\3DSMAX\maxsdk\include\snap.h"\
	"\3DSMAX\maxsdk\include\soundobj.h"\
	"\3DSMAX\maxsdk\include\stack.h"\
	"\3DSMAX\maxsdk\include\stack3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Stdmat.h"\
	"\3dsmax\MAXSDK\INCLUDE\strbasic.h"\
	"\3DSMAX\maxsdk\include\strclass.h"\
	"\3DSMAX\maxsdk\include\tab.h"\
	"\3DSMAX\maxsdk\include\trig.h"\
	"\3DSMAX\maxsdk\include\triobj.h"\
	"\3DSMAX\maxsdk\include\units.h"\
	"\3DSMAX\maxsdk\include\utilexp.h"\
	"\3DSMAX\maxsdk\include\utillib.h"\
	"\3DSMAX\maxsdk\include\vedge.h"\
	"\3DSMAX\maxsdk\include\winutil.h"\
	"\phenx\inc\base.h"\
	"\phenx\inc\blkStrm.h"\
	"\phenx\inc\d_defs.h"\
	"\phenx\inc\d_funcs.h"\
	"\phenx\inc\filedefs.h"\
	"\phenx\inc\filstrm.h"\
	"\phenx\inc\g_barray.h"\
	"\phenx\inc\g_bitmap.h"\
	"\phenx\inc\g_font.h"\
	"\phenx\inc\g_pal.h"\
	"\phenx\inc\g_poly.h"\
	"\phenx\inc\g_raster.h"\
	"\phenx\inc\g_surfac.h"\
	"\phenx\inc\lzhstrm.h"\
	"\phenx\inc\m_base.h"\
	"\phenx\inc\m_box.h"\
	"\phenx\inc\m_dist.h"\
	"\phenx\inc\m_dot.h"\
	"\phenx\inc\m_euler.h"\
	"\phenx\inc\m_lseg.h"\
	"\phenx\inc\m_mat2.h"\
	"\phenx\inc\m_mat3.h"\
	"\phenx\inc\m_mul.h"\
	"\phenx\inc\m_plane.h"\
	"\phenx\inc\m_plist.h"\
	"\phenx\inc\m_point.h"\
	"\phenx\inc\m_quat.h"\
	"\phenx\inc\m_random.h"\
	"\phenx\inc\m_rect.h"\
	"\phenx\inc\m_sphere.h"\
	"\phenx\inc\m_trig.h"\
	"\phenx\inc\memStrm.h"\
	"\phenx\inc\ml.h"\
	"\phenx\inc\p_txcach.h"\
	"\phenx\inc\persist.h"\
	"\phenx\inc\resManager.h"\
	"\phenx\inc\rlestrm.h"\
	"\phenx\inc\streamio.h"\
	"\phenx\inc\streams.h"\
	"\phenx\inc\talgorithm.h"\
	"\phenx\inc\tBitVector.h"\
	"\phenx\inc\threadBase.h"\
	"\phenx\inc\ts_camera.h"\
	"\phenx\inc\ts_CelAnimMesh.h"\
	"\phenx\inc\ts_light.h"\
	"\phenx\inc\ts_Material.h"\
	"\phenx\inc\ts_PointArray.h"\
	"\phenx\inc\ts_RenderContext.h"\
	"\phenx\inc\ts_RenderItem.h"\
	"\phenx\inc\ts_shape.h"\
	"\phenx\inc\ts_transform.h"\
	"\phenx\inc\ts_types.h"\
	"\phenx\inc\ts_vertex.h"\
	"\phenx\inc\tsorted.h"\
	"\phenx\inc\tVector.h"\
	"\phenx\inc\volstrm.h"\
	

"$(INTDIR)\MeshBuilder.obj" : $(SOURCE) $(DEP_CPP_MESHB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\ShapeBuilder.cpp

!IF  "$(CFG)" == "dtsexp - Win32 Release"

NODEP_CPP_SHAPE=\
	".\code\ShapeBuilder.h"\
	

"$(INTDIR)\ShapeBuilder.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

DEP_CPP_SHAPE=\
	".\Inc\MeshBuilder.h"\
	".\inc\ShapeBuilder.h"\
	"\3DSMAX\maxsdk\include\acolor.h"\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3DSMAX\maxsdk\include\assert1.h"\
	"\3DSMAX\maxsdk\include\bitarray.h"\
	"\3DSMAX\maxsdk\include\box2.h"\
	"\3DSMAX\maxsdk\include\box3.h"\
	"\3DSMAX\maxsdk\include\channels.h"\
	"\3DSMAX\maxsdk\include\cmdmode.h"\
	"\3DSMAX\maxsdk\include\color.h"\
	"\3DSMAX\maxsdk\include\control.h"\
	"\3DSMAX\maxsdk\include\coreexp.h"\
	"\3DSMAX\maxsdk\include\custcont.h"\
	"\3DSMAX\maxsdk\include\dbgprint.h"\
	"\3DSMAX\maxsdk\include\dpoint3.h"\
	"\3DSMAX\maxsdk\include\evuser.h"\
	"\3DSMAX\maxsdk\include\export.h"\
	"\3DSMAX\maxsdk\include\gencam.h"\
	"\3DSMAX\maxsdk\include\genhier.h"\
	"\3DSMAX\maxsdk\include\genlight.h"\
	"\3DSMAX\maxsdk\include\genshape.h"\
	"\3DSMAX\maxsdk\include\geom.h"\
	"\3DSMAX\maxsdk\include\geomlib.h"\
	"\3DSMAX\maxsdk\include\gfx.h"\
	"\3DSMAX\maxsdk\include\gfxlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\hitdata.h"\
	"\3DSMAX\maxsdk\include\hold.h"\
	"\3DSMAX\maxsdk\include\impapi.h"\
	"\3DSMAX\maxsdk\include\impexp.h"\
	"\3DSMAX\maxsdk\include\imtl.h"\
	"\3DSMAX\maxsdk\include\inode.h"\
	"\3DSMAX\maxsdk\include\interval.h"\
	"\3dsmax\MAXSDK\INCLUDE\ioapi.h"\
	"\3DSMAX\maxsdk\include\iparamb.h"\
	"\3DSMAX\maxsdk\include\ipoint2.h"\
	"\3DSMAX\maxsdk\include\ipoint3.h"\
	"\3DSMAX\maxsdk\include\lockid.h"\
	"\3DSMAX\maxsdk\include\matrix2.h"\
	"\3DSMAX\maxsdk\include\matrix3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Max.h"\
	"\3DSMAX\maxsdk\include\maxapi.h"\
	"\3DSMAX\maxsdk\include\maxcom.h"\
	"\3DSMAX\maxsdk\include\maxtypes.h"\
	"\3DSMAX\maxsdk\include\mesh.h"\
	"\3DSMAX\maxsdk\include\meshlib.h"\
	"\3DSMAX\maxsdk\include\mouseman.h"\
	"\3DSMAX\maxsdk\include\mtl.h"\
	"\3DSMAX\maxsdk\include\nametab.h"\
	"\3DSMAX\maxsdk\include\nurbs.h"\
	"\3DSMAX\maxsdk\include\nurbslib.h"\
	"\3DSMAX\maxsdk\include\nurbsobj.h"\
	"\3DSMAX\maxsdk\include\object.h"\
	"\3DSMAX\maxsdk\include\objmode.h"\
	"\3DSMAX\maxsdk\include\patch.h"\
	"\3DSMAX\maxsdk\include\patchlib.h"\
	"\3DSMAX\maxsdk\include\patchobj.h"\
	"\3DSMAX\maxsdk\include\plugapi.h"\
	"\3DSMAX\maxsdk\include\plugin.h"\
	"\3DSMAX\maxsdk\include\point2.h"\
	"\3DSMAX\maxsdk\include\point3.h"\
	"\3DSMAX\maxsdk\include\ptrvec.h"\
	"\3DSMAX\maxsdk\include\quat.h"\
	"\3DSMAX\maxsdk\include\ref.h"\
	"\3DSMAX\maxsdk\include\render.h"\
	"\3DSMAX\maxsdk\include\rtclick.h"\
	"\3DSMAX\maxsdk\include\sceneapi.h"\
	"\3DSMAX\maxsdk\include\snap.h"\
	"\3DSMAX\maxsdk\include\soundobj.h"\
	"\3DSMAX\maxsdk\include\stack.h"\
	"\3DSMAX\maxsdk\include\stack3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Stdmat.h"\
	"\3dsmax\MAXSDK\INCLUDE\strbasic.h"\
	"\3DSMAX\maxsdk\include\strclass.h"\
	"\3DSMAX\maxsdk\include\tab.h"\
	"\3DSMAX\maxsdk\include\trig.h"\
	"\3DSMAX\maxsdk\include\triobj.h"\
	"\3DSMAX\maxsdk\include\units.h"\
	"\3DSMAX\maxsdk\include\utilexp.h"\
	"\3DSMAX\maxsdk\include\utillib.h"\
	"\3DSMAX\maxsdk\include\vedge.h"\
	"\3DSMAX\maxsdk\include\winutil.h"\
	"\phenx\inc\base.h"\
	"\phenx\inc\blkStrm.h"\
	"\phenx\inc\d_defs.h"\
	"\phenx\inc\d_funcs.h"\
	"\phenx\inc\filedefs.h"\
	"\phenx\inc\filstrm.h"\
	"\phenx\inc\g_barray.h"\
	"\phenx\inc\g_bitmap.h"\
	"\phenx\inc\g_font.h"\
	"\phenx\inc\g_pal.h"\
	"\phenx\inc\g_poly.h"\
	"\phenx\inc\g_raster.h"\
	"\phenx\inc\g_surfac.h"\
	"\phenx\inc\lzhstrm.h"\
	"\phenx\inc\m_base.h"\
	"\phenx\inc\m_box.h"\
	"\phenx\inc\m_dist.h"\
	"\phenx\inc\m_dot.h"\
	"\phenx\inc\m_euler.h"\
	"\phenx\inc\m_lseg.h"\
	"\phenx\inc\m_mat2.h"\
	"\phenx\inc\m_mat3.h"\
	"\phenx\inc\m_mul.h"\
	"\phenx\inc\m_plane.h"\
	"\phenx\inc\m_plist.h"\
	"\phenx\inc\m_point.h"\
	"\phenx\inc\m_quat.h"\
	"\phenx\inc\m_random.h"\
	"\phenx\inc\m_rect.h"\
	"\phenx\inc\m_sphere.h"\
	"\phenx\inc\m_trig.h"\
	"\phenx\inc\memStrm.h"\
	"\phenx\inc\ml.h"\
	"\phenx\inc\p_txcach.h"\
	"\phenx\inc\persist.h"\
	"\phenx\inc\resManager.h"\
	"\phenx\inc\rlestrm.h"\
	"\phenx\inc\streamio.h"\
	"\phenx\inc\streams.h"\
	"\phenx\inc\talgorithm.h"\
	"\phenx\inc\tBitVector.h"\
	"\phenx\inc\threadBase.h"\
	"\phenx\inc\ts_camera.h"\
	"\phenx\inc\ts_CelAnimMesh.h"\
	"\phenx\inc\ts_light.h"\
	"\phenx\inc\ts_Material.h"\
	"\phenx\inc\ts_PointArray.h"\
	"\phenx\inc\ts_RenderContext.h"\
	"\phenx\inc\ts_RenderItem.h"\
	"\phenx\inc\ts_shape.h"\
	"\phenx\inc\ts_transform.h"\
	"\phenx\inc\ts_types.h"\
	"\phenx\inc\ts_vertex.h"\
	"\phenx\inc\tsorted.h"\
	"\phenx\inc\tVector.h"\
	"\phenx\inc\volstrm.h"\
	

"$(INTDIR)\ShapeBuilder.obj" : $(SOURCE) $(DEP_CPP_SHAPE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\code\SceneEnum.cpp

!IF  "$(CFG)" == "dtsexp - Win32 Release"

DEP_CPP_SCENE=\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3dsmax\MAXSDK\INCLUDE\decomp.h"\
	"\3dsmax\MAXSDK\INCLUDE\dummy.h"\
	"\3dsmax\MAXSDK\INCLUDE\istdplug.h"\
	"\3dsmax\MAXSDK\INCLUDE\modstack.h"\
	
NODEP_CPP_SCENE=\
	".\code\SceneEnum.h"\
	

"$(INTDIR)\SceneEnum.obj" : $(SOURCE) $(DEP_CPP_SCENE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "dtsexp - Win32 Debug"

DEP_CPP_SCENE=\
	".\Inc\MeshBuilder.h"\
	".\inc\SceneEnum.h"\
	".\inc\ShapeBuilder.h"\
	"\3DSMAX\maxsdk\include\acolor.h"\
	"\3dsmax\MAXSDK\INCLUDE\animtbl.h"\
	"\3DSMAX\maxsdk\include\assert1.h"\
	"\3DSMAX\maxsdk\include\bitarray.h"\
	"\3DSMAX\maxsdk\include\box2.h"\
	"\3DSMAX\maxsdk\include\box3.h"\
	"\3DSMAX\maxsdk\include\channels.h"\
	"\3DSMAX\maxsdk\include\cmdmode.h"\
	"\3DSMAX\maxsdk\include\color.h"\
	"\3DSMAX\maxsdk\include\control.h"\
	"\3DSMAX\maxsdk\include\coreexp.h"\
	"\3DSMAX\maxsdk\include\custcont.h"\
	"\3DSMAX\maxsdk\include\dbgprint.h"\
	"\3dsmax\MAXSDK\INCLUDE\decomp.h"\
	"\3DSMAX\maxsdk\include\dpoint3.h"\
	"\3dsmax\MAXSDK\INCLUDE\dummy.h"\
	"\3DSMAX\maxsdk\include\evuser.h"\
	"\3DSMAX\maxsdk\include\export.h"\
	"\3DSMAX\maxsdk\include\gencam.h"\
	"\3DSMAX\maxsdk\include\genhier.h"\
	"\3DSMAX\maxsdk\include\genlight.h"\
	"\3DSMAX\maxsdk\include\genshape.h"\
	"\3DSMAX\maxsdk\include\geom.h"\
	"\3DSMAX\maxsdk\include\geomlib.h"\
	"\3DSMAX\maxsdk\include\gfx.h"\
	"\3DSMAX\maxsdk\include\gfxlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\hitdata.h"\
	"\3DSMAX\maxsdk\include\hold.h"\
	"\3DSMAX\maxsdk\include\impapi.h"\
	"\3DSMAX\maxsdk\include\impexp.h"\
	"\3DSMAX\maxsdk\include\imtl.h"\
	"\3DSMAX\maxsdk\include\inode.h"\
	"\3DSMAX\maxsdk\include\interval.h"\
	"\3dsmax\MAXSDK\INCLUDE\ioapi.h"\
	"\3DSMAX\maxsdk\include\iparamb.h"\
	"\3DSMAX\maxsdk\include\ipoint2.h"\
	"\3DSMAX\maxsdk\include\ipoint3.h"\
	"\3dsmax\MAXSDK\INCLUDE\istdplug.h"\
	"\3DSMAX\maxsdk\include\lockid.h"\
	"\3DSMAX\maxsdk\include\matrix2.h"\
	"\3DSMAX\maxsdk\include\matrix3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Max.h"\
	"\3DSMAX\maxsdk\include\maxapi.h"\
	"\3DSMAX\maxsdk\include\maxcom.h"\
	"\3DSMAX\maxsdk\include\maxtypes.h"\
	"\3DSMAX\maxsdk\include\mesh.h"\
	"\3DSMAX\maxsdk\include\meshlib.h"\
	"\3dsmax\MAXSDK\INCLUDE\modstack.h"\
	"\3DSMAX\maxsdk\include\mouseman.h"\
	"\3DSMAX\maxsdk\include\mtl.h"\
	"\3DSMAX\maxsdk\include\nametab.h"\
	"\3DSMAX\maxsdk\include\nurbs.h"\
	"\3DSMAX\maxsdk\include\nurbslib.h"\
	"\3DSMAX\maxsdk\include\nurbsobj.h"\
	"\3DSMAX\maxsdk\include\object.h"\
	"\3DSMAX\maxsdk\include\objmode.h"\
	"\3DSMAX\maxsdk\include\patch.h"\
	"\3DSMAX\maxsdk\include\patchlib.h"\
	"\3DSMAX\maxsdk\include\patchobj.h"\
	"\3DSMAX\maxsdk\include\plugapi.h"\
	"\3DSMAX\maxsdk\include\plugin.h"\
	"\3DSMAX\maxsdk\include\point2.h"\
	"\3DSMAX\maxsdk\include\point3.h"\
	"\3DSMAX\maxsdk\include\ptrvec.h"\
	"\3DSMAX\maxsdk\include\quat.h"\
	"\3DSMAX\maxsdk\include\ref.h"\
	"\3DSMAX\maxsdk\include\render.h"\
	"\3DSMAX\maxsdk\include\rtclick.h"\
	"\3DSMAX\maxsdk\include\sceneapi.h"\
	"\3DSMAX\maxsdk\include\snap.h"\
	"\3DSMAX\maxsdk\include\soundobj.h"\
	"\3DSMAX\maxsdk\include\stack.h"\
	"\3DSMAX\maxsdk\include\stack3.h"\
	"\3dsmax\MAXSDK\INCLUDE\Stdmat.h"\
	"\3dsmax\MAXSDK\INCLUDE\strbasic.h"\
	"\3DSMAX\maxsdk\include\strclass.h"\
	"\3DSMAX\maxsdk\include\tab.h"\
	"\3DSMAX\maxsdk\include\trig.h"\
	"\3DSMAX\maxsdk\include\triobj.h"\
	"\3DSMAX\maxsdk\include\units.h"\
	"\3DSMAX\maxsdk\include\utilexp.h"\
	"\3DSMAX\maxsdk\include\utillib.h"\
	"\3DSMAX\maxsdk\include\vedge.h"\
	"\3DSMAX\maxsdk\include\winutil.h"\
	"\phenx\inc\base.h"\
	"\phenx\inc\blkStrm.h"\
	"\phenx\inc\d_defs.h"\
	"\phenx\inc\d_funcs.h"\
	"\phenx\inc\filedefs.h"\
	"\phenx\inc\filstrm.h"\
	"\phenx\inc\g_barray.h"\
	"\phenx\inc\g_bitmap.h"\
	"\phenx\inc\g_font.h"\
	"\phenx\inc\g_pal.h"\
	"\phenx\inc\g_poly.h"\
	"\phenx\inc\g_raster.h"\
	"\phenx\inc\g_surfac.h"\
	"\phenx\inc\lzhstrm.h"\
	"\phenx\inc\m_base.h"\
	"\phenx\inc\m_box.h"\
	"\phenx\inc\m_dist.h"\
	"\phenx\inc\m_dot.h"\
	"\phenx\inc\m_euler.h"\
	"\phenx\inc\m_lseg.h"\
	"\phenx\inc\m_mat2.h"\
	"\phenx\inc\m_mat3.h"\
	"\phenx\inc\m_mul.h"\
	"\phenx\inc\m_plane.h"\
	"\phenx\inc\m_plist.h"\
	"\phenx\inc\m_point.h"\
	"\phenx\inc\m_quat.h"\
	"\phenx\inc\m_random.h"\
	"\phenx\inc\m_rect.h"\
	"\phenx\inc\m_sphere.h"\
	"\phenx\inc\m_trig.h"\
	"\phenx\inc\memStrm.h"\
	"\phenx\inc\ml.h"\
	"\phenx\inc\p_txcach.h"\
	"\phenx\inc\persist.h"\
	"\phenx\inc\resManager.h"\
	"\phenx\inc\rlestrm.h"\
	"\phenx\inc\streamio.h"\
	"\phenx\inc\streams.h"\
	"\phenx\inc\talgorithm.h"\
	"\phenx\inc\tBitVector.h"\
	"\phenx\inc\threadBase.h"\
	"\phenx\inc\ts_camera.h"\
	"\phenx\inc\ts_CelAnimMesh.h"\
	"\phenx\inc\ts_light.h"\
	"\phenx\inc\ts_Material.h"\
	"\phenx\inc\ts_PointArray.h"\
	"\phenx\inc\ts_RenderContext.h"\
	"\phenx\inc\ts_RenderItem.h"\
	"\phenx\inc\ts_shape.h"\
	"\phenx\inc\ts_transform.h"\
	"\phenx\inc\ts_types.h"\
	"\phenx\inc\ts_vertex.h"\
	"\phenx\inc\tsorted.h"\
	"\phenx\inc\tVector.h"\
	"\phenx\inc\volstrm.h"\
	

"$(INTDIR)\SceneEnum.obj" : $(SOURCE) $(DEP_CPP_SCENE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Code\PERSIST.CPP
DEP_CPP_PERSI=\
	"\phenx\inc\base.h"\
	"\phenx\inc\blkStrm.h"\
	"\phenx\inc\filedefs.h"\
	"\phenx\inc\filstrm.h"\
	"\phenx\inc\memStrm.h"\
	"\phenx\inc\persist.h"\
	"\phenx\inc\streamio.h"\
	

"$(INTDIR)\PERSIST.OBJ" : $(SOURCE) $(DEP_CPP_PERSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################

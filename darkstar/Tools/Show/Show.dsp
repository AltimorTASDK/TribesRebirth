# Microsoft Developer Studio Project File - Name="Show" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Show - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Show.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Show.mak" CFG="Show - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Show - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Show - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/Tools/Show", UXIAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Show - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Exe"
# PROP Intermediate_Dir "Obj/RMObj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /Gr /Zp4 /MD /W3 /GR /Ot /Ow /Og /Oi /Ob1 /I "inc" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D _USERENTRY=__cdecl /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib shell32.lib advapi32.lib uuid.lib wsock32.lib rpcrt4.lib vfw32.lib winmm.lib rmCore.lib rmGW.lib rmSim.lib rmSimObjects.lib rmSimGui.lib rmgrd.lib rmls.lib rmitr.lib rmts3.lib rmgfxio.lib rmml.lib rmted.lib rmdNet.lib rmSimNet.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC" /out:"Exe/RMShow.exe" /libpath:"$(PHOENIXLIB)"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Exe"
# PROP Intermediate_Dir "Obj/DMObj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W3 /Gm /GR /GX /Zi /Od /I "inc" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D _USERENTRY=__cdecl /D "DEBUG" /D "_DEBUG" /YX /Fd"Exe/Show.pdb" /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib shell32.lib advapi32.lib uuid.lib wsock32.lib rpcrt4.lib vfw32.lib winmm.lib dmCore.lib dmGW.lib dmSim.lib dmSimObjects.lib dmSimGui.lib dmgrd.lib dmls.lib dmitr.lib dmts3.lib dmgfxio.lib dmml.lib dmted.lib dmdNet.lib dmSimNet.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /out:"\work\max\show\DMShow.exe" /libpath:"$(PHOENIXLIB)"
# SUBTRACT LINK32 /verbose /incremental:no /nodefaultlib /force

!ENDIF 

# Begin Target

# Name "Show - Win32 Release"
# Name "Show - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\Code\CamDialog.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_CAMDI=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	"..\..\..\..\phenx\inc\wintools.h"\
	".\Inc\CamDialog.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\Main.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_MAIN_=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\d_defs.h"\
	"..\..\..\..\phenx\inc\d_funcs.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\fn_all.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_raster.h"\
	"..\..\..\..\phenx\inc\g_surfac.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\gfxPlugin.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\GWTreeView.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\interiorPlugin.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\LSPlugin.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simGuiBase.h"\
	"..\..\..\..\phenx\inc\simGuiCanvas.h"\
	"..\..\..\..\phenx\inc\simGuiPlugin.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\simInputPlugin.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simtoolwin.h"\
	"..\..\..\..\phenx\inc\simTreePlugin.h"\
	"..\..\..\..\phenx\inc\SimTreeView.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\skyPlugin.h"\
	"..\..\..\..\phenx\inc\soundFXPlugin.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\terrainPlugin.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\toolPlugin.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	"..\..\..\..\phenx\inc\wintools.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\ShowPlugin.h"\
	".\Inc\STD.H"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\Mover.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_MOVER=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	".\Inc\Mover.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\MyGuy.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_MYGUY=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simPersman.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\ShapeDialog.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_SHAPE=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	"..\..\..\..\phenx\inc\wintools.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\ShapeDialog.h"\
	".\Inc\ShowPlugin.h"\
	".\Inc\TransitionDialog.h"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\Show.rc
# End Source File
# Begin Source File

SOURCE=.\Code\ShowPlugin.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_SHOWP=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simGuiBase.h"\
	"..\..\..\..\phenx\inc\simGuiCanvas.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	"..\..\..\..\phenx\inc\wintools.h"\
	".\Inc\CamDialog.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\ShapeDialog.h"\
	".\Inc\ShowPlugin.h"\
	".\Inc\TransitionDialog.h"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\TransitionDialog.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_TRANS=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	"..\..\..\..\phenx\inc\wintools.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\ShapeDialog.h"\
	".\Inc\ShowPlugin.h"\
	".\Inc\TransitionDialog.h"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\Tripod.cpp

!IF  "$(CFG)" == "Show - Win32 Release"

!ELSEIF  "$(CFG)" == "Show - Win32 Debug"

DEP_CPP_TRIPO=\
	"..\..\..\..\phenx\inc\base.h"\
	"..\..\..\..\phenx\inc\bitset.h"\
	"..\..\..\..\phenx\inc\bitstream.h"\
	"..\..\..\..\phenx\inc\blkstrm.h"\
	"..\..\..\..\phenx\inc\console.h"\
	"..\..\..\..\phenx\inc\core.h"\
	"..\..\..\..\phenx\inc\dInput.h"\
	"..\..\..\..\phenx\inc\filedefs.h"\
	"..\..\..\..\phenx\inc\filstrm.h"\
	"..\..\..\..\phenx\inc\g_barray.h"\
	"..\..\..\..\phenx\inc\g_font.h"\
	"..\..\..\..\phenx\inc\g_types.h"\
	"..\..\..\..\phenx\inc\gdevice.h"\
	"..\..\..\..\phenx\inc\gdmanag.h"\
	"..\..\..\..\phenx\inc\genericActions.h"\
	"..\..\..\..\phenx\inc\grd.h"\
	"..\..\..\..\phenx\inc\grdBlock.h"\
	"..\..\..\..\phenx\inc\grdEdgeTable.h"\
	"..\..\..\..\phenx\inc\grdFile.h"\
	"..\..\..\..\phenx\inc\grdHeight.h"\
	"..\..\..\..\phenx\inc\grdMetrics.h"\
	"..\..\..\..\phenx\inc\grdRange.h"\
	"..\..\..\..\phenx\inc\grdRender.h"\
	"..\..\..\..\phenx\inc\gw.h"\
	"..\..\..\..\phenx\inc\gwBase.h"\
	"..\..\..\..\phenx\inc\gwcanvas.h"\
	"..\..\..\..\phenx\inc\gwDialog.h"\
	"..\..\..\..\phenx\inc\gwMain.h"\
	"..\..\..\..\phenx\inc\gwMenu.h"\
	"..\..\..\..\phenx\inc\gwMsgMap.h"\
	"..\..\..\..\phenx\inc\gwMsgs.h"\
	"..\..\..\..\phenx\inc\gwTool.h"\
	"..\..\..\..\phenx\inc\gwwinsx.h"\
	"..\..\..\..\phenx\inc\inspect.h"\
	"..\..\..\..\phenx\inc\itrbit.h"\
	"..\..\..\..\phenx\inc\itrcollision.h"\
	"..\..\..\..\phenx\inc\itrgeometry.h"\
	"..\..\..\..\phenx\inc\itrinstance.h"\
	"..\..\..\..\phenx\inc\itrlighting.h"\
	"..\..\..\..\phenx\inc\itrmetrics.h"\
	"..\..\..\..\phenx\inc\itrrender.h"\
	"..\..\..\..\phenx\inc\itrShape.h"\
	"..\..\..\..\phenx\inc\lock.h"\
	"..\..\..\..\phenx\inc\lzhstrm.h"\
	"..\..\..\..\phenx\inc\m_base.h"\
	"..\..\..\..\phenx\inc\m_box.h"\
	"..\..\..\..\phenx\inc\m_collision.h"\
	"..\..\..\..\phenx\inc\m_dist.h"\
	"..\..\..\..\phenx\inc\m_dot.h"\
	"..\..\..\..\phenx\inc\m_euler.h"\
	"..\..\..\..\phenx\inc\m_lseg.h"\
	"..\..\..\..\phenx\inc\m_mat2.h"\
	"..\..\..\..\phenx\inc\m_mat3.h"\
	"..\..\..\..\phenx\inc\m_mul.h"\
	"..\..\..\..\phenx\inc\m_plane.h"\
	"..\..\..\..\phenx\inc\m_plist.h"\
	"..\..\..\..\phenx\inc\m_point.h"\
	"..\..\..\..\phenx\inc\m_qsort.h"\
	"..\..\..\..\phenx\inc\m_quat.h"\
	"..\..\..\..\phenx\inc\m_random.h"\
	"..\..\..\..\phenx\inc\m_rect.h"\
	"..\..\..\..\phenx\inc\m_sphere.h"\
	"..\..\..\..\phenx\inc\m_trig.h"\
	"..\..\..\..\phenx\inc\memstrm.h"\
	"..\..\..\..\phenx\inc\ml.h"\
	"..\..\..\..\phenx\inc\move.h"\
	"..\..\..\..\phenx\inc\persist.h"\
	"..\..\..\..\phenx\inc\ResManager.h"\
	"..\..\..\..\phenx\inc\rlestrm.h"\
	"..\..\..\..\phenx\inc\sim.h"\
	"..\..\..\..\phenx\inc\sim3Dev.h"\
	"..\..\..\..\phenx\inc\simAction.h"\
	"..\..\..\..\phenx\inc\simBase.h"\
	"..\..\..\..\phenx\inc\simCanvas.h"\
	"..\..\..\..\phenx\inc\simCollision.h"\
	"..\..\..\..\phenx\inc\simCollisionImages.h"\
	"..\..\..\..\phenx\inc\simConsoleCanvas.h"\
	"..\..\..\..\phenx\inc\simContainer.h"\
	"..\..\..\..\phenx\inc\simCoordDb.h"\
	"..\..\..\..\phenx\inc\simDictionary.h"\
	"..\..\..\..\phenx\inc\simEv.h"\
	"..\..\..\..\phenx\inc\simEvDcl.h"\
	"..\..\..\..\phenx\inc\simGame.h"\
	"..\..\..\..\phenx\inc\simInput.h"\
	"..\..\..\..\phenx\inc\simInputDevice.h"\
	"..\..\..\..\phenx\inc\SimInterior.h"\
	"..\..\..\..\phenx\inc\simNetObject.h"\
	"..\..\..\..\phenx\inc\simObjectTypes.h"\
	"..\..\..\..\phenx\inc\simPersman.h"\
	"..\..\..\..\phenx\inc\simRenderGrp.h"\
	"..\..\..\..\phenx\inc\simResource.h"\
	"..\..\..\..\phenx\inc\simTagDictionary.h"\
	"..\..\..\..\phenx\inc\SimTerrain.h"\
	"..\..\..\..\phenx\inc\simTimerGrp.h"\
	"..\..\..\..\phenx\inc\simTSViewport.h"\
	"..\..\..\..\phenx\inc\simWorld.h"\
	"..\..\..\..\phenx\inc\streamio.h"\
	"..\..\..\..\phenx\inc\streams.h"\
	"..\..\..\..\phenx\inc\tagDictionary.h"\
	"..\..\..\..\phenx\inc\talgorithm.h"\
	"..\..\..\..\phenx\inc\threadBase.h"\
	"..\..\..\..\phenx\inc\timer.h"\
	"..\..\..\..\phenx\inc\tmap.h"\
	"..\..\..\..\phenx\inc\tMHashMap.h"\
	"..\..\..\..\phenx\inc\tplane.h"\
	"..\..\..\..\phenx\inc\ts.h"\
	"..\..\..\..\phenx\inc\ts_camera.h"\
	"..\..\..\..\phenx\inc\ts_celanimmesh.h"\
	"..\..\..\..\phenx\inc\ts_light.h"\
	"..\..\..\..\phenx\inc\ts_Material.h"\
	"..\..\..\..\phenx\inc\ts_plane.h"\
	"..\..\..\..\phenx\inc\ts_PointArray.h"\
	"..\..\..\..\phenx\inc\ts_poly.h"\
	"..\..\..\..\phenx\inc\ts_rendercontext.h"\
	"..\..\..\..\phenx\inc\ts_RenderItem.h"\
	"..\..\..\..\phenx\inc\ts_shadow.h"\
	"..\..\..\..\phenx\inc\ts_shape.h"\
	"..\..\..\..\phenx\inc\ts_shapeinst.h"\
	"..\..\..\..\phenx\inc\ts_transform.h"\
	"..\..\..\..\phenx\inc\ts_types.h"\
	"..\..\..\..\phenx\inc\ts_vertex.h"\
	"..\..\..\..\phenx\inc\tsorted.h"\
	"..\..\..\..\phenx\inc\tstring.h"\
	"..\..\..\..\phenx\inc\tVector.h"\
	"..\..\..\..\phenx\inc\viewcone.h"\
	"..\..\..\..\phenx\inc\volstrm.h"\
	".\Inc\Mover.h"\
	".\Inc\MyGuy.h"\
	".\Inc\Tripod.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\Inc\CamDialog.h
# End Source File
# Begin Source File

SOURCE=.\Inc\Mover.h
# End Source File
# Begin Source File

SOURCE=.\Inc\MyGuy.h
# End Source File
# Begin Source File

SOURCE=.\Inc\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Inc\ShapeDialog.h
# End Source File
# Begin Source File

SOURCE=.\Inc\ShowPlugin.h
# End Source File
# Begin Source File

SOURCE=.\Inc\STD.H
# End Source File
# Begin Source File

SOURCE=.\Inc\TransitionDialog.h
# End Source File
# Begin Source File

SOURCE=.\Inc\Tripod.h
# End Source File
# End Group
# End Target
# End Project

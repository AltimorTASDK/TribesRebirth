# Microsoft Developer Studio Project File - Name="DTSExporter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DTSExporter - Win32 Biped Exporter
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DTSExporter.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DTSExporter.mak" CFG="DTSExporter - Win32 Biped Exporter"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DTSExporter - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DTSExporter - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DTSExporter - Win32 Debug Pre Compress" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DTSExporter - Win32 Vicon Compressing Exporter" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DTSExporter - Win32 Biped Exporter" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

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
# ADD CPP /nologo /G5 /Zp4 /MD /W3 /GR /Ot /Ow /Og /Oi /Ob1 /I "inc" /I "C:\Program Files\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 comctl32.lib core.lib rmCore.lib rmML.lib util.lib rmTS3.lib rmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"C:\Program Files\3DSMax2\Plugins\DTSExporter.dlo" /libpath:"C:\Program Files\3DSMax2\MaxSDK\Lib" /libpath:"$(PHOENIXLIB)"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

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
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "$(MaxDir)\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /out:"D:\3DSMax2.5\Plugins\ModifiedExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"$(MaxDir)\MaxSDK\Lib"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTSExpor"
# PROP BASE Intermediate_Dir "DTSExpor"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTSExpor"
# PROP Intermediate_Dir "DTSExpor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "C:\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /YX /FD /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "C:\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /D VICON_EXPORT_BEHAVIOR=1 /D _VICON_EXP_MODE=1 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /out:"C:\3DSMax2\Plugins\ModifiedExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"C:\3DSMax2\MaxSDK\Lib"
# ADD LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /def:".\Code\ModifiedExporter.def" /out:"C:\3DSMax2\Plugins\ModifiedExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"C:\3DSMax2\MaxSDK\Lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTSExpo0"
# PROP BASE Intermediate_Dir "DTSExpo0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTSExpo0"
# PROP Intermediate_Dir "DTSExpo0"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "C:\3DSMax2\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /D VICON_EXPORT_BEHAVIOR=1 /D _VICON_EXP_MODE=1 /YX /FD /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "$(MaxDir)\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /D VICON_EXPORT_BEHAVIOR=2 /D _VICON_EXP_MODE=2 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /out:"C:\3DSMax2\Plugins\ModifiedExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"C:\3DSMax2\MaxSDK\Lib"
# ADD LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /def:".\Code\ViconCombiningExporter.def" /out:"D:\3DSMax2.5\Plugins\ViconCombiningExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"$(MaxDir)\MaxSDK\Lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DTSExpo1"
# PROP BASE Intermediate_Dir "DTSExpo1"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DTSExpo1"
# PROP Intermediate_Dir "DTSExpo1"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "$(MaxDir)\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /D VICON_EXPORT_BEHAVIOR=2 /D _VICON_EXP_MODE=2 /YX /FD /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /Zi /Od /I "inc" /I "$(MaxDir)\MaxSDK\Include" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D "WINDOWS" /D EXPORT= /D _USERENTRY=__cdecl /D "MAX_2" /D "_DEBUG" /D "SDUMP" /D VICON_EXPORT_BEHAVIOR=2 /D _VICON_EXP_MODE=2 /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
# ADD RSC /l 0x409 /i "inc" /i "../inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /def:".\Code\ViconCombiningExporter.def" /out:"D:\3DSMax2.5\Plugins\ViconCombiningExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"$(MaxDir)\MaxSDK\Lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib core.lib dmCore.lib dmConsole.lib dmML.lib util.lib dmTS3.lib dmGFXIO.lib mesh.lib geom.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /nodefaultlib:"LIBCMTD" /def:".\Code\ViconCombiningExporter.def" /out:"D:\3DSMax2.5\Plugins\ViconCombiningExporter.DLO" /pdbtype:sept /libpath:"$(PHOENIXLIB)" /libpath:"$(MaxDir)\MaxSDK\Lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DTSExporter - Win32 Release"
# Name "DTSExporter - Win32 Debug"
# Name "DTSExporter - Win32 Debug Pre Compress"
# Name "DTSExporter - Win32 Vicon Compressing Exporter"
# Name "DTSExporter - Win32 Biped Exporter"
# Begin Group "Source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\Code\3DSMTL.cpp
DEP_CPP_3DSMT=\
	".\Inc\3DSIMP.H"\
	".\Inc\3DSShape.h"\
	".\Inc\CFILE.H"\
	".\Inc\KFIO.H"\
	".\Inc\MTLDef.H"\
	".\Inc\OFILE.H"\
	
NODEP_CPP_3DSMT=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\captypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dummy.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gamma.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\polyshp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\shape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\shphier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\shpsels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\spline3d.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\splshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\DTSExporter.cpp
DEP_CPP_DTSEX=\
	".\Inc\DTSExporter.h"\
	".\Inc\MeshBuilder.h"\
	".\Inc\SceneEnum.h"\
	".\Inc\ShapeBuilder.h"\
	
NODEP_CPP_DTSEX=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stdmat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	"..\..\..\phoenix\inc\base.h"\
	"..\..\..\phoenix\inc\bitstream.h"\
	"..\..\..\phoenix\inc\blkstrm.h"\
	"..\..\..\phoenix\inc\filedefs.h"\
	"..\..\..\phoenix\inc\filstrm.h"\
	"..\..\..\phoenix\inc\lock.h"\
	"..\..\..\phoenix\inc\lzhstrm.h"\
	"..\..\..\phoenix\inc\m_base.h"\
	"..\..\..\phoenix\inc\m_box.h"\
	"..\..\..\phoenix\inc\m_collision.h"\
	"..\..\..\phoenix\inc\m_dist.h"\
	"..\..\..\phoenix\inc\m_dot.h"\
	"..\..\..\phoenix\inc\m_euler.h"\
	"..\..\..\phoenix\inc\m_lseg.h"\
	"..\..\..\phoenix\inc\m_mat2.h"\
	"..\..\..\phoenix\inc\m_mat3.h"\
	"..\..\..\phoenix\inc\m_mul.h"\
	"..\..\..\phoenix\inc\m_plane.h"\
	"..\..\..\phoenix\inc\m_plist.h"\
	"..\..\..\phoenix\inc\m_point.h"\
	"..\..\..\phoenix\inc\m_qsort.h"\
	"..\..\..\phoenix\inc\m_quat.h"\
	"..\..\..\phoenix\inc\m_random.h"\
	"..\..\..\phoenix\inc\m_rect.h"\
	"..\..\..\phoenix\inc\m_sphere.h"\
	"..\..\..\phoenix\inc\m_trig.h"\
	"..\..\..\phoenix\inc\memstrm.h"\
	"..\..\..\phoenix\inc\ml.h"\
	"..\..\..\phoenix\inc\persist.h"\
	"..\..\..\phoenix\inc\resmanager.h"\
	"..\..\..\phoenix\inc\rlestrm.h"\
	"..\..\..\phoenix\inc\streamio.h"\
	"..\..\..\phoenix\inc\streams.h"\
	"..\..\..\phoenix\inc\talgorithm.h"\
	"..\..\..\phoenix\inc\threadbase.h"\
	"..\..\..\phoenix\inc\ts_camera.h"\
	"..\..\..\phoenix\inc\ts_celanimmesh.h"\
	"..\..\..\phoenix\inc\ts_material.h"\
	"..\..\..\phoenix\inc\ts_pointarray.h"\
	"..\..\..\phoenix\inc\ts_rendercontext.h"\
	"..\..\..\phoenix\inc\ts_renderitem.h"\
	"..\..\..\phoenix\inc\ts_shape.h"\
	"..\..\..\phoenix\inc\ts_transform.h"\
	"..\..\..\phoenix\inc\ts_types.h"\
	"..\..\..\phoenix\inc\ts_vertex.h"\
	"..\..\..\phoenix\inc\tsorted.h"\
	"..\..\..\phoenix\inc\tvector.h"\
	"..\..\..\phoenix\inc\volstrm.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\DTSExporter.def

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\MeshBuilder.cpp
DEP_CPP_MESHB=\
	".\Inc\MeshBuilder.h"\
	".\Inc\ShapeBuilder.h"\
	
NODEP_CPP_MESHB=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stdmat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	"..\..\..\phoenix\inc\base.h"\
	"..\..\..\phoenix\inc\bitstream.h"\
	"..\..\..\phoenix\inc\blkstrm.h"\
	"..\..\..\phoenix\inc\filedefs.h"\
	"..\..\..\phoenix\inc\filstrm.h"\
	"..\..\..\phoenix\inc\lock.h"\
	"..\..\..\phoenix\inc\lzhstrm.h"\
	"..\..\..\phoenix\inc\m_base.h"\
	"..\..\..\phoenix\inc\m_box.h"\
	"..\..\..\phoenix\inc\m_collision.h"\
	"..\..\..\phoenix\inc\m_dist.h"\
	"..\..\..\phoenix\inc\m_dot.h"\
	"..\..\..\phoenix\inc\m_euler.h"\
	"..\..\..\phoenix\inc\m_lseg.h"\
	"..\..\..\phoenix\inc\m_mat2.h"\
	"..\..\..\phoenix\inc\m_mat3.h"\
	"..\..\..\phoenix\inc\m_mul.h"\
	"..\..\..\phoenix\inc\m_plane.h"\
	"..\..\..\phoenix\inc\m_plist.h"\
	"..\..\..\phoenix\inc\m_point.h"\
	"..\..\..\phoenix\inc\m_qsort.h"\
	"..\..\..\phoenix\inc\m_quat.h"\
	"..\..\..\phoenix\inc\m_random.h"\
	"..\..\..\phoenix\inc\m_rect.h"\
	"..\..\..\phoenix\inc\m_sphere.h"\
	"..\..\..\phoenix\inc\m_trig.h"\
	"..\..\..\phoenix\inc\memstrm.h"\
	"..\..\..\phoenix\inc\ml.h"\
	"..\..\..\phoenix\inc\persist.h"\
	"..\..\..\phoenix\inc\resmanager.h"\
	"..\..\..\phoenix\inc\rlestrm.h"\
	"..\..\..\phoenix\inc\streamio.h"\
	"..\..\..\phoenix\inc\streams.h"\
	"..\..\..\phoenix\inc\talgorithm.h"\
	"..\..\..\phoenix\inc\threadbase.h"\
	"..\..\..\phoenix\inc\ts_camera.h"\
	"..\..\..\phoenix\inc\ts_celanimmesh.h"\
	"..\..\..\phoenix\inc\ts_material.h"\
	"..\..\..\phoenix\inc\ts_pointarray.h"\
	"..\..\..\phoenix\inc\ts_rendercontext.h"\
	"..\..\..\phoenix\inc\ts_renderitem.h"\
	"..\..\..\phoenix\inc\ts_shape.h"\
	"..\..\..\phoenix\inc\ts_transform.h"\
	"..\..\..\phoenix\inc\ts_types.h"\
	"..\..\..\phoenix\inc\ts_vertex.h"\
	"..\..\..\phoenix\inc\tsorted.h"\
	"..\..\..\phoenix\inc\tvector.h"\
	"..\..\..\phoenix\inc\volstrm.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\ObList.cpp
DEP_CPP_OBLIS=\
	".\Inc\MeshBuilder.h"\
	".\Inc\ObList.h"\
	".\Inc\SceneEnum.h"\
	".\Inc\ShapeBuilder.h"\
	
NODEP_CPP_OBLIS=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stdmat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	"..\..\..\phoenix\inc\base.h"\
	"..\..\..\phoenix\inc\bitstream.h"\
	"..\..\..\phoenix\inc\blkstrm.h"\
	"..\..\..\phoenix\inc\filedefs.h"\
	"..\..\..\phoenix\inc\filstrm.h"\
	"..\..\..\phoenix\inc\lock.h"\
	"..\..\..\phoenix\inc\lzhstrm.h"\
	"..\..\..\phoenix\inc\m_base.h"\
	"..\..\..\phoenix\inc\m_box.h"\
	"..\..\..\phoenix\inc\m_collision.h"\
	"..\..\..\phoenix\inc\m_dist.h"\
	"..\..\..\phoenix\inc\m_dot.h"\
	"..\..\..\phoenix\inc\m_euler.h"\
	"..\..\..\phoenix\inc\m_lseg.h"\
	"..\..\..\phoenix\inc\m_mat2.h"\
	"..\..\..\phoenix\inc\m_mat3.h"\
	"..\..\..\phoenix\inc\m_mul.h"\
	"..\..\..\phoenix\inc\m_plane.h"\
	"..\..\..\phoenix\inc\m_plist.h"\
	"..\..\..\phoenix\inc\m_point.h"\
	"..\..\..\phoenix\inc\m_qsort.h"\
	"..\..\..\phoenix\inc\m_quat.h"\
	"..\..\..\phoenix\inc\m_random.h"\
	"..\..\..\phoenix\inc\m_rect.h"\
	"..\..\..\phoenix\inc\m_sphere.h"\
	"..\..\..\phoenix\inc\m_trig.h"\
	"..\..\..\phoenix\inc\memstrm.h"\
	"..\..\..\phoenix\inc\ml.h"\
	"..\..\..\phoenix\inc\persist.h"\
	"..\..\..\phoenix\inc\resmanager.h"\
	"..\..\..\phoenix\inc\rlestrm.h"\
	"..\..\..\phoenix\inc\streamio.h"\
	"..\..\..\phoenix\inc\streams.h"\
	"..\..\..\phoenix\inc\talgorithm.h"\
	"..\..\..\phoenix\inc\threadbase.h"\
	"..\..\..\phoenix\inc\ts_camera.h"\
	"..\..\..\phoenix\inc\ts_celanimmesh.h"\
	"..\..\..\phoenix\inc\ts_material.h"\
	"..\..\..\phoenix\inc\ts_pointarray.h"\
	"..\..\..\phoenix\inc\ts_rendercontext.h"\
	"..\..\..\phoenix\inc\ts_renderitem.h"\
	"..\..\..\phoenix\inc\ts_shape.h"\
	"..\..\..\phoenix\inc\ts_transform.h"\
	"..\..\..\phoenix\inc\ts_types.h"\
	"..\..\..\phoenix\inc\ts_vertex.h"\
	"..\..\..\phoenix\inc\tsorted.h"\
	"..\..\..\phoenix\inc\tvector.h"\
	"..\..\..\phoenix\inc\volstrm.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\SceneEnum.cpp
DEP_CPP_SCENE=\
	".\Inc\MeshBuilder.h"\
	".\Inc\SceneEnum.h"\
	".\Inc\ShapeBuilder.h"\
	
NODEP_CPP_SCENE=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\decomp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dummy.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\istdplug.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\modstack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stdmat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	"..\..\..\phoenix\inc\base.h"\
	"..\..\..\phoenix\inc\bitstream.h"\
	"..\..\..\phoenix\inc\blkstrm.h"\
	"..\..\..\phoenix\inc\filedefs.h"\
	"..\..\..\phoenix\inc\filstrm.h"\
	"..\..\..\phoenix\inc\lock.h"\
	"..\..\..\phoenix\inc\lzhstrm.h"\
	"..\..\..\phoenix\inc\m_base.h"\
	"..\..\..\phoenix\inc\m_box.h"\
	"..\..\..\phoenix\inc\m_collision.h"\
	"..\..\..\phoenix\inc\m_dist.h"\
	"..\..\..\phoenix\inc\m_dot.h"\
	"..\..\..\phoenix\inc\m_euler.h"\
	"..\..\..\phoenix\inc\m_lseg.h"\
	"..\..\..\phoenix\inc\m_mat2.h"\
	"..\..\..\phoenix\inc\m_mat3.h"\
	"..\..\..\phoenix\inc\m_mul.h"\
	"..\..\..\phoenix\inc\m_plane.h"\
	"..\..\..\phoenix\inc\m_plist.h"\
	"..\..\..\phoenix\inc\m_point.h"\
	"..\..\..\phoenix\inc\m_qsort.h"\
	"..\..\..\phoenix\inc\m_quat.h"\
	"..\..\..\phoenix\inc\m_random.h"\
	"..\..\..\phoenix\inc\m_rect.h"\
	"..\..\..\phoenix\inc\m_sphere.h"\
	"..\..\..\phoenix\inc\m_trig.h"\
	"..\..\..\phoenix\inc\memstrm.h"\
	"..\..\..\phoenix\inc\ml.h"\
	"..\..\..\phoenix\inc\persist.h"\
	"..\..\..\phoenix\inc\resmanager.h"\
	"..\..\..\phoenix\inc\rlestrm.h"\
	"..\..\..\phoenix\inc\streamio.h"\
	"..\..\..\phoenix\inc\streams.h"\
	"..\..\..\phoenix\inc\talgorithm.h"\
	"..\..\..\phoenix\inc\threadbase.h"\
	"..\..\..\phoenix\inc\ts_camera.h"\
	"..\..\..\phoenix\inc\ts_celanimmesh.h"\
	"..\..\..\phoenix\inc\ts_material.h"\
	"..\..\..\phoenix\inc\ts_pointarray.h"\
	"..\..\..\phoenix\inc\ts_rendercontext.h"\
	"..\..\..\phoenix\inc\ts_renderitem.h"\
	"..\..\..\phoenix\inc\ts_shape.h"\
	"..\..\..\phoenix\inc\ts_transform.h"\
	"..\..\..\phoenix\inc\ts_types.h"\
	"..\..\..\phoenix\inc\ts_vertex.h"\
	"..\..\..\phoenix\inc\tsorted.h"\
	"..\..\..\phoenix\inc\tvector.h"\
	"..\..\..\phoenix\inc\volstrm.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Code\ShapeBuilder.cpp
DEP_CPP_SHAPE=\
	".\Inc\MeshBuilder.h"\
	".\Inc\ShapeBuilder.h"\
	
NODEP_CPP_SHAPE=\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\acolor.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\animtbl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\appio.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\assert1.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\bitarray.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\box3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\buildver.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\channels.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\cmdmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\color.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\control.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\coreexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\custcont.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dbgprint.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\dpoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\euler.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\evuser.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\export.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gencam.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genhier.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genlight.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\genshape.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\geomlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfloat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfx.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gfxlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\gutil.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hitdata.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\hold.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\impexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\imtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\inode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\interval.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ioapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\iparamb.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ipoint3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\lockid.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\log.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\matrix3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\max.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxcom.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtess.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\maxtypes.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mesh.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\meshlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mouseman.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\mtl.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\nametab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\object.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\objmode.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patch.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchlib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\patchobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\plugin.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point2.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\point4.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ptrvec.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\quat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\ref.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\render.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\rtclick.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\sceneapi.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\snap.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\soundobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stack3.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\stdmat.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strbasic.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\strclass.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\tab.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\trig.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\triobj.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\units.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utilexp.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\utillib.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\vedge.h"\
	"..\..\..\..\program files\3dsmax2\maxsdk\include\winutil.h"\
	"..\..\..\phoenix\inc\base.h"\
	"..\..\..\phoenix\inc\bitstream.h"\
	"..\..\..\phoenix\inc\blkstrm.h"\
	"..\..\..\phoenix\inc\filedefs.h"\
	"..\..\..\phoenix\inc\filstrm.h"\
	"..\..\..\phoenix\inc\lock.h"\
	"..\..\..\phoenix\inc\lzhstrm.h"\
	"..\..\..\phoenix\inc\m_base.h"\
	"..\..\..\phoenix\inc\m_box.h"\
	"..\..\..\phoenix\inc\m_collision.h"\
	"..\..\..\phoenix\inc\m_dist.h"\
	"..\..\..\phoenix\inc\m_dot.h"\
	"..\..\..\phoenix\inc\m_euler.h"\
	"..\..\..\phoenix\inc\m_lseg.h"\
	"..\..\..\phoenix\inc\m_mat2.h"\
	"..\..\..\phoenix\inc\m_mat3.h"\
	"..\..\..\phoenix\inc\m_mul.h"\
	"..\..\..\phoenix\inc\m_plane.h"\
	"..\..\..\phoenix\inc\m_plist.h"\
	"..\..\..\phoenix\inc\m_point.h"\
	"..\..\..\phoenix\inc\m_qsort.h"\
	"..\..\..\phoenix\inc\m_quat.h"\
	"..\..\..\phoenix\inc\m_random.h"\
	"..\..\..\phoenix\inc\m_rect.h"\
	"..\..\..\phoenix\inc\m_sphere.h"\
	"..\..\..\phoenix\inc\m_trig.h"\
	"..\..\..\phoenix\inc\memstrm.h"\
	"..\..\..\phoenix\inc\ml.h"\
	"..\..\..\phoenix\inc\persist.h"\
	"..\..\..\phoenix\inc\resmanager.h"\
	"..\..\..\phoenix\inc\rlestrm.h"\
	"..\..\..\phoenix\inc\streamio.h"\
	"..\..\..\phoenix\inc\streams.h"\
	"..\..\..\phoenix\inc\talgorithm.h"\
	"..\..\..\phoenix\inc\threadbase.h"\
	"..\..\..\phoenix\inc\ts_camera.h"\
	"..\..\..\phoenix\inc\ts_celanimmesh.h"\
	"..\..\..\phoenix\inc\ts_material.h"\
	"..\..\..\phoenix\inc\ts_pointarray.h"\
	"..\..\..\phoenix\inc\ts_rendercontext.h"\
	"..\..\..\phoenix\inc\ts_renderitem.h"\
	"..\..\..\phoenix\inc\ts_shape.h"\
	"..\..\..\phoenix\inc\ts_transform.h"\
	"..\..\..\phoenix\inc\ts_types.h"\
	"..\..\..\phoenix\inc\ts_vertex.h"\
	"..\..\..\phoenix\inc\tsorted.h"\
	"..\..\..\phoenix\inc\tvector.h"\
	"..\..\..\phoenix\inc\volstrm.h"\
	

!IF  "$(CFG)" == "DTSExporter - Win32 Release"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Debug Pre Compress"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Vicon Compressing Exporter"

!ELSEIF  "$(CFG)" == "DTSExporter - Win32 Biped Exporter"

!ENDIF 

# End Source File
# End Group
# Begin Group "Resource"

# PROP Default_Filter "rc"
# Begin Source File

SOURCE=.\Res\DTSExporter.RC
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Inc\3DSIMP.H
# End Source File
# Begin Source File

SOURCE=.\Inc\3DSIRES.H
# End Source File
# Begin Source File

SOURCE=.\Inc\3DSShape.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CFILE.H
# End Source File
# Begin Source File

SOURCE=.\Inc\DTSExporter.h
# End Source File
# Begin Source File

SOURCE=.\Inc\KFIO.H
# End Source File
# Begin Source File

SOURCE=.\Inc\MeshBuilder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\MTLDef.H
# End Source File
# Begin Source File

SOURCE=.\Inc\MTLList.H
# End Source File
# Begin Source File

SOURCE=.\Inc\ObList.h
# End Source File
# Begin Source File

SOURCE=.\Inc\OFILE.H
# End Source File
# Begin Source File

SOURCE=.\Inc\resource.h
# End Source File
# Begin Source File

SOURCE=.\Inc\SceneEnum.h
# End Source File
# Begin Source File

SOURCE=.\Inc\ShapeBuilder.h
# End Source File
# End Group
# End Target
# End Project

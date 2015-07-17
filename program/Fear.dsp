# Microsoft Developer Studio Project File - Name="Fear" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Fear - Win32 FearNuMega
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fear.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fear.mak" CFG="Fear - Win32 FearNuMega"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fear - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fear - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Fear - Win32 NuMega" (based on "Win32 (x86) Application")
!MESSAGE "Fear - Win32 FearNuMega" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/", aaaaaaaa"
# PROP Scc_LocalPath ".."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fear - Win32 Release"

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
# ADD CPP /nologo /G5 /Gr /Zp4 /MD /W3 /Gi /GR /GX /Ot /Ow /Og /Oi /Ob1 /I "inc" /I "$(PHOENIXINC)" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /Gs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib rmSim.lib rmSimObjects.lib rmSimGui.lib rmSimNet.lib rmITR.lib rmGRD.lib rmGW.lib rmCore.lib rmDNet.lib rmLS.lib rmTed.lib rmTS3.lib rmGFXIO.lib rmML.lib rmCommon.lib /nologo /subsystem:windows /machine:I386 /out:"Exe/RMFear.exe" /libpath:"$(PHOENIXLIB)"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy /D $(PHOENIXDATA)\*.vol exe /R /Q	xcopy /D                 $(PHOENIXDATA)\*.cs exe\scripts /R /Q	xcopy /D $(PHOENIXDATA)\*.uc exe /R /Q
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Fear - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Exe"
# PROP Intermediate_Dir "obj\dmobj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "d:\fear\program\inc" /I "d:\darkstar\inc" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib dmSim.lib dmSimObjects.lib dmSimGui.lib dmSimNet.lib dmITR.lib dmGRD.lib dmGW.lib dmCore.lib dmDNet.lib dmLS.lib dmTed.lib dmTS3.lib dmGFXIO.lib dmML.lib dmCommon.lib dmConsole.lib winmm.lib dmgfx.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Exe/dmtribes.exe" /pdbtype:sept /libpath:"d:\darkstar\lib"

!ELSEIF  "$(CFG)" == "Fear - Win32 NuMega"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Fear___Win32_NuMega"
# PROP BASE Intermediate_Dir "Fear___Win32_NuMega"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "obj\dmnobj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "d:\fear\program\inc" /I "d:\darkstar\inc" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "d:\fear\program\inc" /I "d:\darkstar\inc" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib dmSim.lib dmSimObjects.lib dmSimGui.lib dmSimNet.lib dmITR.lib dmGRD.lib dmGW.lib dmCore.lib dmDNet.lib dmLS.lib dmTed.lib dmTS3.lib dmGFXIO.lib dmML.lib dmCommon.lib dmConsole.lib winmm.lib dmgfx.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Exe/DMFear.exe" /pdbtype:sept /libpath:"d:\darkstar\lib"
# ADD LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib dmnSim.lib dmnSimObjects.lib dmnSimGui.lib dmnSimNet.lib dmnITR.lib dmnGRD.lib dmnGW.lib dmnCore.lib dmnDNet.lib dmnLS.lib dmnTed.lib dmnTS3.lib dmnGFXIO.lib dmnML.lib dmnCommon.lib dmnConsole.lib winmm.lib dmngfx.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Exe/dmntribes.exe" /pdbtype:sept /libpath:"d:\darkstar\lib"

!ELSEIF  "$(CFG)" == "Fear - Win32 FearNuMega"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Fear___Win32_FearNuMega"
# PROP BASE Intermediate_Dir "Fear___Win32_FearNuMega"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "exe"
# PROP Intermediate_Dir "obj\dmnobj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "d:\fear\program\inc" /I "d:\darkstar\inc" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp4 /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "d:\fear\program\inc" /I "d:\darkstar\inc" /D "MSVC" /D "WIN32" /D EXPORT= /D "STRICT" /D USERENTRY=__cdecl /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib dmnSim.lib dmnSimObjects.lib dmnSimGui.lib dmnSimNet.lib dmnITR.lib dmnGRD.lib dmnGW.lib dmnCore.lib dmnDNet.lib dmnLS.lib dmnTed.lib dmnTS3.lib dmnGFXIO.lib dmnML.lib dmnCommon.lib dmnConsole.lib winmm.lib dmngfx.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Exe/dmntribes.exe" /pdbtype:sept /libpath:"d:\darkstar\lib"
# ADD LINK32 comctl32.lib user32.lib ws2_32.lib kernel32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib uuid.lib dmSim.lib dmSimObjects.lib dmSimGui.lib dmSimNet.lib dmITR.lib dmGRD.lib dmGW.lib dmCore.lib dmDNet.lib dmLS.lib dmTed.lib dmTS3.lib dmGFXIO.lib dmML.lib dmcommon.lib dmconsole.lib winmm.lib dmgfx.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"Exe/dmntribes2.exe" /pdbtype:sept /libpath:"d:\darkstar\lib"

!ENDIF 

# Begin Target

# Name "Fear - Win32 Release"
# Name "Fear - Win32 Debug"
# Name "Fear - Win32 NuMega"
# Name "Fear - Win32 FearNuMega"
# Begin Group "Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\code\aiHelp.cpp
# End Source File
# Begin Source File

SOURCE=.\code\aiObj.cpp
# End Source File
# Begin Source File

SOURCE=.\code\aiPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\BanList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\chatmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\clockhud.cpp
# End Source File
# Begin Source File

SOURCE=.\code\compasshud.cpp
# End Source File
# Begin Source File

SOURCE=.\code\cpuSpeed.cpp
# End Source File
# Begin Source File

SOURCE=.\code\CurWeapHud.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dataBlockManager.cpp
# End Source File
# Begin Source File

SOURCE=.\code\debris.cpp
# End Source File
# Begin Source File

SOURCE=.\code\decalManager.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgAddress.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgCreateServer.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgIRCChat.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgJoinGame.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgMainMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgPlayerSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgquit.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dlgRecordings.cpp
# End Source File
# Begin Source File

SOURCE=.\code\dropPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\code\editCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\code\explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\code\f_base.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearCSDelegate.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearDynamicDataPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiAbort.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiAddresses.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiArrayCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiBitmapCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiBool.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiBox.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiButton.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCenterPrint.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCFGButton.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiChat.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiChatDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiChatMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiCMDObserve.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCmdPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCmdTurretList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCmdView.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiColumns.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCommandItemList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCommandTeamList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiControl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiControlsCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiCrosshair.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiCurCmd.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiDlgBox.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiFilterCBList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiFilterCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiFilters.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiHelpCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiHudCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiHudList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiInventory.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiInvList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCActiveTextEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCActiveTextFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCBanControl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCChanCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCChannelList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCChannelPropsCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCHelpControl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiIRCJoinControl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCNicks.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCScroll.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCTextEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCTopic.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiIRCView.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiJetHud.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiLag.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiMasterList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiMenuCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiMenuDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiMissionCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiMissionList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiPage.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiPaletteCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiPlayerCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiPlayerList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiPopUpMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiRadio.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiRecList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiScoreList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiScrollCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiServerFilterCB.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiServerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiServerList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiShapeView.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiShellBorder.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiSimpleText.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiSkinView.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiSmacker.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiStandardCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiSwitch.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiTabMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiTestEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiTextFMT.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiTextFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\code\fearGuiTextList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiTSCommander.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearGuiUnivButton.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearHudArrayCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearHudCmdObj.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearHudInvList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearHudMatrixCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearHudRadarPing.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearMissionEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearMissionPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearPlayerPSC.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FearPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\filePlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\Flier.cpp
# End Source File
# Begin Source File

SOURCE=.\code\FlyingCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\code\gamebase.cpp
# End Source File
# Begin Source File

SOURCE=.\code\HealthHud.cpp
# End Source File
# Begin Source File

SOURCE=.\code\InputHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\code\interiorShape.cpp
# End Source File
# Begin Source File

SOURCE=.\code\item.cpp
# End Source File
# Begin Source File

SOURCE=.\code\main.cpp
# End Source File
# Begin Source File

SOURCE=.\code\marker.cpp
# End Source File
# Begin Source File

SOURCE=.\code\Mine.cpp
# End Source File
# Begin Source File

SOURCE=.\code\missionObjectList.cpp
# End Source File
# Begin Source File

SOURCE=.\code\moveable.cpp
# End Source File
# Begin Source File

SOURCE=.\code\observerCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\code\partDebris.cpp
# End Source File
# Begin Source File

SOURCE=.\code\Planet.cpp
# End Source File
# Begin Source File

SOURCE=.\code\player.cpp
# End Source File
# Begin Source File

SOURCE=.\code\playerCollision.cpp
# End Source File
# Begin Source File

SOURCE=.\code\playerInventory.cpp
# End Source File
# Begin Source File

SOURCE=.\code\PlayerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\code\playerUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projAttTrail.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projBullet.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projectileFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projGrenade.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projLaser.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projLightning.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projRepair.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projRocketDumb.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projSeekMissile.cpp
# End Source File
# Begin Source File

SOURCE=.\code\projTargetLaser.cpp
# End Source File
# Begin Source File

SOURCE=.\code\ScriptPlugin.cpp
# End Source File
# Begin Source File

SOURCE=.\code\sensor.cpp
# End Source File
# Begin Source File

SOURCE=.\code\sensorManager.cpp
# End Source File
# Begin Source File

SOURCE=.\code\shapebase.cpp
# End Source File
# Begin Source File

SOURCE=.\code\shapeCollision.cpp
# End Source File
# Begin Source File

SOURCE=.\code\simMoveobj.cpp
# End Source File
# Begin Source File

SOURCE=.\code\Sky.cpp
# End Source File
# Begin Source File

SOURCE=.\code\smokePuff.cpp
# End Source File
# Begin Source File

SOURCE=.\code\snowfall.cpp
# End Source File
# Begin Source File

SOURCE=.\code\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\code\StarField.cpp
# End Source File
# Begin Source File

SOURCE=.\code\staticBase.cpp
# End Source File
# Begin Source File

SOURCE=.\code\teamGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\code\tribesShadowRenderImage.cpp
# End Source File
# Begin Source File

SOURCE=.\code\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\code\turret.cpp
# End Source File
# Begin Source File

SOURCE=.\code\vehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\code\version.cpp
# End Source File
# End Group
# End Target
# End Project

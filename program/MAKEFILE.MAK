##############################################################################
PHOENIXMAKE ?= ..\..\..\makes

%include <$(PHOENIXMAKE)\builtins.mak>

#%set PRECOMPILED_HEADER=

%ifdef CG32
CPPFLAGS += -vG
%endif

.PATH.ttag = .\exe
.PATH.cs   = .\exe\scripts

BUILDDIR = ..\build
BUILDBASEDIR = ..\build\base
%set STATIC=true

##############################################################################
#
LIBS =                              \
      %ifdef CG32
      $(BORLAND)\lib\cg32.lib       \
      %endif
      $(BuildPrefix)sim.lib         \
      $(BuildPrefix)SimObjects.lib  \
      $(BuildPrefix)SimGui.lib      \
      $(BuildPrefix)SimNet.lib      \
      $(BuildPrefix)itr.lib         \
      $(BuildPrefix)grd.lib         \
      $(BuildPrefix)gw.lib          \
      $(BuildPrefix)core.lib        \
      $(BuildPrefix)dnet.lib        \
      $(BuildPrefix)ls.lib          \
      $(BuildPrefix)ted.lib         \
      $(BuildPrefix)ts3.lib         \
      $(BuildPrefix)gfxio.lib       \
      $(BuildPrefix)gfx.lib         \
      $(BuildPrefix)ml.lib          \
      $(BuildPrefix)console.lib     \
      $(BuildPrefix)common.lib


##############################################################################
# lsDlg.res needed for the lsDlgPlugin
#
MAIN_OBJ =                          \
    generic.def                     \
    main.obj                        \
    cpuSpeed.obj                    \
    cpuid.obj                       \
    MissionObjectList.obj           \
    fear.res                        \
    ted.res			    			\
    lsDlg.res			    		\
    toolplugin.res

FEAR_SHELL = \
	fearGuiControl.obj				  \
   fearGuiCenterPrint.obj       	  \
   fearGuiTextFMT.obj              \
   fearGuiTextList.obj             \
   fearGuiInventory.obj            \
   fearGuiIRCChannelList.obj       \
   fearGuiIRCChannelPropsCtrl.obj  \
	fearGuiIRCBanControl.obj        \
	fearGuiIRCJoinControl.obj       \
	fearGuiIRCActiveTextEdit.obj    \
	fearGuiIRCActiveTextFormat.obj  \
	fearGuiIRCView.obj			  	  \
	fearGuiIRCTextEdit.obj			  \
	fearGuiIRCNicks.obj			  	  \
	fearGuiIRCChanCtrl.obj		  	  \
	fearGuiIRCTopic.obj		  	  	  \
	fearGuiIRCScroll.obj			  	  \
	fearGuiHudList.obj				  \
	fearGuiMenuCtrl.obj				  \
   fearGuiShapeView.obj            \
   fearGuiSkinView.obj             \
	fearGuiShellBorder.obj          \
	fearGuiCFGButton.obj	        	  \
	fearGuiUnivButton.obj	        \
	fearGuiAbort.obj                \
	fearGuiBox.obj                  \
	fearGuiCmdPanel.obj             \
	fearGuiCmdView.obj              \
	fearGuiCurCmd.obj               \
	fearGuiDlgBox.obj               \
	fearGuiChat.obj                 \
	fearGuiChatDisplay.obj          \
	fearGuiChatMenu.obj          	  \
	fearGuiCommandTeamList.obj      \
	fearGuiCommandItemList.obj      \
	fearGuiCmdTurretList.obj        \
	fearGuiCrosshair.obj            \
	fearGuiDialog.obj               \
	fearGuiHelpCtrl.obj				  \
	fearGuiHudCtrl.obj              \
	fearGuiInvList.obj              \
	fearGuiJetHud.obj               \
	fearGuiMenu.obj                 \
        fearGuiPopUpMenu.obj            \
	fearGuiBitmapCtrl.obj        	\
	fearGuiArrayCtrl.obj         	\
   fearGuiCMDObserve.obj           \
	fearGuiBuddyCombo.obj       		\
	fearGuiCombo.obj         		\
	fearGuiControlsCombo.obj        \
	fearGuiLag.obj			        	  \
	fearGuiMissionCombo.obj         \
	fearGuiMissionList.obj          \
	fearGuiPlayerCombo.obj          \
	fearGuiPlayerList.obj           \
	fearGuiStandardCombo.obj        \
	fearGuiColumns.obj          	\
	fearGuiFilters.obj          	\
	fearGuiFilterCombo.obj       	\
	fearGuiFilterCBList.obj    	    \
	fearGuiServerFilterCB.obj    	\
	fearGuiPage.obj                 \
	fearGuiRadio.obj                \
	fearGuiScoreList.obj            \
	fearGuiScrollCtrl.obj           \
	fearGuiServerList.obj           \
	fearGuiServerInfo.obj            \
	fearGuiSimpleText.obj			  \
	fearGuiSlider.obj					  \
	fearGuiPaletteCtrl.obj          \
	fearGuiSwitch.obj               \
	fearGuiTabMenu.obj	            \
	fearGuiTestEdit.obj             \
	fearGuiTextFormat.obj			  \
	fearGuiTSCommander.obj          \
	fearGuiRecList.obj				\
	fearGuiMenuDisplay.obj			\
	fearGuiMasterList.obj			\
	fearGuiSmacker.obj					\
	dlgQuit.obj							 \
	dlgJoinGame.obj                 \
	dlgRecordings.obj               \
	dlgOptions.obj                  \
	dlgCreateServer.obj             \
	dlgIRCChat.obj                   \
	dlgMainMenu.obj                 \
	dlgConnect.obj                 \
	dlgPlayerSetup.obj                 \
   dlgPlay.obj                     \
   tribesShadowRenderImage.obj     \
   fearMissionPlugin.obj           \
   fearMissionEditor.obj            \
   editCamera.obj                   \

FEAR_HUD = \
	fearHudMatrixCtrl.obj        		\
	fearHudArrayCtrl.obj         		\
	fearHudInvList.obj         		\
	fearHudCmdObj.obj         			\
	healthHud.obj                   \
	clockHud.obj                    \
	compassHud.obj                  \
	curWeapHud.obj                  \
	fearHudRadarPing.obj			\

FEAR_SIM =							\
	filePlugin.obj				    \
	banList.obj						\
    chatMenu.obj                    \
    dataBlockManager.obj            \
	fearPlayerPSC.obj               \
	PlayerManager.obj               \
	player.obj                      \
	playerUpdate.obj                \
	playerInventory.obj             \
                                    \
	fearPlugin.obj                  \
	fearDynamicDataPlugin.obj       \
									\
	fearCSDelegate.obj              \
	dropPoints.obj                  \
	flier.obj                       \
    FlyingCamera.obj            	\
    InputHandler.obj            	\
#	tank.obj						\
    decalManager.obj            	\
    observerCamera.obj          	\
	playerCollision.obj         	\
	shapeCollision.obj          	\
    sensor.obj                  	\
    sensorManager.obj           	\
    scriptPlugin.obj                \
    aiObj.obj 						\
    aiSkill.obj                     \
    aiGraph.obj					    \
    aiPlugin.obj					\
   	aiHelp.obj						\
	snowfall.obj                    \
    moveable.obj                    \
	vehicle.obj						\
	version.obj						\
									\
	teamGroup.obj                   \
	gameBase.obj                    \
	shapeBase.obj                   \
	staticBase.obj                  \
	item.obj                        \
									\
	turret.obj                      \
	mine.obj                        \
									\
    projectile.obj                  \
    smokePuff.obj               	\
    projBullet.obj                  \
    projGrenade.obj                 \
    projRocketDumb.obj              \
    projLaser.obj                   \
    projTargetLaser.obj             \
    projAttTrail.obj                \
    projSeekMissile.obj             \
    projLightning.obj               \
    projRepair.obj               \
    explosion.obj                   \
    projectileFactory.obj           \
                                    \
    debris.obj                      \
    partDebris.obj                  \
                                    \
	sky.obj							\
	StarField.obj				\
    planet.obj						\
                                    \
	interiorShape.obj				\
    sound.obj                  		\
    marker.obj                      \
    trigger.obj                     \
%if $(COMPILER) != "m"
    stdlib.obj                      \
    MDFmem.obj                      \
%endif
#   memplugin.obj                    \
#	memory.obj 						\
#	memdef.obj

EXTRA =                             \
    simMoveObj.obj                  \
    f_base.obj                      \



##############################################################################
#
.ALWAYS : version.obj

game:                                \
    DarkstarData                    \
    Strings                         \
    $(BuildPrefix)tribes.exe          

all:                                \
    DarkstarData                    \
    Strings                         \
    $(BuildPrefix)tribes.exe         \
	missionlighting.exe				

DarkstarData :
   %if %defined(PHOENIXDATA)
      xcopy /D $(PHOENIXDATA)\*.vol exe\base        /R /Q
      xcopy /D $(PHOENIXDATA)\*.cs exe\base\scripts /R /Q
   %endif

#

$(BuildPrefix)tribes.exe:    \
   $(FEAR_SIM)             \
   $(FEAR_SHELL)           \
   $(FEAR_HUD)             \
   $(MAIN_OBJ)             \
   $(EXTRA)                \
   $(LIBS)                         

SimDLL.lib: $(BuildPrefix)tribes.exe
   implib lib\SimDLL.lib exe\$(BuildPrefix)tribes.exe >nul

scripts .MAKE .ALWAYS .MISER:
   %if %defined(PHOENIXDATA)
   %endif
   @%chdir $(.TARGET)
   @make $(MFLAGS)
   @%chdir $(MAKEDIR)

Strings:                            \
    inc\fear.strings.h              \
    inc\help.strings.h              \
    inc\sfx.strings.h               

inc\fear.strings.h .MAKE .MISER : exe\base\scripts\fear.strings.cs
   @tag2bin -iinc -sexe\base\scripts fear.strings.cs

inc\help.strings.h .MAKE .MISER : exe\base\scripts\help.strings.cs
   @tag2bin -iinc -sexe\base\scripts help.strings.cs

inc\sfx.strings.h .MAKE .MISER : exe\base\scripts\sfx.strings.cs
   @tag2bin -iinc -sexe\base\scripts sfx.strings.cs

##############################################################################
# missionlighting program

MISSIONLIGHTING_OBJ =				\
%if $(COMPILER) != "m"
    MDFmem.obj                   \
%endif
    missionlighting.obj

MISSIONLIGHTING_LIBS = 				\
	$(BuildPrefix)ssCommon.lib      \
	$(BuildPrefix)ssSim.lib      	\
	$(BuildPrefix)ssSimObjects.lib  \
	$(BuildPrefix)zeditr.lib

# MISSIONLIGHTING_OBJ must be first in this list
missionlighting.exe:        \
	$(MISSIONLIGHTING_OBJ)  \
	$(FEAR_SHELL)           \
	$(FEAR_HUD)             \
	$(FEAR_SIM)             \
	$(MISSIONLIGHTING_LIBS) \
	$(LIBS)                 \
	$(EXTRA)
	%do %.exe CONSOLE=true CPPFLAGS+=-DFEAR

##############################################################################
# Microsoft memory dll (exports only malloc/free/realloc/calloc)
%if $(COMPILER) == "m"
MEM_DLL_OBJ =     \
   mem_dll.obj
mem.dll:          \
   $(MEM_DLL_OBJ)
%endif

##############################################################################
# Release Build
##############################################################################
FONTS_VOL   = exe\base\fonts\*.*
SCRIPTS_VOL = exe\base\scripts\*.*
GUI_VOL     = exe\base\gui\*.*
PREFAB_VOL  = exe\base\prefabs\*.*

BUILD_FILES = \
	missionlighting.exe \
	dbtribes.exe        \
	rbtribes.exe        \
	console.cs          \
	smackw32.dll		\

BASE_FILES = \
	darkstar.vol        \
	edit.vol            \
	editor.vol          \
	guiToolbar.vol      \
	ted.vol			    \
	shell.vol           \
	shellcommon.vol		\
	entities.vol        \
	interface.vol       \
	alienDML.vol		\
	alienTerrain.vol    \
	alienWorld.vol		\
	desertDML.vol       \
	desertTerrain.vol   \
	desertWorld.vol		\
	iceDML.vol          \
	iceTerrain.vol		\
	iceWorld.vol		\
	lushDML.vol         \
	lushTerrain.vol		\
	lushWorld.vol		\
	mudDML.vol			\
	mudTerrain.vol		\
	mudWorld.vol		\
	marsDML.vol			\
	marsTerrain.vol		\
	marsWorld.vol		\
	human1DML.vol		\
	titanDML.vol		\
	savanaDML.vol		\
	sound.vol			\
	dynlogoF.smk

CONFIG_FILES =
	example.cs          \

VOICE_FILES = \
	male1.vol           \
	male2.vol           \
	male3.vol           \
	male4.vol           \
	male5.vol           \
	female1.vol         \
	female2.vol         \
	female3.vol         \
	female4.vol         \
	female5.vol         \

SKIN_FILES =

MISSION_FILES =

build: buildDir buildVolumes buildCopy

buildDir:
	# Should have a make clean to delete the build
	# @--del /F /S /Q $(BUILDDIR)
	@--mkdir $(BUILDDIR)
	@--mkdir $(BUILDDIR)\config
	@--mkdir $(BUILDDIR)\recordings
	@--mkdir $(BUILDDIR)\temp
	@--mkdir $(BUILDBASEDIR)
	@--mkdir $(BUILDBASEDIR)\missions
	@--mkdir $(BUILDBASEDIR)\voices
	@--mkdir $(BUILDBASEDIR)\skins

#
#
#
buildCopy: buildCopyExe buildCopyBase buildCopyVoices buildCopySkins \
	buildCopyConfig buildCopyMissions

buildCopyExe: $(BUILD_FILES,<exe\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)
	%endfor

buildCopyBase: $(BASE_FILES,<exe\base\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)\base
	%endfor

buildCopyConfig: $(CONFIG_FILES,<exe\config\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)\config
	%endfor

buildCopyVoices: $(VOICE_FILES,<exe\base\voices\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)\base\voices
	%endfor

buildCopySkins: $(SKIN_FILES,<exe\base\skins\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)\base\skins
	%endfor

buildCopyMissions: $(MISSION_FILES,<exe\base\missions\)
	%foreach SFILE in $(.SOURCES)
		@xcopy /D $(SFILE) $(BUILDDIR)\base\missions
	%endfor

#
#
#
buildVolumes: $(BUILDBASEDIR)\fonts.vol $(BUILDBASEDIR)\scripts.vol $(BUILDBASEDIR)\gui.vol $(BUILDBASEDIR)\prefabs.vol

$(BUILDBASEDIR)\fonts.vol: $(FONTS_VOL,*F)
	@del /F /Q $(.TARGET)
	%foreach SFILE in $(.SOURCES)
		@--vt -sp $(.TARGET) $(SFILE)
	%endfor

$(BUILDBASEDIR)\scripts.vol: $(SCRIPTS_VOL,*F)
	@del /F /Q $(.TARGET)
	%foreach SFILE in $(.SOURCES)
		@--vt -sp $(.TARGET) $(SFILE)
	%endfor

$(BUILDBASEDIR)\prefabs.vol: $(PREFAB_VOL,*F)
	@del /F /Q $(.TARGET)
	%foreach SFILE in $(.SOURCES)
		@--vt -sp $(.TARGET) $(SFILE)
	%endfor
	
$(BUILDBASEDIR)\gui.vol: $(GUI_VOL,*F)
	@del /F /Q $(.TARGET)
	@%chdir exe\base
	# This is a little weird, but we want to preserve the
	# subdirectory name after .\exe in the volume
	%foreach SFILE in $(.SOURCES,F,<gui\)
		@--vt ..\..\$(.TARGET) $(SFILE)
	%endfor
	@%chdir ..\..

#
#
#
clean :
	@rmdir /s /q lib
	@rmdir /s /q obj
	@del /q exe\*.strings.btag
	@del /q inc\*.strings.h
	@del /q exe\dbtribes.*
	@del /q exe\rbtribes.*
	@del /q exe\missionlighting.*


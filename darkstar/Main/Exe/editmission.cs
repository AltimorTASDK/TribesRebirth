if test "$1" == ""
   med
   ted

   EditMission med

   newToolWindow editbar MainWindow

   addToolButton editbar SetGame SetPlayingGame.bmp
   setButtonHelp editbar SetGame "Resume Game"
   setToolCommand editbar SetGame "EditMission game"

   addToolButton editbar SetMission SetEditingMission.bmp
   setButtonHelp editbar SetMission "Mission Edit Mode"
   setToolCommand editbar SetMission "EditMission med"

   addToolButton editbar SetTerrain SetEditingTerrain.bmp
   setButtonHelp editbar SetTerrain "Terrain Edit Mode"
   setToolCommand editbar SetTerrain "EditMission ted"

   bind Escape winMouse
   bind f9  EditMission game
   bind f10 EditMission med
   bind f11 EditMission ted

else
if test "$1" == "med"
   focus MissionEditor
   unfocus TedObject

   winMouse
   cursorOn MainWindow
   GuiNewContentCtrl MainWindow SimGui::TSControl
   focus MEMoveObj
   postAction TS_0 Attach MEMoveObj
   unfocus GhostGroup\player
else
if test "$1" == "ted"
   focus TedObject
   unfocus MissionEditor

   winMouse
   cursorOn MainWindow
   GuiNewContentCtrl MainWindow SimGui::TSControl
   focus MEMoveObj
   postAction TS_0 Attach MEMoveObj
   unfocus GhostGroup\player
else
if test "$1" == "game"
   unfocus TedObject
   unfocus MissionEditor

   dirMouse
   cursorOff MainWindow
   GuiLoadContentCtrl MainWindow gui\play.gui
   focus GhostGroup\player
   postAction playGui Attach playerCam
endif
endif
endif
endif



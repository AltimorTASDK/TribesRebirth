// define the 3 mission edit mode functions
// each one disables the other two

function MEModeMission()
{
   focus( MissionEditor );
   unfocus( TedObject );

   winMouse();
   cursorOn( MainWindow );
   unfocus( "2048\\PlayerPSC" );

   GuiNewContentCtrl(MainWindow, SimGui::TSControl);
   focus(editCamera);
   postAction(TS_0, Attach, editCamera);
}

function MEModeTed()
{
   focus( TedObject );
   unfocus( MissionEditor );

   winMouse();
   cursorOn( MainWindow );
   unfocus( "2048\\PlayerPSC" );

   GuiNewContentCtrl(MainWindow, SimGui::TSControl);
   focus(editCamera);
   postAction(TS_0, Attach, editCamera);
}

function MEModeGame()
{
   unfocus( TedObject );
   unfocus( MissionEditor );

   dirMouse();
   cursorOff( MainWindow );
   unfocus(editCamera);
	GuiLoadContentCtrl( MainWindow, "gui\\play.gui" );
   focus( "2048\\PlayerPSC" );
   postAction( playGui, Attach, "2048\\PlayerPSC" );
}

// load mission editor and ted stuff
exec( "med.cs" );
exec( "ted.cs" );
move();
ted();

// add the buttons to the mission editor
addMissionButton( SetGame, "SetPlayingGame.bmp", "Resume play", "MEModeGame();" );
addMissionButton( SetMission, "SetEditingMission.bmp", "Mission Edit Mode", "MEModeMission();" );
addMissionButton( SetTerrain, "SetEditingTerrain.bmp", "Terain Edit mode", "MEModeTed();" );
addMissionButton( Space1 );
addMissionButton( Pause, "SetPauseGame.bmp", "Pause game", "$SimGame::TimeScale = 0.0;", "$SimGame::TimeScale = 1.0;" );
addMissionButton( Space2 );
addMissionButton( Undo, "Undo.bmp", "Undo move/rotate", "missionUndoMoveRotate();" );
addMissionButton( Space3 );
addMissionButton( MovementMethod, "Temp.bmp", "New movement?", "$MED::useOldMovementMethod = false;", "$MED::useOldMovementMethod = true;" );

//addMissionButton( Undo, "Undo.bmp", "Undo move/rotate", "missionUndoMoveRotate();" );

bind(keyboard, make, Escape, to, "winMouse();" );

//
// bind some handy keys
bind(keyboard, make, f9, to, "MEModeGame();" );
bind(keyboard, make, f10, to, "MEModeMission();" );
bind(keyboard, make, f11, to, "MEModeTed();" );

// go into mission edit mode
MEModeMission();

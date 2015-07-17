//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _MISSIONPLUGIN_H_
#define _MISSIONPLUGIN_H_

//------------------------------------------------------------------------------

class MissionPlugin : public SimConsolePlugin
{
	enum CallbackID
	{
		newMissionEditor,          // creates a TreeView list box showing simObjects
      MissionRegType,            // register an object type mask value (See simobjecttypes.h)
      MissionRegObject,          // register a console command with Objects menu
      MissionRegTerrain,         // register a console command with Terrain menu
      MissionAddObject,          // adds existing object to Mission
      MissionCreateObject,       // creates object with persist and adds it to Mission
      MissionLoadObjects,        // load a saved mission object file
      newMissionGroup,           // creates a new mission group
      newMission,                // create a new mission
      addMissionButton,          // adds a button to the toolbar
      removeMissionButton,       // removes the button from the toolbar
      removeMissionButtons,      // removes all the buttons from the toolbar, and the toolbar itself
      setMissionButtonChecked,   // sets the state for a toggle button in the toolbar - processes command too
      setMissionButtonEnabled,   // sets the button to be enabled or not
      isMissionButtonChecked,    // checks if the button is pressed in or not
      isMissionButtonEnabled,    // checks to see if the button is enabled or not
      setAutoSaveInterval,       // set the interval between autosaving - 0 to disable
      setAutoSaveName,           // filename to autosave to - defaults to autosave.mis
      saveObjectPersist,         // persist object to disk
      loadObjectPersist,         // load in a persisted object
      undoMoveRotate,            // undo one level of move/rotation
	};

public:
	void init();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
   const char * getConsoleCommandString( int argc, const char * argv[], int offset );
};

#endif   // _MISSIONPLUGIN_H_

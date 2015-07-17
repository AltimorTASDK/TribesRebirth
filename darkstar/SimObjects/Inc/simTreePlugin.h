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

#ifndef _SIMTREEPLUGIN_H_
#define _SIMTREEPLUGIN_H_

//------------------------------------------------------------------------------

class SimTreePlugin : public SimConsolePlugin
{
	enum CallbackID
	{
		SimTreeCreate,          // creates a TreeView list box showing simObjects
      SimTreeAddSet,        // specifies a group to display in the treeview
      SimTreeRegBitmaps,      // specify list of bitmaps to use for treeview
      SimTreeRegClass,        // associate class type with a bitmap index
      SimTreeRegScript,       // Add a script or script command to the menu
	};

public:
	void init();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif   // _SIMTREEPLUGIN_H_

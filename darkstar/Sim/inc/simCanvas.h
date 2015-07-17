//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMCANVAS_H_
#define _SIMCANVAS_H_

#include <simbase.h>
#include <simev.h>
#include <simaction.h>
#include <gwcanvas.h>

//--------------------------------------------------------------------------- 

class GFXPalette;


//--------------------------------------------------------------------------- 
// Contains the list of canvas' to be rendered

class SimCanvasSet: public SimSet
{
public:
	SimCanvasSet() { id = SimCanvasSetId; }
	virtual void render();
};

class CMDConsole;

//--------------------------------------------------------------------------- 
// SimCanvas wraps the Window library's canvas class
// and manages drawing the console
// all the old viewport code was ripped out (causa SimGui) -
// if someone needs it he/she can make a subclass of SimCanvas
// to deal with it - Mark

class SimCanvas: public GWCanvas, public SimObject
{
protected:
   typedef SimObject Parent;
   typedef GWCanvas  CanvasParent;
   CMDConsole     *console;

public:
   SimCanvas();
   ~SimCanvas();

   void surfaceChanged();

   bool processArguments(int argc, const char **argv);

   bool onAdd();

   virtual void onMM_MCINotify(WPARAM wFlags, LPARAM lDevID);  // notification when MCI device like cd audio
                                                               // has completed a command, (ie, done Playing current track)
   virtual void onMM_MIXMControlChange(WPARAM hMixer, LPARAM dwControlId); // notification when volume has been changed
   virtual void onMM_MIXMLineChange(WPARAM hMixer, LPARAM dwLineId);       // outsied our app

	virtual void preRender();
	virtual void render();

	virtual void setPalette(GFXPalette *pPalette, int transitionColor = -1);
};


#endif //_SIMCANVAS_H_

//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <sim.h>
#include <gwcanvas.h>
#include <gwmain.h>
#include "simCanvas.h"
#include "console.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "simConsolePlugin.h"
#include "simGame.h"
#include "d_caps.h"
//------------------------------------------------------------------------------
void SimCanvasSet::render()
{
   for ( iterator i= begin(); i != end(); i++)
   {
      SimCanvas *canvas = static_cast<SimCanvas*>(*i);
      canvas->preRender();
      canvas->lock();
      canvas->render();
      canvas->unlock();
      canvas->flip();
   }   
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SimCanvas::SimCanvas()   
{ 
	console = CMDConsole::getLocked();
}

//------------------------------------------------------------------------------
SimCanvas::~SimCanvas()
{ 
}

//------------------------------------------------------------------------------

bool SimCanvas::processArguments(int argc, const char **argv)
{
   CMDConsole *console = CMDConsole::getLocked();
   int w = 640, h = 480;
   const char *title;
   bool quitOnDestroy = false;
   if (argc > 0)
      title = argv[0];
   else
      title = "Main";
   if (argc > 1) sscanf(argv[1], "%d", &w);
   if (argc > 2) sscanf(argv[2], "%d", &h);
   if ((argc > 3) &&
       (argv[3][0]=='t' || argv[3][0]=='T' || argv[3][0]=='1'))
      quitOnDestroy = true;

   if (argc >= 6) {
      Point2I min;
      Point2I max;
      sscanf(argv[4], "%d %d", &min.x, &min.y);
      sscanf(argv[5], "%d %d", &max.x, &max.y);
      setEnforceCanvasSize(min, max);
   }

	if (argc > 0) 
	{
		//bool mw = (argc > 2)? atoi(argv[2]) != 0: false;
      if( GWCanvas::create(title,
   			WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
   			Point2I(CW_USEDEFAULT,0),
            Point2I(w, h),
   			NULL,0))
   	{
   	   show(GWMain::cmdShow);
         GWCanvas::quitOnDestroy( quitOnDestroy );
         
         SimWorld *world = SimGame::get()->getWorld();
			
         if ( world )
   		   setPalette(world->getPalette());
         return true;
      }
      else
			console->printf("SimCanvas: error creating GWCanvas");
   }
	else
		console->printf("SimCanvas: title width height quit(t/f)");
	return false;
}

bool SimCanvas::onAdd()
{
   addToSet(SimCanvasSetId);

   return Parent::onAdd();
}

//------------------------------------------------------------------------------
void SimCanvas::preRender()
{
}

//------------------------------------------------------------------------------
void SimCanvas::render()
{
	//if (console)
	//	console->render(getSurface());
}      

//------------------------------------------------------------------------------
void SimCanvas::setPalette(GFXPalette *pPalette, int transitionColor) 
{ 
   if (getSurface() && transitionColor != -1 && !(getSurface()->getCaps() & GFX_DEVCAP_PALETTE_CHANGE_NOFLASH))
   {
      lock();
      getSurface()->clear(transitionColor);
      unlock();
      flip();
   }
   GWCanvas::setPalette(pPalette);
}

void
SimCanvas::surfaceChanged()
{
   CanvasParent::surfaceChanged();

   if (console != NULL) {
      float gammaValue = console->getFloatVariable("pref::Display::gammaValue", 1.0f);
      AssertFatal(getSurface() != NULL, "No surface to set gammaValue");
      getSurface()->setGamma(gammaValue);
   }
   AssertWarn(console != NULL, "Could not set canvas gamma value...");
}

//------------------------------------------------------------------------------
void SimCanvas::onMM_MCINotify(WPARAM wFlags, LPARAM lDevID)
{
   SimSet *redBookSet = dynamic_cast<SimSet *>(manager->findObject(SimRedbookSetId));
   if (redBookSet)
   {
      SimMCINotifyEvent ev;
      ev.flags = wFlags;
      ev.devId = lDevID;
      redBookSet->processEvent(&ev);                  
   }
}   

//------------------------------------------------------------------------------
void SimCanvas::onMM_MIXMControlChange(WPARAM hMixer, LPARAM dwControlId)
{
   SimSet *redBookSet = dynamic_cast<SimSet *>(manager->findObject(SimRedbookSetId));
   if (redBookSet)
   {
      SimMIXNotifyEvent ev;
      ev.hMixer = hMixer;
      ev.dwId = dwControlId;
      ev.control = true;
      redBookSet->processEvent(&ev);                  
   }
}   

//------------------------------------------------------------------------------
void SimCanvas::onMM_MIXMLineChange(WPARAM hMixer, LPARAM dwLineId)
{
   SimSet *redBookSet = dynamic_cast<SimSet *>(manager->findObject(SimRedbookSetId));
   if (redBookSet)
   {
      SimMIXNotifyEvent ev;
      ev.hMixer = hMixer;
      ev.dwId = dwLineId;
      ev.control = false;
      redBookSet->processEvent(&ev);                  
   }
}     

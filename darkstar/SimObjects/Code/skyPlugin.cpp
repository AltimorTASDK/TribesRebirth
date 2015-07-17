//----------------------------------------------------------------------------
//
// skyPlugin.cpp
//
//----------------------------------------------------------------------------

#include <sim.h>
#include <ts.h>

#include "simConsolePlugin.h"
#include <SimSetIterator.h>
#include "skyPlugin.h"
#include "simStarField.h"
#include "simterrain.h"
#include "simsky.h"

enum SkyCallbackID {
   GlobeLines,
   ShowLowerStars,
   SetStarsVisibility,
   SetSkyMaterialListTag,
};


//----------------------------------------------------------------------------

void SkyPlugin::init()
{
	console->printf("SkyPlugin");
   console->addCommand(GlobeLines, "globeLines", this);
   console->addCommand(ShowLowerStars, "showLowerStars", this);
   console->addCommand(SetStarsVisibility, "setStarsVisibility", this);
   console->addCommand(SetSkyMaterialListTag, "setSkyMaterialListTag", this);
}

//----------------------------------------------------------------------------

void SkyPlugin::startFrame()
{
}

void SkyPlugin::endFrame()
{
}

//----------------------------------------------------------------------------

const char *SkyPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   argc;argv;
	switch(id) {
      case GlobeLines:
      {
         for ( SimSetIterator obj(manager); *obj; ++obj) 
         {
            SimStarField *sf = dynamic_cast<SimStarField *>(*obj);
            if (sf) {
               sf->toggleGlobeLines();
               break;
            }
         }   
         break;
      }
      
      case ShowLowerStars:
      {
         bool on = false;
         if (argc > 1 && (argv[1][0] == '1' || argv[1][0] == 't' || argv[1][0] == 't'))
            on = true;
         for ( SimSetIterator obj(manager); *obj; ++obj) 
         {
            SimStarField *sf = dynamic_cast<SimStarField *>(*obj);
            if (sf) {
               sf->setBottomVisible(on);
               break;
            }
         }            
         break;
      }   

      case SetStarsVisibility:
      {
         bool on = false;
         if (argc > 1 && (argv[1][0] == '1' || argv[1][0] == 't' || argv[1][0] == 't'))
            on = true;
         for ( SimSetIterator obj(manager); *obj; ++obj) 
         {
            SimStarField *sf = dynamic_cast<SimStarField *>(*obj);
            if (sf) {
               sf->setVisibility(on);
               break;
            }
         }            
         break;
      }   

      case SetSkyMaterialListTag:
      {
         int tag = atoi(argv[1]);
         for ( SimSetIterator obj(manager); *obj; ++obj) 
         {
            SimSky *sky = dynamic_cast<SimSky *>(*obj);
            if (sky) {
               sky->setMaterialListTag(tag);
               break;
            }
         }            
         break;
      }   


	}
	return 0;
}


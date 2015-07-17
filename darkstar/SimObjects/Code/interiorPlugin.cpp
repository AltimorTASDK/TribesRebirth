//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <sim.h>

#include "simConsolePlugin.h"
#include "interiorPlugin.h"
#include "simInterior.h"

//----------------------------------------------------------------------------

namespace {

int
mungeOutsideBits(int in_bits)
{
   // Takes the outsideBits int, and renders it into a decimal number that
   //  prints as binary, with the exception of the placeholder 8
   //
   in_bits >>= 2;
   
   int retVal = 8000000;

   if ((in_bits & (1 << 5)) != 0)
      retVal += 100000;
   if ((in_bits & (1 << 4)) != 0)
      retVal += 10000;
   if ((in_bits & (1 << 3)) != 0)
      retVal += 1000;
   if ((in_bits & (1 << 2)) != 0)
      retVal += 100;
   if ((in_bits & (1 << 1)) != 0)
      retVal += 10;
   if ((in_bits & (1 << 0)) != 0)
      retVal += 1;

   return retVal;
}

}; // namespace {}


void InteriorPlugin::init()
{
   console->printf("InteriorPlugin");
#ifdef DEBUG
   console->addCommand(SetInteriorShapeState, "setITRShapeState",       this);
   console->addCommand(SetInteriorLightState, "setITRLightState",       this);
   console->addCommand(AnimateLight,          "animateInteriorLight",   this);
   console->addCommand(StopLightAnim,         "stopInteriorLightAnim",  this);
   console->addCommand(ResetLight,            "resetInteriorLight",     this);
   console->addCommand(ToggleBoundingBox,     "ITR::toggleBoundingBox", this);
#endif
}

//----------------------------------------------------------------------------

void InteriorPlugin::startFrame()
{
}

void InteriorPlugin::endFrame()
{
   if(CMDConsole::updateMetrics)
   {
      // Update console variables.
      console->setIntVariable("ITRMetrics::OutsideBits",
                              mungeOutsideBits(ITRMetrics.render.outsideBits));
      console->setIntVariable("ITRMetrics::CurrentLeaf",
                              ITRMetrics.render.currentLeaf);
      console->setIntVariable("ITRMetrics::VisibleLeafs",
                              ITRMetrics.render.leafs);
      console->setIntVariable("ITRMetrics::VisibleSurfaces",
                              ITRMetrics.render.surfaces);
      console->setIntVariable("ITRMetrics::RenderedPolys",
                              ITRMetrics.render.polys);
      console->setIntVariable("ITRMetrics::SurfTransformCacheHits",
                              ITRMetrics.render.surfTransformCacheHits);
      console->setIntVariable("ITRMetrics::SurfTransformCacheMisses",
                              ITRMetrics.render.surfTransformCacheMisses);
      console->setIntVariable("ITRMetrics::NumRenderedInteriors",
                              ITRMetrics.numRenderedInteriors);
      console->setIntVariable("ITRMetrics::NumInteriorLinks",
                              ITRMetrics.render.numInteriorLinks);
   }
   ITRMetrics.reset();
}


//----------------------------------------------------------------------------

const char *InteriorPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   extern bool g_drawSimInteriorBBox;

   switch(id) {
     case SetInteriorShapeState: {
         if (argc == 3) {
            int stateNum;
            sscanf(argv[2], "%d", &stateNum);
            
            SimInterior* pInterior =
               dynamic_cast<SimInterior*>(manager->findObject(argv[1]));

            if (pInterior == NULL ||
                pInterior->setState(stateNum) == false) {
               console->printf("Error setting state on Object: %s", argv[1]);
               return "False";
            }
         } else {
            console->printf("setInteriorShapeState: objectName <state #>");
         }
      }
      break;
     case SetInteriorLightState: {
         if (argc == 3) {
            int stateNum;
            sscanf(argv[2], "%d", &stateNum);
            
            SimInterior* pInterior =
               dynamic_cast<SimInterior*>(manager->findObject(argv[1]));

            if (pInterior == NULL ||
                pInterior->setLightState(stateNum) == false) {
               console->printf("Error setting state on Object: %s", argv[1]);
               return "False";
            }
         } else {
            console->printf("setInteriorShapeState: objectName <state #>");
         }
      }
      break;
      case AnimateLight:
         if (argc == 4) {
            SimInterior* simInt = dynamic_cast<SimInterior*>(manager->findObject(argv[1]));
            if (simInt == NULL) {
               console->printf(avar("Object %s does not exist, or is not an SimInterior", argv[1]));
               return "False";
            }
            
            int loopCount;
            sscanf(argv[3], "%d", &loopCount);

            int lightId = simInt->getLightId(argv[2]);
            if (simInt->animateLight(lightId, loopCount) == false) {
               console->printf(avar("Could not animate light: %s", argv[2]));
               return "False";
            }
         } else {
            console->printf("animateInteriorLight: interiorObjName lightName loopCount");
         }
         break;
      case StopLightAnim:
         if (argc == 3) {
            SimInterior* simInt = dynamic_cast<SimInterior*>(manager->findObject(argv[1]));
            if (simInt == NULL) {
               console->printf(avar("Object %s does not exist, or is not an SimInterior", argv[1]));
               return "False";
            }
            
            int lightId = simInt->getLightId(argv[2]);
            if (simInt->deanimateLight(lightId) == false) {
               console->printf(avar("Could not stop light: %s", argv[2]));
               return "False";
            }
         } else {
            console->printf("stopInteriorLightAnim: interiorObjName lightName");
         }
         break;
      case ResetLight:
         if (argc == 3) {
            SimInterior* simInt = dynamic_cast<SimInterior*>(manager->findObject(argv[1]));
            if (simInt == NULL) {
               console->printf(avar("Object %s does not exist, or is not an SimInterior", argv[1]));
               return "False";
            }
            
            int lightId = simInt->getLightId(argv[2]);
            if (simInt->resetLight(lightId) == false) {
               console->printf(avar("Could not reset light: %s", argv[2]));
               return "False";
            }
         } else {
            console->printf("resetInteriorLight: interiorObjName lightName");
         }
         break;
      case ToggleBoundingBox:
         g_drawSimInteriorBBox ^= true;
         break;
   }
   return 0;
}


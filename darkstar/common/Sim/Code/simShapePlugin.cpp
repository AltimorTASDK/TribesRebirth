//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simConsolePlugin.h"
#include "simShapePlugin.h"
#include "simShape.h"

void
SimShapePlugin::init()
{
   console->printf("SimShapePlugin");
#ifdef DEBUG
   console->addCommand(ToggleBoundingBox, "SimShape::toggleBoundingBox", this);
#endif
}


const char *
SimShapePlugin::consoleCallback(CMDConsole* /*console*/,int id,
                                int /*argc*/, const char * /*argv*/[])
{
   switch (id) {
     case ToggleBoundingBox: {
         bool current = SimShape::getDrawShapeBBox();
         current = !current;
         SimShape::setDrawShapeBBox(current);
      }
      break;
   }

   return 0;
}

void
SimShapePlugin::endFrame()
{
   SimShape::sm_frameKey++;
}

//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include "movPlay.h"
#include "movPlayPlugin.h"

//------------------------------------------------------------------------------
void MovPlayPlugin::init()
{
	console->addCommand(NewMovPlay,         "newMovPlay",         this);
   console->addCommand(OpenMovie,          "openMovie",          this);
   console->addCommand(CloseMovie,         "closeMovie",         this);
   console->addCommand(PlayMovie,          "playMovie",          this);
   console->addCommand(PlayMovieToComp,    "playMovieToComp",    this);
   console->addCommand(StopMovie,          "stopMovie",          this);
   console->addCommand(PauseMovie,         "pauseMovie",         this);
}

//----------------------------------------------------------------------------
void MovPlayPlugin::startFrame()
{
}

void MovPlayPlugin::endFrame()
{
}

//----------------------------------------------------------------------------
const char * MovPlayPlugin::consoleCallback(CMDConsole *, int id,int argc,const char *argv[])
{
   if (!manager) return ("False");
	switch(id) 
	{
      //--------------------------------------
      case NewMovPlay:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (! mpObj)
            {
               MovPlay *mpObj = new MovPlay();
               manager->addObject( (SimObject *)mpObj, argv[1] );
               return (0);
            }
            else
               console->printf("MovPlay %s already exists!", argv[1]);
         }
         else
            console->printf("newMovPlay <objId>");
         return "False";
      }

      //--------------------------------------
      case OpenMovie:{
         if (argc == 3 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               if (mpObj->openMovie(argv[2]))
               {
                  return (0);
               }
               else
               {
                  console->printf("Could not open %s", argv[2]);
               }
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("openMovie <movObjId> <avi, mpg, mov file>");
         return "False";
      }

      //--------------------------------------
      case CloseMovie:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               mpObj->closeMovie();
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("closeMovie <movObjId>");
         return "False";
      }

      //--------------------------------------
      case PlayMovie:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               mpObj->play();
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("playMovie <movObjId>");
         return "False";
      }

      //--------------------------------------
      case PlayMovieToComp:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               mpObj->playToCompletion();
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("playMovieToComp <movObjId>");
         return "False";
      }

      //--------------------------------------
      case StopMovie:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               mpObj->stop();
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("stopMovie <movObjId>");
         return "False";
      }

      //--------------------------------------
      case PauseMovie:{
         if (argc == 2 )
         {
            MovPlay *mpObj = dynamic_cast<MovPlay*>(manager->findObject(argv[1]));
            if (mpObj)
            {
               mpObj->pause();
            }
            else
               console->printf("MovPlay %s doesn't exists!", argv[1]);
         }
         else
            console->printf("pauseMovie <movObjId>");
         return "False";
      }
   }
	return (0);
}

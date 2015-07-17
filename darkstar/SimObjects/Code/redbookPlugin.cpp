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
#include "redbook.h"
#include "simConsolePlugin.h"
#include "redbookPlugin.h"

//------------------------------------------------------------------------------
void RedbookPlugin::init()
{
   console->printf("RedbookPlugin");
   console->addCommand(NewRedbook,       "newRedbook",         this);
   console->addCommand(Open,             "rbOpen",             this);
   console->addCommand(Close,            "rbClose",            this);
   console->addCommand(Eject,            "rbEject",            this);
   console->addCommand(Retract,          "rbRetract",          this);
   console->addCommand(GetStatus,        "rbGetStatus",        this);
   console->addCommand(GetTrackCount,    "rbGetTrackCount",    this);
   console->addCommand(GetTrackInfo,     "rbGetTrackInfo",     this);
   console->addCommand(GetTrackPosition, "rbGetTrackPosition", this);
   console->addCommand(Play,             "rbPlay",             this);
   console->addCommand(Stop,             "rbStop",             this);
   console->addCommand(Pause,            "rbPause",            this);
   console->addCommand(Resume,           "rbResume",           this);
   console->addCommand(SetVolume,        "rbSetVolume",        this);
   console->addCommand(GetVolume,        "rbGetVolume",        this);
   console->addCommand(SetPlayMode,		  "rbSetPlayMode",		this);
}   

//----------------------------------------------------------------------------
void RedbookPlugin::startFrame()
{
}

//------------------------------------------------------------------------------
void RedbookPlugin::endFrame()
{
}

//----------------------------------------------------------------------------
const char *RedbookPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   if (!manager) return "False";

	switch(id) 
	{
      //--------------------------------------
 	   case NewRedbook:
         if (argc == 3)
         {
            SimCanvas *canvas = dynamic_cast<SimCanvas *>(manager->findObject(argv[2]));
            if (!canvas)
            {
               console->printf("invalid canvas: %s", argv[2]);
               return "False";
            }

            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (!rbo) 
            {
               rbo = new Redbook;
               manager->addObject(rbo);
               manager->assignName(rbo, argv[1]);
               
               if (rbo->open(canvas) && rbo->getStatus() == Redbook::STOPPED)
                  return "True";

					if (rbo->getStatus() != Redbook::STOPPED)
						rbo->forceUnacquire ();

               // else can't initialize
               manager->deleteObject(rbo);
#ifdef DEBUG
               console->printf("Err, could not open a device.  May be in use.");
#endif
            }
            else
               console->printf("The <obj Id> already exists");     
         }
         else
            console->printf("newRedbook <obj Id> <simCanvas>");
         return "False";

      //--------------------------------------
      case Open:
         if (argc == 3)
         {
            SimCanvas *canvas = dynamic_cast<SimCanvas *>(manager->findObject(argv[2]));
            if (!canvas)
            {
               console->printf("invalid canvas: %s", argv[2]);
               return "False";
            }
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               if (rbo->open(canvas) )
                  return "True";
               console->printf("Err, could not open a device.");
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbOpen <obj Id> <CD device ID> <simCanvas>");
         return "False";

      //--------------------------------------
      case Close:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->close();
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbClose <obj Id>");
         return "False";

      //--------------------------------------
      case Eject:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->eject();
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbEject <obj Id>");
         return "False";

      //--------------------------------------
      case Retract:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->retract();
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbRetract <obj Id>");
         return "False";

      //--------------------------------------
      case GetStatus:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               switch (rbo->getStatus())
               {
                  case Redbook::NO_DEVICE :
                     console->printf("No device mapped to Redbook object");
                     break;
                  case Redbook::DEVICE_NOT_READY :
                     console->printf("Device is not ready");
                     break;
                  case Redbook::TRAY_OPENED :
                     console->printf("CD tray is opened");
                     break;
                  case Redbook::PLAYING :
                     console->printf("CD is currently being played");
                     break;
                  case Redbook::STOPPED :
                     console->printf("CD is stopped");
                     break;
                  case Redbook::PAUSED :
                     console->printf("CD is paused");
                     break;

                  default:
                     console->printf("CD is in an unknown state!! - This is an error!!");
               }
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbGetCDStatus <obj Id>");
         return "False";
      
      //--------------------------------------
      case GetTrackCount:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               console->printf("Number of tracks: %d", rbo->getTrackCount());
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbGetTrackCount <obj Id>");
         return "False";


      //--------------------------------------
      case GetTrackInfo:
         if (argc == 3)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               Flt32 time;

               if ( rbo->getTrackInfo(atoi(argv[2]), &time) )
               {
                  console->printf("Track is %f seconds long", time );
                  return 0;
               }
               else
                  console->printf("Invalid track number");
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbGetTrackInfo <obj Id> <track num>");
         return "False";

      //--------------------------------------
      case GetTrackPosition:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               console->printf("Current track position is %f", rbo->getTrackPosition());
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbGetTrackPosition <obj Id>");
         return "False";

      //--------------------------------------
      case Play:
         if (argc == 3 || argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
					if (argc == 2 || rbo->getPlayMode() == 2)
						{
							rbo->setPlayMode (2);
							rbo->playAll ();
							return 0;
						}
               if ( rbo->play(atoi(argv[2])) )
                  return 0;
               else
                  console->printf("Invalid track number");
            }
#ifdef DEBUG
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
#endif
         }
         else
            console->printf("rbPlay <obj Id> <track num>");
         return "False";

      //--------------------------------------
      case Stop:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->stop();
               return 0;
            }
#ifdef DEBUG
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
#endif
         }
         else
            console->printf("rbStop <obj Id>");
         return "False";

      //--------------------------------------
      case Pause:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->pause();
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbPause <obj Id>");
         return "False";

      //--------------------------------------
      case Resume:
         if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               rbo->resume();
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbResume <obj Id>");
         return "False";

      //--------------------------------------
      case SetVolume:
	      if (argc == 3)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               Flt32 volume;

               volume = atof(argv[2]);
               if ((volume < 0.0f) || (volume > 1.0f))
               {
                  console->printf("volume must range between 0.0 and 1.0");
                  return "False";
               }
               rbo->setVolume(volume);
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbSetVolume <obj Id> <volume>");
         return "False";

      //--------------------------------------
      case GetVolume:
	      if (argc == 2)
         {
            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
            if (rbo)
            {
               console->printf("CD audio volume is %f", rbo->getVolume());
               return 0;
            }
            else
               console->printf("unable to find Redbook Obj %s", argv[1]);
         }
         else
            console->printf("rbGetVolume <obj Id>");
         return "False";

      //--------------------------------------
		case SetPlayMode:
			if (argc == 3)
				{
	            Redbook *rbo = dynamic_cast<Redbook*>(manager->findObject(argv[1]));
					if (rbo)
						{
							rbo->setPlayMode (atoi(argv[2]));
							return 0;
						}
#ifdef DEBUG
					else
						console->printf ("unable to find Redbook Obj %s", argv[1]);
#endif
				}
			else
				console->printf ("rbSetPlayMode <obj Id> <play mode [0=oneshot, 1=loop, 2=continuous]>");

			return "False";
   }
	return 0;
}

//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <soundFX.h>
#include <sim.h>
#include <simConsolePlugin.h>
#include <soundFXPlugin.h>
#include <SimTagDictionary.h>
#include <simSoundSequence.h>
#include <stringTable.h>

enum CallbackID {
   NewSfx,
   Open,
   Close,
   Query,
   SetFormat,
   GetFormat,
   Add2DProfile,
   Add3DProfile,
   AddPair,
   RemoveProfile,
   RemovePair,
   Play,
   Stop,
   Mute,
   SetPosition,
   SetPan,
   SetListenerPosition,
   SetVolume,
   SetMaxBuffers,
   GetMaxBuffers,
};



//------------------------------------------------------------------------------
void SoundFXPlugin::init()
{
   console->printf("SoundFXPlugin");
   console->addCommand(NewSfx,               "newSfx",                  this);
   console->addCommand(Open,                 "sfxOpen",                 this);
   console->addCommand(Close,                "sfxClose",                this);
   console->addCommand(Query,                "sfxQuery",                this);
   console->addCommand(Add2DProfile,         "sfxAdd2DProfile",         this);
   console->addCommand(Add3DProfile,         "sfxAdd3DProfile",         this);
   console->addCommand(AddPair,              "sfxAddPair",              this);
   console->addCommand(RemoveProfile,        "sfxRemoveProfile",        this);
   console->addCommand(RemovePair,           "sfxRemovePair",           this);
   console->addCommand(Play,                 "sfxPlay",                 this);
   console->addCommand(Stop,                 "sfxStop",                 this);
   console->addCommand(Mute,                 "sfxMute",                 this);
   console->addCommand(SetPosition,          "sfxSetPosition",          this);
   console->addCommand(SetPan,               "sfxSetPan",               this);
   console->addCommand(SetListenerPosition,  "sfxSetListenerPosition",  this);
   console->addCommand(SetFormat,            "sfxSetFormat",            this);
   console->addCommand(GetFormat,            "sfxGetFormat",            this);
   console->addCommand(SetVolume,            "sfxSetVolume",            this);
   console->addCommand(SetMaxBuffers,        "sfxSetMaxBuffers",        this);
   console->addCommand(GetMaxBuffers,        "sfxGetMaxBuffers",        this);

   console->addVariable(0, "pref::sfx2DVolume", CMDConsole::Unit, &Sfx::Manager::sm_2dVolumeAtten);
   console->addVariable(0, "pref::sfx3DVolume", CMDConsole::Unit, &Sfx::Manager::sm_3dVolumeAtten);
}


//----------------------------------------------------------------------------
void SoundFXPlugin::startFrame()
{
}

void SoundFXPlugin::endFrame()
{
#ifdef DEBUG
   if( manager )
   {
      Sfx::Manager *sfx = static_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
      if ( sfx )
      {
         Sfx::Metrics m;
         sfx->getMetrics( m );

         Console->setIntVariable("$SFXMetrics::msTimerPeriod",  m.msTimerPeriod );
         Console->setIntVariable("$SFXMetrics::nHardwareMixed", m.nHardwareMixed);
         Console->setIntVariable("$SFXMetrics::nSoftwareMixed", m.nSoftwareMixed);
         Console->setIntVariable("$SFXMetrics::nStreaming",     m.nStreaming    );
         Console->setIntVariable("$SFXMetrics::nStarved",       m.nStarved      );
         Console->setIntVariable("$SFXMetrics::nBuffers",       m.nBuffers      );
         Console->setVariable(   "$SFXMetrics::channels",       m.channels      );
      }
   }
#endif
}


//----------------------------------------------------------------------------
const char *SoundFXPlugin::consoleCallback(CMDConsole *, int id,int argc,const char *argv[])
{
   if (!manager) return "False";
   switch(id) 
   {
      //--------------------------------------
      case NewSfx:
         if (argc == 1 )
         {            
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (!sfx)
            {
               sfx = new Sfx::Manager;
               if (sfx)
               {
                  manager->addObject(sfx,SimSoundFXObjectId);
                  if( !sfx->open() )
                  {
                     CMDConsole::getLocked()->printf("Sound System: error opening sound device.");
                     return "False";
                  }
               }
            }

            SimSoundSequence *ss = dynamic_cast<SimSoundSequence*>(manager->findObject(SimSoundSequenceObjectId));
            if (!ss)
            {
               ss = new SimSoundSequence();
               manager->addObject(ss, SimSoundSequenceObjectId);
            }
            return 0;
         }
         else
            console->printf("newSfx");
         return "False";

      //--------------------------------------
      case Open:
         if (argc == 1 )
         {
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (sfx)
            {
               if( sfx->open() )
                  return (0);
               else
                  CMDConsole::getLocked()->printf("Sound System: error opening sound device.");
            }
            else
               CMDConsole::getLocked()->printf("Error: newSfx first.");
            return "False";
         }
         else
            console->printf("sfxOpen");
         return "False";

      //--------------------------------------
      case Close:
         if (argc == 1 )
         {
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (sfx)
            {
               sfx->close();
               return (0);
            }
            else
               CMDConsole::getLocked()->printf("Error: newSfx first.");
            return "False";
         }
         else
            console->printf("sfxClose");
         return "False";

      //--------------------------------------
      case Query:
         if (argc == 2 )
         {
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (sfx)
            {
               if (sfx->query(atoi(argv[1])))
               {
                  return "TRUE";
               }
               else
               {
                  return "FALSE";
               }
            }
            else
               CMDConsole::getLocked()->printf("Error: newSfx first.");
            return "False";
         }
         else
            console->printf("sfxClose(driverID);");
         return "False";

      //--------------------------------------
      case SetFormat:
         if (argc == 4)
         {
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (sfx)
            {
               int kHz     = atoi(argv[1]);
               int bits    = atoi(argv[2]);
               bool stereo = atoi(argv[3]);
               sfx->setFormat(kHz, bits, stereo);
               return (0);
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxSetFormat kHz bits bool(stereo)");
         return "False";

      //--------------------------------------
      case GetFormat:
         if (argc == 1)
         {
            Sfx::Manager *sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (sfx)
            {
               DWORD kHz;
               WORD  bits;
               bool stereo;
               sfx->getFormat(kHz, bits, stereo);
               console->printf("Sfx::Manager %dkHz %dbit %s", kHz, bits, stereo ? "stereo" : "mono");
               return (0);
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxGetFormat");
         return "False";

      //--------------------------------------
      case Add2DProfile:
         if (argc >= 3 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( !sfx )
               return (0);
            
            Sfx::Profile *profile = new Sfx::Profile;
            profile->id          = atoi(argv[1] );
            if (profile->id == 0)
            {
               delete profile;
               console->printf("sfxAdd3DProfile: bad tag (%s)", argv[1]);
               return (0);
            }

            profile->flags       = 0;
            profile->baseVolume  = atof( argv[2] );
            profile->coneInsideAngle = 90;
            profile->coneOutsideAngle= 180; 
            profile->coneVector.set(0.0f, -1.0f, 0.0f);
            profile->coneOutsideVolume = 0.0f;  
            profile->minDistance       = 30.0f;     
            profile->maxDistance       = 300.0f;     

            for ( int i=3; i < argc; i++)
            {
               if ( stricmp(argv[i], "SFX_IS_LOOPING" ) == 0)   profile->flags |= SFX_IS_LOOPING;
               if ( stricmp(argv[i], "SFX_IS_STREAMING" ) == 0) profile->flags |= SFX_IS_STREAMING;
               if ( stricmp(argv[i], "SFX_IS_PITCHING" ) == 0)  profile->flags |= SFX_IS_PITCHING;
            }  

            sfx->addProfile( profile ); 
         }
         else
            console->printf("sfxAdd2DProfile( id, baseVolume, [flags] )");
         break;

      //--------------------------------------
      case Add3DProfile:
         if (argc >= 5 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if (!sfx)
               return (0);   

            Sfx::Profile *profile = new Sfx::Profile;
            profile->id = atoi ( argv[1] );
            if (profile->id == 0)
            {
               delete profile;
               console->printf("sfxAdd3DProfile: bad tag (%s)", argv[1]);
               return (0);
            }

            profile->flags       = 0;
            profile->baseVolume  = atof( argv[2] );
            profile->minDistance = atof( argv[3] );
            profile->maxDistance = atof( argv[4] );

            int flagStart;
            if (argc >= 11 && isdigit(*argv[5]))
            {
               flagStart = 11;
               profile->coneInsideAngle = atof(argv[5]);
               profile->coneOutsideAngle= atof(argv[6]); 
               profile->coneVector.x    = atof(argv[7]);
               profile->coneVector.y    = atof(argv[8]);
               profile->coneVector.z    = atof(argv[9]);
               profile->coneOutsideVolume = atof(argv[10]);  
               profile->coneVector.normalize();
            }
            else
            {
               flagStart = 5;
               profile->coneInsideAngle = 90;
               profile->coneOutsideAngle= 180; 
               profile->coneVector.x    =  0.0f;
               profile->coneVector.y    = -1.0f;
               profile->coneVector.z    =  0.0f;
               profile->coneOutsideVolume = 0.0f;  
            }
            profile->flags |= SFX_IS_HARDWARE_3D;
            for ( int i=flagStart; i < argc; i++)
            {
               if ( stricmp(argv[i], "SFX_IS_LOOPING" ) == 0)   profile->flags |= SFX_IS_LOOPING;
               if ( stricmp(argv[i], "SFX_IS_STREAMING" ) == 0) profile->flags |= SFX_IS_STREAMING;
               if ( stricmp(argv[i], "SFX_IS_PITCHING" ) == 0)  profile->flags |= SFX_IS_PITCHING;
               if ( stricmp(argv[i], "SFX_IS_HARDWARE_3D" ) == 0)  profile->flags |= SFX_IS_HARDWARE_3D;
               if ( stricmp(argv[i], "SFX_IS_SOFTWARE_3D" ) == 0)  
               {
                  profile->flags &= ~SFX_IS_HARDWARE_3D;
                  profile->flags |= SFX_IS_SOFTWARE_3D;
               }
            }  

            sfx->addProfile( profile );
         }
         else
            console->printf("sfxAdd3DProfile( id, baseVolume, minDist, maxDist, [coneVolume, coneInside, coneOutside, vector.x, vector.y, vector.z,] [flags]");
         break;

      //--------------------------------------
      case AddPair:
         if (argc == 4 || argc == 5)
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( !sfx )
               return (0);
            
            Sfx::Pair *pair = new Sfx::Pair;
            pair->id        = atoi( argv[1] );
            pair->profileId = atoi( argv[2] );
            pair->name      = stringTable.insert(argv[3]);
            pair->priority  = (argc == 5) ? atof(argv[4]) : 0.0f;

            sfx->addPair( pair );
         }
         else
            console->printf("sfxAddPair( id, profileId, wav_name. [0.0-1.0 priority] )");
         break;

      //--------------------------------------
      case Play:
         if (argc == 2 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               SFX_HANDLE h = sfx->play( atoi( argv[1] ) );
               console->printf("sfxPlay handle = %d", h);
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxPlay id");
         break;

      //--------------------------------------
      case Stop:
         if (argc == 2 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               sfx->selectHandle( (SFX_HANDLE)atoi( argv[1] ) ); 
               sfx->stop(); 
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxStop id");
         break;

      //--------------------------------------
      case SetVolume:
         if ( argc == 2 || argc == 3 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               float value;
               sscanf(argv[1], "%f", &value);
               if (argc == 2)
               {
                  sfx->setMasterVolume(value);
                  console->printf("Sfx master volume set to %3.2f", value);
               }
               else
               {
                  sfx->selectHandle( (SFX_HANDLE)atoi( argv[2] ) ); 
                  sfx->setVolume(value); 
               }
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxSetVolume float {SFX_HANDLE}");
         break;

      //--------------------------------------
      case SetPosition:
         if (argc == 5 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               TMat3F tmat;
               if (sfx->selectHandle( (SFX_HANDLE)atoi( argv[1] ) ) != SFX_INVALID_HANDLE)
               {
                  sscanf(argv[2], "%f", &tmat.p.x);
                  sscanf(argv[3], "%f", &tmat.p.y);
                  sscanf(argv[4], "%f", &tmat.p.z);
                  sfx->setTransform( tmat, Vector3F(0,0,0) ); 
               }
               else
                  console->printf("unable to find buffer id");
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxSetPosition id x y z");
         break;

      //--------------------------------------
      case SetPan:
         if (argc == 5 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               float pan;
               if (sfx->selectHandle( (SFX_HANDLE)atoi( argv[1] ) ) != SFX_INVALID_HANDLE)
               {
                  sscanf(argv[2], "%f", &pan);
                  sfx->setPan( pan ); 
               }
               else
                  console->printf("unable to find buffer id");
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxSetPan id fPan(-1.0 to +1.0)");
         break;

      //--------------------------------------
      case Mute:
         if (argc == 2)
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               bool onOff = stricmp(argv[1], "true") == 0;
               sfx->mute(onOff);
            }
            else
               console->printf("sfxMute( true/false );");
         }
         break;

      //--------------------------------------
      case SetListenerPosition:
         if (argc == 4 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
            {
               TMat3F tmat;
               sscanf(argv[1], "%f", &tmat.p.x);
               sscanf(argv[2], "%f", &tmat.p.y);
               sscanf(argv[3], "%f", &tmat.p.z);
               sfx->setTransform( tmat, Vector3F(0,0,0) ); 
            }
            else
               console->printf("unable to find Sound object");
         }
         else
            console->printf("sfxSetListenerPosition x y z");
         break;

      //--------------------------------------
      case SetMaxBuffers:
         if (argc >= 2 )
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
               sfx->setMaxBuffers( atoi(argv[1]) );
            else
               console->printf("unable to find Sound object");
         }
         break;
      
      //--------------------------------------
      case GetMaxBuffers:
         {
            Sfx::Manager * sfx = dynamic_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
            if ( sfx )
               console->printf("Sfx::Manager::maxBuffers = %d", sfx->getMaxBuffers() );
            else
               console->printf("unable to find Sound object");
         }
         break;

   }
   return 0;
}


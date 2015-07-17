//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "redbook.h"
#include "console.h"
#include "gwBase.h"

#define MILLISEC_TO_SEC(a)    ((a) / 1000.0f)


Flt32 Redbook::originalVolume = 0.0f;
Flt32 Redbook::desiredVolume = 0.0f;

bool  Redbook::volumeInitialized = false; 
Int32 Redbook::numDevicesOpened  = 0;      // number of devices opened
                                           // by all redbook objects

bool  Redbook::usingMixer        = false;  
Int32 Redbook::vol_device_id     = -1;     // holds HMIXER or an AUX device

MIXERCONTROLDETAILS          Redbook::volumeDetails;
MIXERCONTROLDETAILS_UNSIGNED Redbook::volumeValue;



//------------------------------------------------------------------------------
Redbook::Redbook()
{
   device_id = 0;
   deviceAcquired = false;
   appHWND = 0;

	playMode = MODE_ONESHOT;
}   

//------------------------------------------------------------------------------
bool Redbook::onAdd()
{
   if (!Parent::onAdd())
      return false;

   SimSet *redBookSet = dynamic_cast<SimSet *>(manager->findObject(SimRedbookSetId));
   if (!redBookSet)
   {
      redBookSet = new SimSet();
      AssertFatal(redBookSet, "could not create SimSet");
      manager->addObject(redBookSet);
      manager->assignId(redBookSet, SimRedbookSetId);
   }
      
   redBookSet->addObject(this);   
   return true;   
}   

//------------------------------------------------------------------------------
void Redbook::onRemove()
{
   if (deviceAcquired)
      close();
   Parent::onRemove();
}

//------------------------------------------------------------------------------
void Redbook::onPlayStopped ()
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;

   if (! deviceAcquired)
      return;

   mci_status_parm.dwItem = MCI_STATUS_MODE;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
      return;

	if (mci_status_parm.dwReturn == MCI_MODE_PLAY)
		return;

	switch (playMode)
		{
			case MODE_REPEAT:
				play (currentTrack);
				break;

			case MODE_CONTINUOUS:
				playAll ();
				break;
		};
}


//------------------------------------------------------------------------------
bool Redbook::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      case SimMCINotifyEventType: {
         const SimMCINotifyEvent *ev = static_cast<const SimMCINotifyEvent *>(event);
         if ((MCIDEVICEID)ev->devId == device_id)
         {
            // the play function is complete
            // so, update current state
#ifdef DEBUG
            Console->printf("Done playing");
#endif
            getStatus();  
            onPlayStopped();
         }
         break;
      }
      case SimMIXNotifyEventType:
         desiredVolume = getVolume();
         break;
   }
   
   return true;
}

//------------------------------------------------------------------------------
// Attempts to open the specified CD device.  If it fails, then it attempts
// to open any accessible CD device.   
//
// A CD audio device must be mapped to this object 
// before any of the other operations can be applied. 
bool Redbook::open(SimCanvas *canvas)
{
   AssertFatal(canvas, "invalid parameter0");
   appHWND = canvas->getHandle();

   MCI_OPEN_PARMS mci_open_parms;
   MCI_SET_PARMS mci_set_parms;
   
   if (deviceAcquired)
   {
      AssertWarn(0, "Each redbook object can only open one device at a time");
      return false;
   }

   DWORD errorId;
   mci_open_parms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
   
   // first try to open a CD device and don't share it
   errorId = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID, (DWORD)(LPMCI_OPEN_PARMS)&mci_open_parms);
   if (errorId)
   {
      reportError(errorId);
      // next, try to get a shared CD device.
      errorId = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_SHAREABLE, (DWORD)(LPMCI_OPEN_PARMS)&mci_open_parms);
      if (errorId)
      {
         reportError(errorId);
         // failed to get a CD device
         return false;
      }
   }

   device_id = mci_open_parms.wDeviceID;
   deviceAcquired = true;
   paused = false;

   if (! volumeInitialized)
      initVolume();

   numDevicesOpened++;

   // set CD device to report time in millisecods
   mci_set_parms.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
   errorId = mciSendCommand(mci_open_parms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPMCI_SET_PARMS)&mci_set_parms);
   if (errorId)
   {
      reportError(errorId);
      close();
      return false;
   }

   return true;
}   

//------------------------------------------------------------------------------
void Redbook::close()
{
   DWORD errorId;
   MCI_GENERIC_PARMS mci_close_parms;

   if (! deviceAcquired)
      return;

   stop();

   errorId = mciSendCommand(device_id, MCI_CLOSE, 0, (DWORD)(LPMCI_GENERIC_PARMS)&mci_close_parms);
   if (errorId)
      reportError(errorId);

   device_id = 0;
   deviceAcquired = false;

   numDevicesOpened--;
   if (volumeInitialized && numDevicesOpened <= 0)
   {
      setVolume(originalVolume);
      volumeInitialized = false;
      if (usingMixer)
         mixerClose((HMIXER)vol_device_id);
   }
}  

//------------------------------------------------------------------------------
void Redbook::eject()
{
   DWORD errorId;
   MCI_SET_PARMS mci_set_parms;

   errorId = mciSendCommand(device_id, MCI_SET, MCI_SET_DOOR_OPEN, (DWORD)(LPMCI_SET_PARMS)&mci_set_parms);
   if (errorId)
     reportError(errorId);
} 

//------------------------------------------------------------------------------
void Redbook::retract()
{
   DWORD errorId;
   MCI_SET_PARMS mci_set_parms;

   errorId = mciSendCommand(device_id, MCI_SET, MCI_SET_DOOR_CLOSED, (DWORD)(LPMCI_SET_PARMS)&mci_set_parms);   
   if (errorId)
     reportError(errorId);
} 

//------------------------------------------------------------------------------
Redbook::Status Redbook::getStatus()
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;

   if (! deviceAcquired)
   {
      paused = false;
      return NO_DEVICE;
   }

   mci_status_parm.dwItem = MCI_STATUS_MODE;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      paused = false;
      return DEVICE_NOT_READY;
   }

   switch (mci_status_parm.dwReturn)
   {
      case MCI_MODE_PAUSE:
         paused = true;
         return PAUSED;

      case MCI_MODE_PLAY:
         paused = false;
         return PLAYING;
      
      case MCI_MODE_STOP:
         if (paused)
            return PAUSED;
         else
            return STOPPED;
      
      case MCI_MODE_OPEN:
         paused = false;
         return TRAY_OPENED;
      
      default:
         paused = false;
         return DEVICE_NOT_READY;
   }
} 

//------------------------------------------------------------------------------
// Returns the position of the current track in seconds.
Flt32 Redbook::getTrackPosition()
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;
   UInt32 curGlobalPos;
   Flt32 curPos;

   switch (getStatus())
   {
      case PAUSED:
         return MILLISEC_TO_SEC(pausedTime_ms - startTime_ms);

      case PLAYING:
         // get the current global position on the CD
         mci_status_parm.dwItem = MCI_STATUS_POSITION;
         errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
         if (errorId)
         {
            reportError(errorId);
            return 0.0f;
         }
         curGlobalPos = mci_status_parm.dwReturn;

         // return the position in the current track
         curPos = MILLISEC_TO_SEC(curGlobalPos - startTime_ms); 
         // account for error when seeking disc prior to actually
         // playing the disc.  Assume, no song is longer then 73 minutes
         if (curPos > 4380.0f)
            curPos = 0.0f;
         return curPos;
   }

   return 0.0f;
}   

//------------------------------------------------------------------------------
// Returns the number of tracks on the CD
UInt32 Redbook::getTrackCount()
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;

   mci_status_parm.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;

   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return 0;
   }

   return mci_status_parm.dwReturn;
}

//------------------------------------------------------------------------------
bool Redbook::getTrackInfo(UInt32 track, Flt32 *lengthSeconds)
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;

   *lengthSeconds = 0;

   mci_status_parm.dwItem = MCI_STATUS_LENGTH;
   mci_status_parm.dwTrack = track;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }

   *lengthSeconds = MILLISEC_TO_SEC(mci_status_parm.dwReturn);

   return true;
}

//------------------------------------------------------------------------------
bool Redbook::play(UInt32 track)
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;
   MCI_PLAY_PARMS mci_play_parms;

   paused = false;

   // get start time of track
   mci_status_parm.dwItem = MCI_STATUS_POSITION;
   mci_status_parm.dwTrack = track;
	currentTrack = track;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }
   mci_play_parms.dwFrom = mci_status_parm.dwReturn;

   // get end time of track
   mci_status_parm.dwItem = MCI_STATUS_LENGTH;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }
   mci_play_parms.dwTo = mci_play_parms.dwFrom + mci_status_parm.dwReturn;
   mci_play_parms.dwCallback = MAKELONG(appHWND, 0);

   // try to play the track
   errorId = mciSendCommand(device_id, MCI_PLAY, MCI_FROM|MCI_TO|MCI_NOTIFY, (DWORD)(LPMCI_PLAY_PARMS)&mci_play_parms);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }

   // preserve state info on current track
   startTime_ms = pausedTime_ms = mci_play_parms.dwFrom;
   endTime_ms = mci_play_parms.dwTo;
   
   return true;
}  

//------------------------------------------------------------------------------
bool Redbook::playAll(void)
{
   DWORD errorId;
   MCI_STATUS_PARMS mci_status_parm;
   MCI_PLAY_PARMS mci_play_parms;

   paused = false;

   // get start time of track
   mci_status_parm.dwItem = MCI_STATUS_POSITION;
   mci_status_parm.dwTrack = 1;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }
   mci_play_parms.dwFrom = mci_status_parm.dwReturn;
	long time = mci_play_parms.dwFrom;

	for (UInt32 i = 1; i < getTrackCount(); i++)
		{
		   // get end time of track
		   mci_status_parm.dwItem = MCI_STATUS_LENGTH;
		   mci_status_parm.dwTrack = i;
		   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
		   if (errorId)
		   {
		      reportError(errorId);
		      return false;
		   }
			time += mci_status_parm.dwReturn;
		}

   mci_play_parms.dwTo = time;
   mci_play_parms.dwCallback = MAKELONG(appHWND, 0);

   // try to play the track
   errorId = mciSendCommand(device_id, MCI_PLAY, MCI_FROM|MCI_TO|MCI_NOTIFY, (DWORD)(LPMCI_PLAY_PARMS)&mci_play_parms);
   if (errorId)
   {
      reportError(errorId);
      return false;
   }

   // preserve state info on current track
   startTime_ms = pausedTime_ms = mci_play_parms.dwFrom;
   endTime_ms = time;
   
   return true;
}  

//------------------------------------------------------------------------------
void Redbook::stop()
{
   DWORD errorId;
   MCI_GENERIC_PARMS mci_gen_parms;

   errorId = mciSendCommand(device_id, MCI_STOP, 0, (DWORD)(LPMCI_GENERIC_PARMS)&mci_gen_parms);
   if (errorId)
     reportError(errorId);
   
   paused = false;
}  

//------------------------------------------------------------------------------
void Redbook::pause()
{
   DWORD errorId;
   MCI_GENERIC_PARMS mci_gen_parms;
   MCI_STATUS_PARMS mci_status_parm;

   if (!isPlaying())
      return;

   mci_status_parm.dwItem = MCI_STATUS_POSITION;
   errorId = mciSendCommand(device_id, MCI_STATUS, MCI_STATUS_ITEM|MCI_WAIT, (DWORD)(LPMCI_STATUS_PARMS)&mci_status_parm);
   if (errorId)
   {
      reportError(errorId);
      return;
   }

   errorId = mciSendCommand(device_id, MCI_PAUSE, 0, (DWORD)(LPMCI_GENERIC_PARMS)&mci_gen_parms);
   if (errorId)
   {
      reportError(errorId);
      return;
   }

   paused = true;
   pausedTime_ms = mci_status_parm.dwReturn;
}  

//------------------------------------------------------------------------------
void Redbook::resume()
{
   DWORD errorId;
   MCI_PLAY_PARMS mci_play_parms;

   if (!isPaused())
      return;

   mci_play_parms.dwFrom = pausedTime_ms;
   mci_play_parms.dwTo = endTime_ms;
   mci_play_parms.dwCallback = MAKELONG(appHWND, 0);

   // try to play the track
   errorId = mciSendCommand(device_id, MCI_PLAY, MCI_FROM|MCI_TO|MCI_NOTIFY, (DWORD)(LPMCI_PLAY_PARMS)&mci_play_parms);
   if (errorId)
     reportError(errorId);

   paused = false;
}  



//------------------------------------------------------------------------------
// Vol should range between 0.0f and 1.0f
// 0.0f is full volume, and 1.0f is silence
void Redbook::adjustVolume(Flt32 vol)
{
   if (! volumeInitialized)
      return;

   if ((vol >= 0.0f) && (vol<=1.0f))
   {
      vol = 1.0f - vol;
      UInt32 vol_int = vol * 127 * 516;

      if (usingMixer) {
         volumeValue.dwValue = vol_int;
         mixerSetControlDetails((HMIXEROBJ)vol_device_id, &volumeDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      } 
      else
      {
         vol_int |= (vol_int << 16);
         auxSetVolume((UInt32)vol_device_id, vol_int);
      }
   }
}  

//------------------------------------------------------------------------------
void Redbook::setVolume(Flt32 vol)
{
   adjustVolume(vol);
   desiredVolume = vol;   
}  
 
//------------------------------------------------------------------------------
// The returned value will range between 0.0f and 1.0f
// 0.0f is full volume, and 1.0f is silence
Flt32 Redbook::getVolume()
{
   if (! volumeInitialized)
      return 1.0f;

   UInt32 vol_int;

   if (usingMixer) 
   {
      mixerGetControlDetails((HMIXEROBJ)vol_device_id, &volumeDetails, MIXER_SETCONTROLDETAILSF_VALUE);
      vol_int = (volumeValue.dwValue&0xffff) >> 9;
   }
   else
   {
      if (auxGetVolume((UInt32)vol_device_id, &vol_int) == 0)
         vol_int = (vol_int&0xffff) >> 9 ;
      else
         vol_int = 0;
   }

   Flt32 vol = 1.0f - vol_int / 127.0f;
   vol = (vol > 1.0f) ? 1.0f : vol;
   vol = (vol < 0.0f) ? 0.0f : vol;

   return vol;
}  

//------------------------------------------------------------------------------
void Redbook::initVolume()
{
   MIXERLINE         lineInfo;
   MIXERLINECONTROLS lineControls;
   MIXERCONTROL      volumeControl;

   AUXCAPS Caps;
   Int32 i;

   if (volumeInitialized)
      return;

   // first attempt to get the volume control through the mixer API
   for(i=mixerGetNumDevs() - 1; i >= 0; i--) 
   {
      //if (mixerOpen((HMIXER*) &vol_device_id, i, MAKELONG(appHWND, 0), 0, 0) == MMSYSERR_NOERROR) 
      if (mixerOpen((HMIXER*) &vol_device_id, i, (DWORD)appHWND, 0, CALLBACK_WINDOW) == MMSYSERR_NOERROR) 
      {
         memset(&lineInfo, 0, sizeof(lineInfo));
         lineInfo.cbStruct = sizeof(lineInfo);
         lineInfo.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

         if (mixerGetLineInfo((HMIXEROBJ)vol_device_id, &lineInfo, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR) 
         {
            memset(&lineControls, 0, sizeof(lineControls));
            memset(&volumeControl, 0, sizeof(volumeControl));
            lineControls.cbStruct = sizeof(lineControls);
            lineControls.dwLineID = lineInfo.dwLineID;
            lineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
            lineControls.cControls = 1;
            lineControls.cbmxctrl = sizeof(volumeControl);
            lineControls.pamxctrl = &volumeControl;

            if (mixerGetLineControls((HMIXEROBJ)vol_device_id, &lineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR)
            {
               memset(&volumeDetails, 0, sizeof(volumeDetails));
               memset(&volumeValue, 0, sizeof(volumeValue));
               volumeDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
               volumeDetails.dwControlID = volumeControl.dwControlID;
               volumeDetails.cChannels = 1;
               volumeDetails.cbDetails = sizeof(volumeValue);
               volumeDetails.paDetails = &volumeValue;

               if (mixerGetControlDetails((HMIXEROBJ)vol_device_id, &volumeDetails, MIXER_SETCONTROLDETAILSF_VALUE) == MMSYSERR_NOERROR) 
               {
                  usingMixer = true;
                  volumeInitialized = true;
                  originalVolume = getVolume();
                  desiredVolume = originalVolume;
                  return;
               }
            }
         }
         mixerClose((HMIXER)vol_device_id);
         vol_device_id = -1;
      }
   }

   // if the mixer API attempt failed, get the aux volume control
   if (vol_device_id == -1)
   {
      for(i=auxGetNumDevs() - 1; i >= 0; i--) 
      {
         auxGetDevCaps(i, &Caps, sizeof(Caps));
         if ((Caps.wTechnology == AUXCAPS_CDAUDIO) && (Caps.dwSupport & AUXCAPS_VOLUME)) 
         {
            vol_device_id = i;
            usingMixer = false;
            volumeInitialized = true;
            originalVolume = getVolume();
            desiredVolume = originalVolume;
            return;
         }
      }
   }
}

//------------------------------------------------------------------------------
void Redbook::reportError(DWORD errorId)
{
#ifdef DEBUG
   char buffer[129];
   if ( mciGetErrorString(errorId, buffer, 128) )
      Console->printf(buffer);
#else
	errorId;
#endif
}   

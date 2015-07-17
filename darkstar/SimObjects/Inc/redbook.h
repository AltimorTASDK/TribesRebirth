//------------------------------------------------------------------------------
// Description 
//    SimObject for dealing with Redbook CD audio.
//
// NOTE:
//    A CD audio device must be mapped to this object via the open() call 
//    before any of the other operations can be applied. 
//
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _REDBOOK_H_
#define _REDBOOK_H_


#include <windows.h>
#include <mmsystem.h>
#include <sim.h>
#include <simCanvas.h>


class Redbook : public SimObject 
{
   typedef SimObject Parent;

protected:
   MCIDEVICEID device_id;
   HWND appHWND;
   bool deviceAcquired;

   // state information about the current track being played
   bool   paused;
   UInt32 startTime_ms;  // in milliseconds
   UInt32 pausedTime_ms; 
   UInt32 endTime_ms;

	enum PlayModes	{
							MODE_ONESHOT  = 0,
							MODE_REPEAT,
							MODE_CONTINUOUS,
						};
	
	int playMode;
	int currentTrack;

   // volume stuff
   static Flt32 originalVolume;     // Mixer volume setting of windows before any
                                    // any redbook object is created.  This volume
                                    // is restored once all redbook objects are deleted

   static Flt32 desiredVolume;      // Volume desired by our app.   

   static bool  volumeInitialized;  // whether volume maniplation can be done

   static Int32 numDevicesOpened;   // number of devices opened by all redbook objects
                                    // When, it reaches zero, the original volume is restored

   static bool  usingMixer;         // whether we're using the MIXER or AUX device for volume control
   static Int32 vol_device_id;      // holds HMIXER or an AUX device

   static MIXERCONTROLDETAILS          volumeDetails; // state variables for mixer
   static MIXERCONTROLDETAILS_UNSIGNED volumeValue;

   void initVolume();
   void adjustVolume(Flt32 vol);  // vol should range between 0 and 1
   
   bool onAdd();
   void onRemove();                                  
   void reportError(DWORD errorId);

public:
   enum Status { NO_DEVICE, DEVICE_NOT_READY, PLAYING, STOPPED, PAUSED, TRAY_OPENED };

   Redbook();

   bool open(SimCanvas *canvas);
   void close();

   void eject();
   void retract();

	void forceUnacquire (void) {deviceAcquired = false;}
   
   Redbook::Status getStatus();

   bool isPlaying();
   bool isStopped();
   bool isPaused();

	void setPlayMode (int mode) {playMode = mode;}
	int getPlayMode (void) {return playMode;}

   UInt32 getTrackCount();
   bool   getTrackInfo(UInt32 track, Flt32 *lenghtSeconds);
   Flt32  getTrackPosition();

   virtual bool play(UInt32 track);
   virtual bool playAll(void);
   virtual void stop();
   virtual void pause();
   virtual void resume();
   virtual void onPlayStopped();

   void  setVolume(Flt32 vol);  // vol should range between 0 and 1
   Flt32 getVolume();           // 0 == full volume and 1 == silence

   bool processEvent(const SimEvent*);
};


//------------------------------------------------------------------------------
// Inline implemenations for RedbookObj.
//------------------------------------------------------------------------------

inline bool Redbook::isPlaying()
{
   return (getStatus() == PLAYING);
}

inline bool Redbook::isStopped()
{
   return (getStatus() == STOPPED);
}

inline bool Redbook::isPaused()
{
   return (getStatus() == PAUSED);
}

#endif //_REDBOOK_H_

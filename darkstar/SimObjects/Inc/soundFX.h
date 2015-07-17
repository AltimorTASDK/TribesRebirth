//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSFX_H_
#define _SIMSFX_H_


#include <sim.h>
#include <mmsystem.h>
#include <dSound.h>
#include <ia3d.h>
#include <threadBase.h>

typedef void* SFX_HANDLE;
#define SFX_INVALID_HANDLE ((SFX_HANDLE)0)


// SoundBuffer flags
#define SFX_IS_LOOPING           (1<< 1)  // enables continuous looping
#define SFX_IS_STREAMING         (1<< 2)  // enables streaming
#define SFX_IS_PITCHING          (1<< 3)  // enables streaming
#define SFX_IS_SOFTWARE_3D       (1<< 4)  // internal use only
#define SFX_IS_HARDWARE_3D       (1<< 5)  // internal use only
#define SFX_IS_3D                (SFX_IS_SOFTWARE_3D|SFX_IS_HARDWARE_3D)
// INTERNAL USE ONLY : SimSoundFX / SoundBuffer flags
#define SFX_USER_MASK            (0x00ff) // internal use only
#define SFX_INTERNAL_MASK        (0xff00) // internal use only
#define SFX_OWN_MEMORY           (1<<16)  // internal use only
#define SFX_HARDWARE_BUFFER      (1<<17)  // internal use only
#define SFX_2D_BUFFER_CHANGE     (1<<20)  // internal use only
#define SFX_3D_BUFFER_CHANGE     (1<<21)  // internal use only
#define SFX_3D_LISTENER_CHANGE   (1<<22)  // internal use only
#define SFX_PLAYING              (1<<23)  // internal use only
#define SFX_STOPPED              (1<<24)  // internal use only
#define SFX_SUSPENDED            (1<<25)  // stopped but not done
#define SFX_TRANSITION           (1<<26)  // suspended->playing or playing->suspended
#define SFX_PURGE                (1<<27)  // internal use only
#define SFX_FILL_BUFFER          (1<<28)  // internal use only

#define SFX_BUFFER_LENGTH        (2000)  // ms length of streaming buffer
#define SFX_TIMER_PERIOD         (SFX_BUFFER_LENGTH/4)  // ms between buffer updates
#define SFX_TIMER_ADJUST         (50)    // ms time adjust if starving detected

#define SFX_QUERY_DSOUND         1
#define SFX_QUERY_DSOUND3D       2
#define SFX_QUERY_AUREAL3D       3
#define SFX_QUERY_LIVE3D         4

namespace Sfx
{

//------------------------------------------------------------------------------
struct Metrics
{
   DWORD nHardwareMixed;
   DWORD nSoftwareMixed;
   DWORD nStreaming;
   DWORD nStarved;
   DWORD nBuffers;
   DWORD msTimerPeriod;
   char  channels[256];
};

  
//------------------------------------------------------------------------------
struct Profile
{
   DWORD id;                     // profile identifier
   DWORD flags;                  // looping, streaming, etc
   float baseVolume;             // base volume 

   //-------------------------------------- 3D information
   DWORD     coneInsideAngle;    // degrees 0-360
   DWORD     coneOutsideAngle;   // degrees 0-360
   Vector3F  coneVector;
   float     coneOutsideVolume;  // max attenuation 1.0 to 0.0 no attenuation
   float     minDistance;        // in YOUR world units
   float     maxDistance;        // in YOUR world units
};   


//------------------------------------------------------------------------------
struct Pair
{
   DWORD id;                     // pair identifer used to play(id, ...)
   DWORD profileId;              // profile of sound, how to play it.
   float priority;               // 1.0 highest priority, 0.0 use distance as priority
   const char  *name;
};


//------------------------------------------------------------------------------
class DLLAPI Base
{
protected:
   static SimManager *manager;
   
   static DWORD soundFlags;
   static DWORD caps;
   static DWORD hwMemory;
   static DWORD nHardwareMixed;
   static DWORD nSoftwareMixed;
   static DWORD nStreaming;
   static DWORD nStarved;
   static DWORD nBuffers;
   static DWORD msTimerPeriod;
   static TMat3F   listenerTransform;
   static Vector3F listenerVelocity;
   static Vector3F listenerVector;

   static IDirectSound            *pDirectSound;
   static IDirectSoundBuffer      *pPrimary;
   static IDirectSound3DListener  *pListener;
};


//------------------------------------------------------------------------------
class DLLAPI SoundBuffer: public Base
{
private:
   static DWORD nextHandle;
   SFX_HANDLE getNextHandle();

   IDirectSoundBuffer   *pBuffer;
   IDirectSound3DBuffer *p3DBuffer;
   
   SFX_HANDLE handle;
   Profile profile;
   float priority;

   const char   *name;
   DWORD  flags;
   TMat3F transform;
   Vector3F velocity;
   int      time;

   DWORD bufferSize;
   DWORD wavePosition;
   char* pWavePosition;
   DWORD writeCursor;
   LONG  csFill;

   WAVEFORMATEX *format;
   char *waveData;
   DWORD waveSize;

   float pan;      // -1.0 to 1,0  -0.5 = 50% right channel attenuation
   float volume;   // 0.0 to 1.0   0.0 = 0db attenuation  1.0 = 100% attenuation (silence)
   float targetVolume;
   INT frequency;  // 100 to 100,000Hz

   void  closeWav();
   bool  openWav();
   bool  restoreBuffer(SoundBuffer *copy=NULL);
   DWORD getFillSize();
   void  handleFill();
   void  adjStopFlags();
   BYTE  getSilence();
   float estimateVolume();
   void  commit2D();
   void  commit3D();
   IDirectSoundBuffer* duplicate();

public:
   SoundBuffer();
   ~SoundBuffer();

   bool open(const char *name, float priority, const Profile *prof, SoundBuffer *copy = NULL);
   void close();
   SFX_HANDLE getHandle()           { return handle; }
   SFX_HANDLE generateNewHandle()   { return handle = getNextHandle(); }
   const char *getName()            { return name; }
   DWORD getProfileID()             { return profile.id; }
   float getPriority() const;       

   void updateStreaming();
   void setFlag(DWORD f)    { flags |= (f & SFX_USER_MASK); }
   void clearFlag(DWORD f)  { flags &= ~(f & SFX_USER_MASK); }  
   DWORD getFlags()         { return flags; }

   void play();
   void stop();
   void suspend();
   void setPan(float p)     { pan = p; flags |= SFX_2D_BUFFER_CHANGE; } 
   void setVolume(float v)  {
      volume = profile.baseVolume + v;
      if (volume > 1.0f)
         volume = 1.0f;
      flags |= SFX_2D_BUFFER_CHANGE;
   }
   void setFrequency(INT f) { frequency = f; flags |= SFX_2D_BUFFER_CHANGE; }
   void setTransform(const TMat3F &tmat, const Vector3F &vel);
   void commit();

   // used for sound clipping
   int   getTime() const { return (time); }
   float getVolume()    { return (volume); }
   bool  isPlaying()    { return (flags & SFX_PLAYING); }
   bool  isLooping()    { return (flags & SFX_IS_LOOPING); }
   bool  isStopped()    { return (flags & SFX_STOPPED); }
   bool  isSuspended()  { return (flags & SFX_SUSPENDED); }
   bool  isPurgeable()  { return (flags & SFX_PURGE); }
};   



//------------------------------------------------------------------------------
class DLLAPI Manager: public SimObject, public Base, public ThreadBase
{
//--------------------------------------
private:
   typedef SimObject Parent;
   typedef HRESULT (WINAPI *FN_DirectSoundCreate)(const GUID * lpGUID, LPDIRECTSOUND * ppDS, IUnknown FAR *pUnkOuter );
   FN_DirectSoundCreate fnDirectSoundCreate;
   HMODULE hDll;
   HWND hWnd;
   LPIA3D pIA3d;
   bool usingAreal;
   
   DWORD frequency;
   WORD  bitsSample;
   bool  stereo;
   float volume;
   int   maxBuffers;
   bool  muted;

   int threadProcess();     

   void cleanup();
   void updateStreaming();
   void update3D();
   bool restore3DInterface();
   bool restorePrimaryBuffer();
   bool getCaps();
   bool threadClose();

   SoundBuffer* findBuffer(DWORD id);
   SoundBuffer* findBuffer(SFX_HANDLE handle);
   SoundBuffer* _alloc(StringTableEntry, float priority, Profile *prof, const TMat3F &tmat);

   SoundBuffer *selectedBuffer;

   //--------------------------------------
   typedef VectorPtr<Profile*> ProfileVector;
   ProfileVector profile;

   typedef Vector<Pair*> PairVector;
   PairVector pair;

   typedef VectorPtr<SoundBuffer*> BufferVector;
   BufferVector buffer;          
   CRITICAL_SECTION csManager;

   void  lock();
   void  unlock();
   void  prioritize();

//--------------------------------------
public:
   static float sm_2dVolumeAtten;
   static float sm_3dVolumeAtten;

   enum CAPS_ENUM
   {
      NATIVE            = (1<<0),
      CERTIFIED         = (1<<1),
      USE_HARDWARE_3D   = (1<<2),
      HAS_HARDWARE_3D   = (1<<3),
      HAS_AUREAL_3D     = (1<<4),
      HAS_LIVE_3D       = (1<<5),
   };

   //--------------------------------------
   Manager();   
   ~Manager();
   
   // use extreme care accessing DSound from under SFX.
   IDirectSound* getDSound() { return (pDirectSound); }

   //--------------------------------------
   void addProfile(Profile *p);
   void addPair(Pair *p);
   void removeProfile(DWORD id);
   void removePair(DWORD id);
   Profile* findProfile(DWORD id);
   Pair*    findPair(DWORD id);
   Pair*    findPair(const char *name);

   int  getMaxBuffers();
   void setMaxBuffers(int n);
   void mute(bool tf);
   bool query(int query);


   //-------------------------------------- Buffer Manipulation
   SFX_HANDLE selectHandle(SFX_HANDLE handle);
   SFX_HANDLE playAt(DWORD id, const TMat3F &tmat, const Vector3F &vel);
   SFX_HANDLE playAt(StringTableEntry name, float priority, Profile *prof, const TMat3F &tmat, const Vector3F &vel);
   SFX_HANDLE playRelative(StringTableEntry name, float priority, Profile *prof, Point3F &vec);

   SFX_HANDLE play(DWORD id);
   SFX_HANDLE play(const char *name);
   SFX_HANDLE play(StringTableEntry name, float priority, Profile *prof);
   SFX_HANDLE play();
   void stop();
   void release();
   void setTransform(const TMat3F &tmat, const Vector3F &vel);
   void setVolume(float v);
   void setPan(float p);
   bool isLooping (void);

   void  setFlag(DWORD f);
   void  clearFlag(DWORD f);
   DWORD getFlags();
   
   bool isDone();

   //-------------------------------------- System Manipulation
   void stopAll();
   void setListenerTransform(const TMat3F &tmat, const Vector3F &vel);
   void setMasterVolume(float v);

   //-------------------------------------- System Setup
   bool setFormat(DWORD frequency, WORD bitdepth, bool stereo);
   bool getFormat(DWORD &frequency, WORD &bitdepth, bool &stereo);

   bool open();
   bool close();

   bool isCertified();
   bool isNative();
   void getMetrics( Metrics &m);

   //--------------------------------------
   bool processEvent(const SimEvent *event);
   bool onAdd();
   void onRemove();

   static Manager* find(SimManager *manager);
   static SFX_HANDLE Play(SimManager *manager, DWORD id);
   static SFX_HANDLE Play(SimManager *manager, char *wavFileName);
   static SFX_HANDLE PlayAt(SimManager *manager, DWORD id, const TMat3F &tmat, const Vector3F &vel);
   static void Stop(SimManager *manager, SFX_HANDLE handle );
};   


//------------------------------------------------------------------------------
inline int Manager::getMaxBuffers()
{
   return (maxBuffers);   
}   

inline bool Manager::isCertified()
{
   return ( caps & CERTIFIED );
}

inline bool Manager::isNative()
{
   return ( caps & NATIVE );
}

inline Manager* Manager::find(SimManager *manager)
{
   return static_cast<Manager*>(manager->findObject(SimSoundFXObjectId));
}

inline SFX_HANDLE Manager::Play(SimManager *manager, DWORD id)
{
   Manager *sfx = static_cast<Manager*>(manager->findObject(SimSoundFXObjectId));
   if (sfx)
      return sfx->play(id);   
   else
      return (SFX_INVALID_HANDLE);
} 
  
inline SFX_HANDLE Manager::Play(SimManager *manager, char *wavFileName)
{
   Manager *sfx = static_cast<Manager*>(manager->findObject(SimSoundFXObjectId));
   if (sfx)
      return sfx->play(wavFileName);   
   else
      return (SFX_INVALID_HANDLE);
}   

inline SFX_HANDLE Manager::PlayAt(SimManager *manager, DWORD id, const TMat3F &tmat, const Vector3F &vel)
{
   Manager *sfx = static_cast<Manager*>(manager->findObject(SimSoundFXObjectId));
   if (sfx)
      return sfx->playAt(id, tmat, vel);
   else
      return (SFX_INVALID_HANDLE);
}   

inline void Manager::Stop(SimManager *manager, SFX_HANDLE handle )
{
   Manager *sfx = static_cast<Manager*>(manager->findObject(SimSoundFXObjectId));
   if (sfx)
   {
      sfx->selectHandle(handle);
      sfx->stop();
   }
}   



//--------------------------------------------------------------------------- 

struct SoundEvent: public SimEvent
{
   enum MessageType
   {
      Play=0,     // does not use pt
      Stop,       // does not use pt
      PlayAt,     // must supply a pt
   }msg;
   char name[256];
   DWORD profileId;
   Point3F pt;
   
   DECLARE_PERSISTENT( Sfx::SoundEvent );
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);

   SoundEvent() { type = SimSoundFXEventType; } 
   SoundEvent(MessageType _msg, const char *_name, DWORD _profile) 
   { 
      type = SimSoundFXEventType;       
      msg  = _msg;
      profileId = _profile;
      strcpy(name, _name);
      strlwr(name);           
   }
};

}; // namespace Sfx



#endif //_SIMSFX_H_

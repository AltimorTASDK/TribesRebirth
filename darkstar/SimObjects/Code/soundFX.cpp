//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <dsound.h>
#include <sim.h>
#include <soundFX.h>
#include <simSoundSequence.h>
#include <darkstar.strings.h>
#include <simResource.h>
#include <console.h>
#include <m_qsort.h>
#include <stringTable.h>

//// DirectSound 279afa83-4981-11ce-a521-0020af0be560
//const GUID IID_IDirectSound            = {0x279AFA83,0x4981,0x11CE, {0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60}};
//// DirectSoundBuffer 279afa85-4981-11ce-a521-0020af0be560
//const GUID IID_IDirectSoundBuffer      = {0x279AFA85,0x4981,0x11CE, {0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60}};
////DirectSound3DListener 279afa84-4981-11ce-a521-0020af0be560
//const GUID IID_IDirectSound3DListener  = {0x279AFA84,0x4981,0x11CE, {0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60}};
////DirectSound3DBuffer 279afa86-4981-11ce-a521-0020af0be560
//const GUID IID_IDirectSound3DBuffer    = {0x279AFA86,0x4981,0x11CE, {0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60}};

// A3d Interface ID! {D8F1EEE1-F634-11cf-8700-00A0245D918B}
const GUID GUID_IA3d = { 0xd8f1eee1, 0xf634, 0x11cf, {0x87, 0x0, 0x0, 0xa0, 0x24, 0x5d, 0x91, 0x8b}};

namespace Sfx
{

float Manager::sm_2dVolumeAtten = 1.0;
float Manager::sm_3dVolumeAtten = 1.0;

float convertFromLinear(float vol)
{
   if (vol < 0.01f)
      vol = 0.01f;

   float logTwo = float(log(vol) / log(2.0));

   return 0.069 * -logTwo;
}

extern const char* dsToStr(HRESULT h);


IDirectSound            *Base::pDirectSound = NULL;
IDirectSoundBuffer      *Base::pPrimary  = NULL;
IDirectSound3DListener  *Base::pListener = NULL;
DWORD Base::soundFlags = 0;
DWORD Base::caps  = 0;
DWORD Base::hwMemory = 0;
DWORD Base::nHardwareMixed = 0;
DWORD Base::nSoftwareMixed = 0;
DWORD Base::nStreaming = 0;
DWORD Base::nStarved   = 0;
DWORD Base::nBuffers   = 0; 
DWORD Base::msTimerPeriod = SFX_TIMER_PERIOD; 
TMat3F   Base::listenerTransform;
Vector3F Base::listenerVelocity;
Vector3F Base::listenerVector;

SimManager *Base::manager = NULL;

//------------------------------------------------------------------------------
int Manager::threadProcess()
{
   restorePrimaryBuffer();
   while (1)
   {
      //default behavior: sleep until it's time to stop the thread
      switch (WaitForSingleObject( getStopEvent(), msTimerPeriod ))
      {
         case WAIT_OBJECT_0:  
         default:
            return (1);
         case WAIT_TIMEOUT:   
            cleanup();
            updateStreaming();
            break;  
      }
   }
}

inline void Manager::lock()    
{ 
   EnterCriticalSection( &csManager ); 
}

inline void Manager::unlock()  
{ 
   LeaveCriticalSection( &csManager );  
}


//------------------------------------------------------------------------------
Manager::Manager()  
{
   InitializeCriticalSection( &csManager );

   id = SimSoundFXObjectId;

   fnDirectSoundCreate = NULL;   
   volume         = 0.0f;
   selectedBuffer = NULL;
   pDirectSound   = NULL;
   pPrimary  = NULL;
   pListener = NULL;
   hDll      = NULL;
   hWnd      = NULL;
   frequency = 11025;
   bitsSample= 8;
   stereo    = true;
   pIA3d     = NULL;
   usingAreal= false;
   muted     = false;
   caps      = 0;

   maxBuffers = 16;

   listenerTransform.identity();
   listenerVelocity.set();
}   


//------------------------------------------------------------------------------
Manager::~Manager()
{  

   for (ProfileVector::iterator i = profile.begin(); i != profile.end(); i ++)
      delete (*i);

   for (BufferVector::iterator j = buffer.begin(); j != buffer.end(); j ++)
      delete (*j);

   for (PairVector::iterator k = pair.begin(); k != pair.end(); k ++)
      delete (*k);

   DeleteCriticalSection( &csManager );
}


//------------------------------------------------------------------------------
bool Manager::getCaps()
{
   IDirectSoundBuffer *pPrimaryTemp = pPrimary;
   caps  = Console->getBoolVariable("$pref::useDSound3D", false) ? USE_HARDWARE_3D : 0;
   if (usingAreal)
      caps |= USE_HARDWARE_3D;
   hwMemory = 0;

   // test for Aureal
   HMODULE hA3Dll = LoadLibrary("a3d.dll");
   if (hA3Dll)
   {
      caps |= HAS_AUREAL_3D;
      FreeLibrary(hA3Dll);
   }

   if ( pDirectSound )   
   {
      DSCAPS dsCaps;
      memset(&dsCaps, 0, sizeof(DSCAPS)); // Zero it out.
      dsCaps.dwSize = sizeof(DSCAPS);
      if ( pDirectSound->GetCaps( &dsCaps ) == DS_OK )
      {
         caps |= (dsCaps.dwFlags & DSCAPS_EMULDRIVER ) ? 0 : NATIVE;
         caps |= (dsCaps.dwFlags & DSCAPS_CERTIFIED  ) ? CERTIFIED : 0;
         //caps |= (dsCaps.dwFreeHw3DAllBuffers ) ? (HAS_HARD_3D|HAS_HW_3D|HAS_DS_3D) : 0;
         hwMemory = dsCaps.dwTotalHwMemBytes;

         // gets some additional information from the 
         // primary surface (we may need to make a temporary one)
         if ( !pPrimaryTemp )
         {
            DSBUFFERDESC dsbd;
            memset(&dsbd, 0, sizeof(DSBUFFERDESC)); // Zero it out.
            dsbd.dwSize  = sizeof(DSBUFFERDESC);
            dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
            pDirectSound->CreateSoundBuffer( &dsbd, &pPrimaryTemp, NULL );
         }
         if ( pPrimaryTemp )
         {
            IDirectSound3DListener  *pListenerTemp;
            if (pPrimaryTemp->QueryInterface( IID_IDirectSound3DListener, (void**)&pListenerTemp ) == DS_OK)
            {
               caps |= (HAS_HARDWARE_3D);
               pListenerTemp->Release();
            }
         }
         if ( !pPrimary && pPrimaryTemp )
         {
            pPrimaryTemp->Stop();
            pPrimaryTemp->Release();
         }
         return ( true );
      }
   }
   return ( false );
}


//------------------------------------------------------------------------------
bool Manager::query(int query)
{
   if (!caps)
      if (!getCaps())
         return (false);

   switch (query)
   {
      case SFX_QUERY_DSOUND:
         return (true);          // if it got this far it's true
      case SFX_QUERY_DSOUND3D:
         return (caps & HAS_HARDWARE_3D);
      case SFX_QUERY_AUREAL3D:
         return (caps & HAS_AUREAL_3D);
      case SFX_QUERY_LIVE3D:
         return (caps & HAS_LIVE_3D);
      default:
         return (false);
   }   
}   



//------------------------------------------------------------------------------
bool Manager::restore3DInterface()
{
   if ( pPrimary && !pListener)
   {
      if (pPrimary->QueryInterface( IID_IDirectSound3DListener, (void**)&pListener ) == DS_OK)
         caps |= HAS_HARDWARE_3D;
      else
         caps &= ~(HAS_HARDWARE_3D);
      return ( true );
   }
   return ( false );
}


//------------------------------------------------------------------------------
bool Manager::restorePrimaryBuffer()
{
   bool restored = false;
   if ( !pPrimary )
   {
      // build a brand new primary surface
      DSBUFFERDESC dsbd;
      memset(&dsbd, 0, sizeof(DSBUFFERDESC)); // Zero it out.
      dsbd.dwSize   = sizeof(DSBUFFERDESC);
      dsbd.dwFlags  = DSBCAPS_PRIMARYBUFFER;
      dsbd.dwFlags |= (caps & HAS_HARDWARE_3D) ? DSBCAPS_CTRL3D : 0;

      if (pDirectSound->CreateSoundBuffer( &dsbd, &pPrimary, NULL ) == DS_OK)
         restored = true;
      setFormat(frequency, bitsSample, stereo);
   }
   else
   {
      DWORD status;
      if ( pPrimary->GetStatus( &status ) == DS_OK  )
      {
         if ( status & DSBSTATUS_BUFFERLOST )
         {
            // attempt to restore the old primary surface
            if (pPrimary->Restore() == DS_OK )
               restored = true;
         }
         else
            return ( true );  // is still valid, no need to restore
      }
   }

   if ( restored )
   {
      restore3DInterface();
      pPrimary->Play(0, 0, DSBPLAY_LOOPING);
      pPrimary->SetVolume( (int)(volume*-10000.0f) );
      return ( true );
   }

   return ( false );
}


//------------------------------------------------------------------------------
bool Manager::setFormat(DWORD _frequency, WORD _bitsSample, bool _stereo)
{
   if ( pPrimary )
   {
      WAVEFORMATEX format;
      format.wFormatTag = WAVE_FORMAT_PCM;
      format.cbSize     = 0;

      for ( bool s = _stereo; s; s = false)
         for ( DWORD f = _frequency; f >= 11025; f /= 2)
            for ( WORD d = _bitsSample; d >= 8; d /= 2)
            {

               format.nChannels        = s ?  2 : 1;
               format.wBitsPerSample   = d;
               format.nSamplesPerSec   = f;
               format.nBlockAlign      = ((format.nChannels == 2 ) + 1) * ((format.wBitsPerSample == 16) + 1);
               format.nAvgBytesPerSec  = format.nSamplesPerSec * format.nBlockAlign;

               HRESULT result = pPrimary->SetFormat( &format );
               if ( result == DS_OK ) 
               {
                  frequency  = _frequency;
                  bitsSample = _bitsSample;
                  stereo     = _stereo;
                  return (true);
               }
            }
      return ( false );
   }
   // else
   frequency  = _frequency;
   bitsSample = _bitsSample;
   stereo     = _stereo;
   return ( true );
}



//------------------------------------------------------------------------------
bool Manager::getFormat(DWORD &_frequency, WORD &_bitsSample, bool &_stereo)
{
   if ( pPrimary )
   {
      WAVEFORMATEX format;
      DWORD sz;
      if ( pPrimary->GetFormat( &format, sizeof(WAVEFORMATEX), &sz) == DS_OK )
      {
         _frequency = format.nSamplesPerSec;
         _bitsSample= format.wBitsPerSample;
         _stereo    = (format.nChannels == 2);
         return ( true );
      }
   }
   _frequency = frequency;
   _bitsSample= bitsSample;
   _stereo    = stereo;
   return ( false );
}


//------------------------------------------------------------------------------
void Manager::setMaxBuffers(int n)
{
   // cap at 32 channels
   maxBuffers = (n > 32) ? 32 : n;
}   


//------------------------------------------------------------------------------
bool Manager::open()
{
   DWORD proc, curproc;

   if (!Base::manager) return (false);
   close();

   //--------------------------------------
   // Get window handle for application owning this thread
   hWnd = GetTopWindow(0);
   curproc = GetCurrentProcessId();
   while (hWnd)
   {
      GetWindowThreadProcessId(hWnd, &proc);
      if (proc == curproc)
         break;

      hWnd = GetWindow(hWnd, GW_HWNDNEXT);
   }

   // If no window found, use active window (no guarantee it's the right window though!)
   if (hWnd == NULL)
      hWnd = GetActiveWindow();

   //--------------------------------------
   hDll = LoadLibrary("dSound.dll");
   if ( hDll )
   {  //-------------------------------------- TRY Areal 3D if requested
      DWORD result;
      pDirectSound = NULL;
      if (Console->getBoolVariable("$pref::useAureal3D", false))
      {
         A3dCreate(NULL, (void**)&pDirectSound, NULL);
         usingAreal = true;
         if (SUCCEEDED(pDirectSound->QueryInterface(GUID_IA3d, (void **)&pIA3d)))
         {
            pIA3d->SetResourceManagerMode(A3D_RESOURCE_MODE_DYNAMIC);
         }
      }
      else
      {  //-------------------------------------- else TRY DirectSound
         fnDirectSoundCreate = (FN_DirectSoundCreate)GetProcAddress( hDll, "DirectSoundCreate");
         if ( fnDirectSoundCreate ) 
            result = fnDirectSoundCreate( NULL, &pDirectSound, NULL );
      }

      if (pDirectSound)
      {
         //result = pDirectSound->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE);
         //if (!SUCCEEDED(result))
         //{
         //   Console->printf("Sound System: Exclusive co-op level failed: %s", dsToStr(result));            
         
         // Changed to work with Roger Wilco...
         result = pDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
         if (!SUCCEEDED(result))
         {
            Console->printf("Sound System: Priority co-op level failed: %s", dsToStr(result));            
            result = pDirectSound->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
            if (!SUCCEEDED(result))
               Console->printf("Sound System: Normal co-op level failed: %s", dsToStr(result));            
         }
         //}

         //--------------------------------------
         if (SUCCEEDED(result))
         {
            getCaps();
            setPriority( THREAD_PRIORITY_HIGHEST );
            startThread();
            return ( true );
         }
      }
      else
         Console->printf("Sound System: error creating direct sound object: %s", dsToStr(result));            
   }
   close();
   return ( false );
}


//------------------------------------------------------------------------------
bool Manager::close()
{
   stopAll();
   cleanup();
   stopThread();
   usingAreal = false;
   return ( threadClose() );   
}

bool Manager::threadClose()
{
   if (pIA3d) 
   {
      pIA3d->Release();
      pIA3d = NULL;
   }
   if ( pPrimary ) pPrimary->Stop();   

   if ( pListener ) 
   {
      pListener->Release();
      pListener = NULL;
   }
   if ( pPrimary ) 
   {
      pPrimary->Release();   
      pPrimary = NULL;
   }
   if ( pDirectSound ) 
   {
      pDirectSound->Release();
      pDirectSound = NULL;
   }
   if ( hDll ) 
   {
      fnDirectSoundCreate = NULL;
      FreeLibrary( hDll );
      hDll = NULL;
   }
   hWnd = NULL;
   selectedBuffer = NULL;
   return ( true );   
}



//------------------------------------------------------------------------------
bool Manager::processEvent(const SimEvent* in_pEvent)
{
   switch (in_pEvent->type)
   {
      case SimFrameEndNotifyEventType:
         prioritize();
         update3D();
         break;

      case SimSoundFXEventType: {
            SoundEvent *event = const_cast<SoundEvent*>(static_cast<const SoundEvent*>(in_pEvent));
            switch ( event->msg )
            {
               case SoundEvent::Play: {
                  const char *name = stringTable.lookup(event->name);
                  Profile *profile = findProfile(event->profileId);
                  if (profile)
                     play(name, 1.0f, profile);
               }break;
               case SoundEvent::PlayAt: {
                  const char *name = stringTable.lookup(event->name);
                  Profile *profile = findProfile(event->profileId);
                  if (profile)
                  {
                     TMat3F tmat(true);
                     tmat.p = event->pt;
                     playAt(name, 1.0f, profile, tmat, Vector3F(0,0,0));
                  }
               }break;
            }
         }
         return true;
   }
   return ( false );   
}


//------------------------------------------------------------------------------
bool Manager::onAdd()
{
   if (!Parent::onAdd())
      return false;
   Base::manager = SimObject::manager;
   SimSet *set = static_cast<SimSet*>
      (SimObject::manager->findObject(SimFrameEndNotifySetId));
   if (!set)
   {
      set = new SimSet(false);
      SimObject::manager->addObject(set, SimFrameEndNotifySetId);
   }
   set->addObject(this);
   return true;
}


//------------------------------------------------------------------------------

void Manager::onRemove()
{
   close();
   Base::manager = NULL;
   Parent::onRemove();
}


//------------------------------------------------------------------------------
void Manager::getMetrics( Metrics &m )
{
   m.nHardwareMixed = nHardwareMixed;
   m.nSoftwareMixed = nSoftwareMixed;
   m.nStreaming     = nStreaming;
   m.nStarved       = nStarved;
   m.nBuffers       = nBuffers;
   m.msTimerPeriod  = msTimerPeriod;

   if (pPrimary)
   {
      char *p = m.channels;
      lock();

      p += sprintf(p, "%d: ", buffer.size());

      for (int i=0; i < buffer.size(); i++)
      {
         if(DWORD(p) - DWORD(m.channels) > 230)
            break;

         DWORD flags = buffer[i]->getFlags();
         if (flags & SFX_IS_3D)        *p++ = '3';
         if (flags & SFX_IS_LOOPING)   *p++ = 'l';
         if (flags & SFX_PLAYING)      *p++ = 'P';
         if (flags & SFX_STOPPED)      *p++ = 'S';
         if (flags & SFX_SUSPENDED)    *p++ = 's';
         if (flags & SFX_PURGE)        *p++ = 'X';
         *p++ = ' ';
         // handy when debugging
         //p+=sprintf(p, "%0.2f ",buffer[i]->getPriority());
      }
      unlock();
      *p = 0;
   }
   else
      m.channels[0] = 0;
}


//------------------------------------------------------------------------------
void Manager::cleanup()
{
   lock();
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); )
   {
      if ( (*i)->isPurgeable() && (*i) != selectedBuffer)
      {
         delete (*i);
         buffer.erase( i );
      }
      else
         i++;
   }
   unlock();
}   


//------------------------------------------------------------------------------
void Manager::updateStreaming()
{
   if (!pPrimary) return;
   lock();
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
      (*i)->updateStreaming();
   unlock();
}


//------------------------------------------------------------------------------
void Manager::update3D()
{
   if (!pPrimary) return;
   lock();

   listenerTransform.getRow(0, &listenerVector); //xaxis out my left ear

   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
      (*i)->commit();
   unlock();

   if (pListener) pListener->CommitDeferredSettings();
   soundFlags &= ~SFX_3D_LISTENER_CHANGE;
}


//------------------------------------------------------------------------------
void Manager::addProfile(Profile *p)
{
   removeProfile(p->id);
   profile.push_back(p);
}  

 
//------------------------------------------------------------------------------
void Manager::addPair(Pair *p)
{
   removePair(p->id);
   pair.push_back(p);
}   


//------------------------------------------------------------------------------
void Manager::removeProfile(DWORD id)
{
   // remove the profile
   ProfileVector::iterator i= profile.begin();
   for (; i != profile.end(); i++)
      if ((*i)->id == id)
      {
         delete *i;
         profile.erase(i);
         return;
      }
}  

 
//------------------------------------------------------------------------------
void Manager::removePair(DWORD id)
{
   PairVector::iterator i= pair.begin();
   for (; i != pair.end(); i++)
      if ((*i)->id == id)
      {
         delete *i;
         pair.erase(i);
         return;
      }
}   


//------------------------------------------------------------------------------
Profile* Manager::findProfile(DWORD id)
{
   ProfileVector::iterator i= profile.begin();
   for (; i != profile.end(); i++)
      if ((*i)->id == id)
         return (*i);
   return (NULL);
}   

//------------------------------------------------------------------------------
Pair* Manager::findPair(DWORD id)
{
   PairVector::iterator i= pair.begin();
   for (; i != pair.end(); i++)
      if ((*i)->id == id)
         return (*i);
   return (NULL);
}   


//------------------------------------------------------------------------------
Pair* Manager::findPair(const char *name)
{
   PairVector::iterator i= pair.begin();
   for (; i != pair.end(); i++)
      if (! stricmp((*i)->name, name))
         return (*i);
   return (NULL);
}   


//------------------------------------------------------------------------------
SoundBuffer* Manager::findBuffer(SFX_HANDLE handle)
{
   SoundBuffer *result = NULL;

   lock();
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
      if ( (*i)->getHandle() == handle )
      {
         result = (*i);
         break;
      }
   unlock();
   return ( result );
}


//------------------------------------------------------------------------------
SFX_HANDLE Manager::selectHandle(SFX_HANDLE handle)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (pPrimary) 
   {
      lock();
      selectedBuffer = findBuffer( handle );
      if ( selectedBuffer ) 
         result = handle;
      unlock();
   }
   return ( result );
}


//------------------------------------------------------------------------------
SoundBuffer* Manager::_alloc(const char *name, float priority, Profile *prof, const TMat3F &tmat)
{
   // if no primary buffer, we can't play any sounds
   if (!pPrimary) return (NULL);         

   if(!prof) // should probably play as default 2D sound
      return (SoundBuffer*)SFX_INVALID_HANDLE;

   // if it is 3D but not looping, can we possibly hear it?
   if ((prof->flags & SFX_IS_3D) && !(prof->flags & SFX_IS_LOOPING))
   {  
      float dist = m_distf(tmat.p, listenerTransform.p);
      if (dist > prof->maxDistance)
         return (NULL);
   }

   lock();

   // look for a buffer that can be restarted.
   // or find a buffer to duplicate from
   SoundBuffer *dup = NULL;
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
      if ( (*i)->getName() == name )
      {
         if ((*i)->isPurgeable())
         {  // found something we can restart
            SoundBuffer *s = *i;
            s->generateNewHandle();
            s->setTransform(tmat, Vector3F(0,0,0));
            s->commit();
            unlock();
            return (s);   
         }
         else
            dup = *i;
      }

   // alloc a new sound buffer
   SoundBuffer *s = new SoundBuffer;
   if (s && s->open( name, priority, prof, dup ))
   {
      buffer.push_back( s );
      s->setTransform(tmat, Vector3F(0,0,0));
      s->commit();
      unlock();
      return (s);   
   }
   delete s;

   unlock();
   return (NULL);
}   


//------------------------------------------------------------------------------
SFX_HANDLE Manager::playAt(DWORD id, const TMat3F &tmat, const Vector3F &vel)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   Pair *pair = findPair(id);
   if (!pair)
      return (result);
   lock();
   if ( (selectedBuffer = _alloc( pair->name, pair->priority, findProfile(pair->profileId), tmat )) != NULL )
   {
      selectedBuffer->setTransform(tmat, vel);
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}


//------------------------------------------------------------------------------
SFX_HANDLE Manager::playAt(StringTableEntry name, float priority, Profile *prof, const TMat3F &tmat, const Vector3F &vel)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   lock();
   if ( (selectedBuffer = _alloc( name, priority, prof, tmat )) != NULL )
   {
      selectedBuffer->setTransform(tmat, vel);
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}

//------------------------------------------------------------------------------
SFX_HANDLE Manager::playRelative(StringTableEntry name, float priority, Profile *prof, Point3F &relPos)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   lock();
   TMat3F tmat;
   tmat.p = listenerTransform.p + relPos;
   
   if ( (selectedBuffer = _alloc( name, priority, prof, tmat )) != NULL )
   {
      selectedBuffer->setTransform(tmat, Point3F(0,0,0));
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}

//------------------------------------------------------------------------------
typedef int (_USERENTRY *fcmp)(const void *, const void *);
static int __cdecl priorityCompare(const SoundBuffer **a, const SoundBuffer **b)
{  // sort larger numbers to the head of the list
   // if the priorities are equal, tie-break by choosing the youngest sound
   float diff = (*b)->getPriority() - (*a)->getPriority();
   if (diff < 0.0f)  return (-1);
   if (diff > 0.0f)  return (+1);
   return ((*b)->getTime() - (*a)->getTime());
}   


//------------------------------------------------------------------------------
void Manager::prioritize()
{
   lock();
   if (buffer.size() == 0)
   {
      unlock();
      return;
   }

   selectedBuffer = NULL;
   m_qsort((void *)&(buffer[0]), buffer.size(), sizeof(SoundBuffer*), (fcmp)priorityCompare);
   
   int playing = 0;
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
   {
      if ((*i)->isPlaying())
      {  // if you can't hear it, suspend it.
         if (IsZero((*i)->getPriority()))
            (*i)->suspend();
         else
         {  // if we are out of buffers suspend anything that is playing
            // non-looping sounds will stop instead of suspend
            if (playing > maxBuffers)
               (*i)->suspend();
            else
               playing++;
         }
      }
      else
      {  
         if (playing <= maxBuffers)
         {  // start anything that is suspended, ie looping sounds
            if ((*i)->isSuspended())
            {
               (*i)->play();
               playing++;
            }
         }
      }
   }
   unlock();   
}   




//------------------------------------------------------------------------------

SFX_HANDLE Manager::play(StringTableEntry name, float priority, Profile *prof)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   lock();
   if ( (selectedBuffer = _alloc( name, priority, prof, TMat3F() )) != NULL )
   {
      selectedBuffer->setVolume(convertFromLinear(sm_2dVolumeAtten));
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}

//------------------------------------------------------------------------------

SFX_HANDLE Manager::play(DWORD id)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   Pair *pair = findPair(id);
   if (!pair)
      return (result);
   lock();
   if ( (selectedBuffer = _alloc( pair->name, pair->priority, findProfile(pair->profileId), TMat3F() )) != NULL )
   {
      selectedBuffer->setVolume(convertFromLinear(sm_2dVolumeAtten));
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}


//------------------------------------------------------------------------------
SFX_HANDLE Manager::play(const char *name)
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   if (muted) return (SFX_INVALID_HANDLE);
   name = stringTable.insert(name);
   Pair *pair = findPair(name);
   Profile *prof = NULL;
   float priority = 1.0;
   if(pair)
   {
      prof = findProfile(pair->profileId);
      priority = pair->priority;
   }

   lock();
   if ( (selectedBuffer = _alloc( name, priority, prof, TMat3F() )) != NULL )
   {
      selectedBuffer->setVolume(convertFromLinear(sm_2dVolumeAtten));
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}

//------------------------------------------------------------------------------
SFX_HANDLE Manager::play()
{
   SFX_HANDLE result = SFX_INVALID_HANDLE;
   lock();
   if (selectedBuffer)
   {
      selectedBuffer->setVolume(convertFromLinear(sm_2dVolumeAtten));
      selectedBuffer->play();
      result = selectedBuffer->getHandle();   
   }
   unlock();
   return ( result );
}


//------------------------------------------------------------------------------
void Manager::stop()
{
   lock();
   if (selectedBuffer) selectedBuffer->stop();
   unlock();
}

//------------------------------------------------------------------------------
bool Manager::isDone()
{
   bool result;
   lock();
   result = selectedBuffer ? (selectedBuffer->isPurgeable()) : true;
   unlock();
   return (result);
}


//------------------------------------------------------------------------------
bool Manager::isLooping()
{
   bool result;
   lock();
   result = selectedBuffer ? (selectedBuffer->isLooping()) : false;
   unlock();
   return (result);
}


//------------------------------------------------------------------------------
void Manager::release()
{
   lock();
   if (selectedBuffer)
   {
      selectedBuffer->stop();
      selectedBuffer = NULL;
   }
   unlock();
}


//------------------------------------------------------------------------------
void Manager::setTransform(const TMat3F &tmat, const Vector3F &vel)
{
   lock();
   if (selectedBuffer) selectedBuffer->setTransform(tmat, vel);
   unlock();
}


//------------------------------------------------------------------------------
void Manager::setVolume(float v)
{
   lock();
   if (selectedBuffer) selectedBuffer->setVolume(v);
   unlock();
}


//------------------------------------------------------------------------------
void Manager::setPan(float p)
{
   lock();
   if (selectedBuffer) selectedBuffer->setPan(p);
   unlock();
}   


//------------------------------------------------------------------------------
void Manager::setFlag(DWORD f)
{
   lock();
   if (selectedBuffer) selectedBuffer->setFlag(f);
   unlock();
}


//------------------------------------------------------------------------------
void Manager::clearFlag(DWORD f)
{
   lock();
   if (selectedBuffer) selectedBuffer->clearFlag(f);
   unlock();
}


//------------------------------------------------------------------------------
DWORD Manager::getFlags()
{
   DWORD flg = 0;
   lock();
   if (selectedBuffer) flg = selectedBuffer->getFlags();
   unlock();
   return ( flg );
}


//------------------------------------------------------------------------------
void Manager::mute(bool tf)
{
   muted = tf;
   if (muted)
      stopAll();
}


//------------------------------------------------------------------------------
void Manager::stopAll()
{
   lock();

   selectedBuffer = NULL;
   BufferVector::iterator i = buffer.begin();
   for (; i != buffer.end(); i++)
      (*i)->stop();

   unlock();
}   


//------------------------------------------------------------------------------
void Manager::setListenerTransform(const TMat3F &tmat, const Vector3F &vel)
{
   lock();
   listenerTransform = tmat;
   listenerVelocity  = vel;
   if ( pListener )
   {
      Vector3F front, top;
      listenerTransform.getRow(2, &front);  // yaxis
      listenerTransform.getRow(1, &top);    // zaxis

      pListener->SetOrientation( front.x, front.y, front.z, top.x, top.y, top.z, DS3D_DEFERRED);
      pListener->SetPosition( tmat.p.x, tmat.p.y, tmat.p.z, DS3D_DEFERRED);
      pListener->SetVelocity( vel.x, vel.y, vel.z, DS3D_DEFERRED);
   }
   //set listener commit flag to notify buffers the listener position has changed
   soundFlags |= SFX_3D_LISTENER_CHANGE;
   unlock();
}


//------------------------------------------------------------------------------
void Manager::setMasterVolume(float v)
{
   volume = v;
   if ( pPrimary ) 
      pPrimary->SetVolume( (int)(volume*-10000.0f) );
}


//------------------------------------------------------------------------------
IMPLEMENT_PERSISTENT_TAG( Sfx::SoundEvent, SimSoundFXEventType );

void SoundEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   SimEvent::pack(manager, ps, bstream);
   bstream->writeInt(msg, 2);
   char *p = strstr(name, ".wav");  // save a couple bytes on pack/unpack
   if (p) *p = 0;
   bstream->writeString(name);
   bstream->writeInt(profileId, 32);
   if (bstream->writeFlag( msg == PlayAt ))
   {
      bstream->writeSignedFloat(pt.x, 24);
      bstream->writeSignedFloat(pt.y, 24);
      bstream->writeSignedFloat(pt.z, 24);
   }
   if (p) *p = '.';
}  
 
void SoundEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;
   SimEvent::unpack(manager, ps, bstream);
   msg = (Sfx::SoundEvent::MessageType)bstream->readInt(2);
   bstream->readString(name);
   strcat(name, ".wav");
   profileId = bstream->readInt(32);
   if (bstream->readFlag())
   {
      pt.x = bstream->readSignedFloat(24);
      pt.y = bstream->readSignedFloat(24);
      pt.z = bstream->readSignedFloat(24);
   }
}   


}; // namespace Sfx

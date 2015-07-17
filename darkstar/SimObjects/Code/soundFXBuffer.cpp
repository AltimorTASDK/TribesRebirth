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
#include <simResource.h>
namespace Sfx
{

//------------------------------------------------------------------------------
// static members
DWORD SoundBuffer::nextHandle = 0;

extern float convertFromLinear(float vol);


///*  Useful debugging
const char* dsToStr(HRESULT h)
{
   switch ( h )
   {
      case DS_OK:                       return "DS_OK";                  
      case DSERR_ALLOCATED:             return "DSERR_ALLOCATED";
      case DSERR_CONTROLUNAVAIL:        return "DSERR_CONTROLUNAVAIL";    
      case DSERR_INVALIDPARAM:          return "DSERR_INVALIDPARAM";      
      case DSERR_INVALIDCALL:           return "DSERR_INVALIDCALL";       
      case DSERR_GENERIC:               return "DSERR_GENERIC";           
      case DSERR_PRIOLEVELNEEDED:       return "DSERR_PRIOLEVELNEEDED";   
      case DSERR_OUTOFMEMORY:           return "DSERR_OUTOFMEMORY";       
      case DSERR_BADFORMAT:             return "DSERR_BADFORMAT";         
      case DSERR_UNSUPPORTED:           return "DSERR_UNSUPPORTED";       
      case DSERR_NODRIVER:              return "DSERR_NODRIVER";          
      case DSERR_ALREADYINITIALIZED:    return "DSERR_ALREADYINITIALIZED";
      case DSERR_NOAGGREGATION:         return "DSERR_NOAGGREGATION";     
      case DSERR_BUFFERLOST:            return "DSERR_BUFFERLOST";        
      case DSERR_OTHERAPPHASPRIO:       return "DSERR_OTHERAPPHASPRIO";   
      case DSERR_UNINITIALIZED:         return "DSERR_UNINITIALIZED";
      default: return ( "DS Unknown");     
   }
}
//*/


//------------------------------------------------------------------------------
SoundBuffer::SoundBuffer()
{
   handle      = getNextHandle();
   pBuffer     = NULL;
   p3DBuffer   = NULL;
   format      = NULL;
   waveData    = NULL;
   name        = NULL;
   wavePosition= 0;
   writeCursor = 0;
   bufferSize  = 0;

   volume      = 0.0f;     // full volume (actual volume after 3D effects)
   pan         = 0.0f;     // centered

   transform.identity();
   csFill = 0;
   nBuffers++;

   time = GetTickCount();
}


//------------------------------------------------------------------------------
SoundBuffer::~SoundBuffer()
{
   close();
   nBuffers--;
}


//------------------------------------------------------------------------------
SFX_HANDLE SoundBuffer::getNextHandle()
{
   nextHandle++;
   if ( !nextHandle ) nextHandle++;    //handle must not equal SFX_INVALID_HANDLE
   return ( (SFX_HANDLE)nextHandle );
}


//------------------------------------------------------------------------------
bool SoundBuffer::open(const char *nm, float in_priority, const Profile *prof, SoundBuffer *copy )
{
   close();

   profile = *prof;
   name    = nm;
   flags   = profile.flags & ~SFX_INTERNAL_MASK;
   flags  |= SFX_SUSPENDED;
   priority= in_priority;

   if ( flags & SFX_IS_HARDWARE_3D )
   {
      if ( !((caps & Manager::HAS_HARDWARE_3D) && (caps & Manager::USE_HARDWARE_3D)) )
      {
         flags &= ~SFX_IS_3D;
         flags |= SFX_IS_SOFTWARE_3D;
      }
   }
   velocity.set();
   transform.identity();

   if ( openWav() )
   {
      // calculate the Direct Sound buffer size
      if ( flags & SFX_IS_STREAMING )
      {
         bufferSize = (format->nAvgBytesPerSec * SFX_BUFFER_LENGTH) / 1000;
         if ( (waveSize * 2) < bufferSize)
         {
            // if the sample is not at least twice as long as the size of 
            // the streaming buffer don't bother streaming.
            flags &= ~SFX_IS_STREAMING;
            bufferSize = waveSize;
         }
      }
      else
         bufferSize = waveSize;
      restoreBuffer( copy );
      return ( true );
   }
   else
   {
      flags &= SFX_PURGE;
      return ( false );
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::close()
{
   closeWav();
   if ( p3DBuffer )
   {
      p3DBuffer->Release();
      p3DBuffer = NULL;
   }

   if ( pBuffer )
   {
      pBuffer->Stop();
      pBuffer->Release();   
      pBuffer = NULL;
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::setTransform(const TMat3F &tmat, const Vector3F &vel)
{
   transform = tmat;
   velocity  = vel;
   flags |= SFX_3D_BUFFER_CHANGE;      
}


//------------------------------------------------------------------------------
BYTE SoundBuffer::getSilence()
{
    BYTE silenceData = 0;

    // Silence data depends on format of Wave file
    if (format)
    {
        if (format->wBitsPerSample == 8)
        {
            // For 8-bit formats (unsigned, 0 to 255)
            // Packed DWORD = 0x80808080;
            silenceData = 0x80;
        }
        else if (format->wBitsPerSample == 16)
        {
            // For 16-bit formats (signed, -32768 to 32767)
            // Packed DWORD = 0x00000000;
            silenceData = 0x00;
        }
    }

    return (silenceData);
}


//------------------------------------------------------------------------------
IDirectSoundBuffer* SoundBuffer::duplicate()
{
   IDirectSoundBuffer *pCopy = NULL;
   if ( pBuffer && pDirectSound)
      pDirectSound->DuplicateSoundBuffer( pBuffer, &pCopy );
   
   return ( pCopy );
}


//------------------------------------------------------------------------------
bool SoundBuffer::restoreBuffer(SoundBuffer *copy)
{
   bool restored = false;
   if ( !pBuffer )   
   {
      // build a brand new secondary surface
      DSBUFFERDESC dsbd;
      memset(&dsbd, 0, sizeof(DSBUFFERDESC)); // Zero it out.
      dsbd.dwSize   = sizeof(DSBUFFERDESC);
      dsbd.dwFlags  = DSBCAPS_GETCURRENTPOSITION2;
      // 12/97 using hardware mixing seems to cause huge pauses, up to ~400ms, on
      // the sound buffer unlock calls as it transfers the sample to the card.
      if (  (flags & SFX_IS_HARDWARE_3D)
         && (caps & Manager::USE_HARDWARE_3D)
         && (caps & Manager::HAS_HARDWARE_3D)
         && !(flags & SFX_IS_STREAMING) )
      {
         dsbd.dwFlags |= DSBCAPS_CTRL3D;
         dsbd.dwFlags |= DSBCAPS_STATIC;
         dsbd.dwFlags |= (DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME);
      }
      else
      {
         dsbd.dwFlags |= DSBCAPS_LOCSOFTWARE;
         dsbd.dwFlags |= (DSBCAPS_CTRLPAN|DSBCAPS_CTRLVOLUME);
      }
      if (flags & SFX_IS_PITCHING)
         dsbd.dwFlags |= DSBCAPS_CTRLFREQUENCY;

      dsbd.dwBufferBytes= bufferSize;
      dsbd.lpwfxFormat  = (WAVEFORMATEX*)&format->wFormatTag;   // same as WAVEFORMATEX struct

      if ( copy )
         pBuffer = copy->duplicate();

      if ( !pBuffer )
      {
         pDirectSound->CreateSoundBuffer( &dsbd, &pBuffer, NULL );
         flags |= SFX_FILL_BUFFER;
      }


      if ( pBuffer )
      {
         DSBCAPS dsCaps;
         memset(&dsCaps, 0, sizeof(DSBCAPS)); // Zero it out.
         dsCaps.dwSize = sizeof(DSBCAPS);
         if ( pBuffer->GetCaps( &dsCaps ) == DS_OK )
         {
            if ( dsCaps.dwFlags & DSBCAPS_LOCHARDWARE )
               flags |= SFX_HARDWARE_BUFFER;
            else
               flags &= ~SFX_HARDWARE_BUFFER;
         }
         if (flags & SFX_IS_HARDWARE_3D)  
            pBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void**)&p3DBuffer );
      }
   }
   else
   {
      DWORD status;
      if ( pBuffer->GetStatus( &status ) == DS_OK  )
      {
         if ( status & DSBSTATUS_BUFFERLOST )
         {
            if (flags & (SFX_IS_LOOPING | SFX_IS_STREAMING))
            {
               // attempt to restore the old primary surface
               if (pBuffer->Restore() == DS_OK )
               {
                  flags |= SFX_FILL_BUFFER;
                  restored = true;
               }
            }
            else
               adjStopFlags();
         }
         else
            return ( true );  // is still valid, no need to restore
      }
   }

   if ( restored )
   {
      //restore 3D interface
      pBuffer->QueryInterface( IID_IDirectSound3DBuffer, (void**)&p3DBuffer );
      flags |= SFX_3D_BUFFER_CHANGE;
      commit();
      play();
      return ( true );
   }
   else
      return ( pBuffer );
}


//------------------------------------------------------------------------------
DWORD SoundBuffer::getFillSize()
{
   DWORD dsPlayCursor, dsWriteCursor;
   DWORD size;
   if ( !pBuffer ) return 0;
   if (pBuffer->GetCurrentPosition( &dsPlayCursor, &dsWriteCursor) == DS_OK )
   {
      if ( flags & SFX_FILL_BUFFER)
      {
         if ( writeCursor == dsPlayCursor )   
            size = bufferSize;
         else
         {
            if ( writeCursor <= dsPlayCursor )   
               size = dsPlayCursor - writeCursor;
            else
               size = bufferSize - writeCursor + dsPlayCursor;
         }
         flags &= ~SFX_FILL_BUFFER;
      }
      else
      {
         if ( (flags & SFX_PLAYING) && (flags & SFX_IS_STREAMING) )
         {
            if ( writeCursor <= dsPlayCursor )   
               size = dsPlayCursor - writeCursor;
            else
               size = bufferSize - writeCursor + dsPlayCursor;
         }
         else
         {
            size = 0;      
         }
      }
   }
   return ( size );
}


//------------------------------------------------------------------------------
void SoundBuffer::handleFill()
{
   if ( InterlockedExchange( &csFill, TRUE) ) return;

   DWORD fillSize = getFillSize();
   if ( fillSize )
   {
      void  *p1, *p2, *p3;
      DWORD l1,l2,l3;
      if ( (fillSize+wavePosition) > waveSize )
      {
         //---------------------------------------------------------------------
         // need to loop the sample
         if ( flags & SFX_IS_LOOPING )
         {
            DWORD remaining = waveSize - wavePosition;

            if ( pBuffer->Lock(writeCursor, fillSize, &p1, &l1, &p2, &l2, 0) == DS_OK)
            {
               if ( remaining <= l1)
               {
                  // transition happened in first segment
                  p3 = (char*)p1+remaining;
                  l3 = l1-remaining;
                  memcpy(p1, waveData+wavePosition, remaining); 
                  if (l3) memcpy(p3, waveData, l3); 
                  wavePosition = (wavePosition +l1) % waveSize;
                  
                  if ( l2 )  memcpy(p2, waveData+wavePosition, l2);

                  pBuffer->Unlock(p1, l1, p2, l2);

                  wavePosition += l2;
                  writeCursor   = (writeCursor+fillSize) % bufferSize;
               }
               else
               {
                  // transition happened in second segment
                  remaining -= l1;
                  p3 = (char*)p2+remaining;
                  l3 = l2-remaining;
                  memcpy(p1, waveData+wavePosition, l1); 
                  
                  wavePosition += l1;
                  if (remaining) memcpy(p2, waveData+wavePosition, remaining); 
                  if (l3)        memcpy(p3, waveData, l3); 

                  pBuffer->Unlock(p1, l1, p2, l2);

                  wavePosition  = l3;
                  writeCursor   = (writeCursor+fillSize) % bufferSize;
               }
            }
         }
         //---------------------------------------------------------------------
         // need to finish and fill with silence
         else
         {
            if ( wavePosition <= waveSize )
            {
               DWORD remaining = waveSize - wavePosition;

               if ( pBuffer->Lock(writeCursor, fillSize, &p1, &l1, &p2, &l2, 0) == DS_OK)
               {
                  if ( remaining <= l1)
                  {
                     // transition happened in first segment
                     p3 = (char*)p1+remaining;
                     l3 = l1-remaining;
                     memcpy(p1, waveData+wavePosition, remaining); 
                     if (l3) memset(p3, getSilence(), l3); 
                     if (l2) memset(p2, getSilence(), l2);

                     pBuffer->Unlock(p1, l1, p2, l2);

                     wavePosition += l1+l2;
                     writeCursor   = (writeCursor+fillSize) % bufferSize;
                  }
                  else
                  {
                     // transition happened in second segment
                     remaining -= l1;
                     p3 = (char*)p2+remaining;
                     l3 = l2-remaining;
                     memcpy(p1, waveData+wavePosition, l1); 
                     
                     wavePosition += l1;
                     if (remaining) memcpy(p2, waveData+wavePosition, remaining); 
                     if (l3)        memset(p3, getSilence(), l3); 

                     pBuffer->Unlock(p1, l1, p2, l2);

                     wavePosition += l1+l2;
                     writeCursor   = (writeCursor+fillSize) % bufferSize;
                  }
               }
            }
            else
            {
               if ( wavePosition > (waveSize+bufferSize) )
                  stop();   
               else
               {
                  if ( pBuffer->Lock(writeCursor, fillSize, &p1, &l1, &p2, &l2, 0) == DS_OK)
                  {
                     memset(p1, getSilence(), l1);   
                     if (l2) memset(p2, getSilence(), l2);   

                     pBuffer->Unlock(p1, l1, p2, l2);
                     wavePosition += l1+l2;
                     writeCursor   = (writeCursor+fillSize) % bufferSize;
                  }
               }
            }
         }
      }
      else
      {
         //---------------------------------------------------------------------
         // straight forward fill
         if ( pBuffer->Lock(writeCursor, fillSize, &p1, &l1, &p2, &l2, 0) == DS_OK)
         {
            memcpy(p1, waveData+wavePosition, l1); 
            if ( l2 )  memcpy(p2, waveData+wavePosition+l1, l2);

            pBuffer->Unlock(p1, l1, p2, l2);
               
            wavePosition += l1+l2;
            writeCursor   = (writeCursor+fillSize) % bufferSize;
         }
      }
   }
   InterlockedExchange( &csFill, FALSE );
}


//------------------------------------------------------------------------------
void SoundBuffer::updateStreaming()
{
   DWORD status;
   if ( !pBuffer ) return;
   restoreBuffer();
   handleFill();

   if ( pBuffer->GetStatus( &status ) == DS_OK  )
      if ( !(status & DSBSTATUS_PLAYING) )
         adjStopFlags();
}


//------------------------------------------------------------------------------
void SoundBuffer::play()
{
   if ( pBuffer )
   {
      pBuffer->Stop();
      pBuffer->SetCurrentPosition(0);
      commit();
      updateStreaming();

      // if it's a 3D sound and out of hearing range don't play it
      // if it's a 2D sound that's basically turned off don't play it.
      if (!IsEqual(volume, 1.0f))
         if (pBuffer->Play(0, 0, (flags & (SFX_IS_LOOPING | SFX_IS_STREAMING)) ? DSBPLAY_LOOPING : 0) == DS_OK)
         {
            flags &= ~(SFX_STOPPED|SFX_SUSPENDED|SFX_PURGE);
            flags |= SFX_PLAYING;
            if ( flags & SFX_HARDWARE_BUFFER ) 
               nHardwareMixed++;
            else
               nSoftwareMixed++;
            time = GetTickCount();
         }
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::adjStopFlags()
{
   if ( flags & (SFX_PLAYING|SFX_SUSPENDED) )
   {
      if ( flags & SFX_HARDWARE_BUFFER ) 
         nHardwareMixed--;
      else
         nSoftwareMixed--;
   }
   flags &= ~SFX_PLAYING;
   flags |= SFX_STOPPED;
   if (!(flags & SFX_SUSPENDED))
      flags |= SFX_PURGE;
}


//------------------------------------------------------------------------------
void SoundBuffer::stop()
{
   if ( pBuffer )
   {
      flags &= ~SFX_SUSPENDED;
      pBuffer->Stop();
      adjStopFlags();
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::suspend()
{
   if (flags & SFX_IS_LOOPING)
      flags |= SFX_SUSPENDED;
   else
      flags |= SFX_STOPPED;
   if ( pBuffer )
   {
      pBuffer->Stop();
      adjStopFlags();
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::closeWav()
{
   waveData   = NULL;
   format = NULL;   
}


//------------------------------------------------------------------------------
bool SoundBuffer::openWav()
{
   if ( waveData ) return ( true );       // if already loaded... success
   if ( !manager ) return ( false );   // if no manager... failure
   DWORD *p = (DWORD*)SimResource::get(manager)->lock(name);
   if ( p )      
   {
      DWORD riff  = *p++;
      DWORD length= *p++;
      DWORD type  = *p++;

      if (riff == FOURCC('R','I','F','F') && type == FOURCC('W','A','V','E'))
      {
         DWORD *end = (DWORD*)( ((char*)p)+length-4 );

         while (p < end)
         {
            type   = *p++;
            length = *p++;
            switch (type)
            {
               case FOURCC('f','m','t',' '):
                  if (length < sizeof(WAVEFORMAT))
                     return (false);
                  format    = (WAVEFORMATEX *)p;
                  frequency = format->nSamplesPerSec;
                  break;

               case FOURCC('d','a','t','a'):
                  waveData = (char*)p;
                  waveSize  = length;
                  return (true);
            }
            p = (DWORD*)( (char*)p + ((length+1)&~1) );
         }
      }
   }
   return (false);
}


//------------------------------------------------------------------------------
float SoundBuffer::getPriority() const
{
   if (IsZero(priority))
      return (1.0f-volume);
   else
      return (priority);
}   


//------------------------------------------------------------------------------
float SoundBuffer::estimateVolume()
{
   if (!(flags & SFX_IS_3D)) 
      return (profile.baseVolume);

   volume = 1.0f;
   Vector3F relative( transform.p );
   relative -= listenerTransform.p;               
   float dist = relative.lenf();
   if (IsZero(dist))
      dist = 0.00001f;     //don't want any divide by Zero GP faults

   // adjust VOLUME for distance
   if ( dist >= profile.maxDistance )
      volume = 1.0f;                      // full attenuation
   else if ( dist > profile.minDistance )
   {
      float factor = (dist - profile.minDistance) / (profile.maxDistance - profile.minDistance);
      //factor = (factor >= RealF(0))  ? m_sqrtf(factor) : 0.0f;
      factor = (factor >= RealF(0))  ? factor : 0.0f;
      volume +=  ((1.0-volume) * factor);
   }
   return (volume);
}   


//------------------------------------------------------------------------------
void SoundBuffer::commit()
{
   DWORD status;
   if ( pBuffer && pBuffer->GetStatus( &status ) == DS_OK)   
   {
      if ( status != DSBSTATUS_BUFFERLOST )
      {
         // if buffer changed or listener moved update it
         if (( soundFlags & SFX_3D_LISTENER_CHANGE ) || 
             ( flags & SFX_3D_BUFFER_CHANGE ) )
            commit3D();
         if ( flags & SFX_2D_BUFFER_CHANGE ) commit2D();
      }
   }
   else
      estimateVolume();
}


//------------------------------------------------------------------------------
void SoundBuffer::commit2D()
{
   if ( pBuffer )
   {
      pBuffer->SetPan( (int)(pan*10000.0f) );
      pBuffer->SetVolume( (int)(volume*-10000.0f) );
      pBuffer->SetFrequency( frequency );
      flags &= ~SFX_2D_BUFFER_CHANGE;      
   }
}


//------------------------------------------------------------------------------
void SoundBuffer::commit3D()
{
   if ( pBuffer )
   {
      switch ( flags & SFX_IS_3D )
      {
         case SFX_IS_SOFTWARE_3D:
         //-------------------------------------- Update SFX software 3D
         {
            Vector3F relative( transform.p );
            relative -= listenerTransform.p;               
            float dist = relative.lenf();
            if (IsZero(dist))
               dist = 0.00001f;     //don't want any divide by Zero GP faults

            // calculate PAN
            pan = m_dot(listenerVector, relative) / dist * 0.2f;  

            float x;

            // adjust for cone VOLUME
            if ( !IsZero(profile.coneOutsideVolume) )
            {
               #define DS_FUDGE_FACTOR (12.0f)
               Vector3F cone;
               float inside  = float(profile.coneInsideAngle)/360.0f;
               float outside = float(profile.coneOutsideAngle)/360.0f;
               relative.neg();   
               m_mul( profile.coneVector, (RMat3F)transform, &cone);
               x = 1.0 - (((m_dot(cone, relative) / dist) + 1.0f) / 2.0f);
               if ( x >= outside )
                  volume = profile.baseVolume + convertFromLinear(Manager::sm_3dVolumeAtten) + profile.coneOutsideVolume/DS_FUDGE_FACTOR;
               else if (x <= inside)
                  volume = profile.baseVolume + convertFromLinear(Manager::sm_3dVolumeAtten); 
               else
               {
                  float factor = ((x - inside) / (outside - inside));
                  volume = profile.baseVolume + convertFromLinear(Manager::sm_3dVolumeAtten) + ( (profile.coneOutsideVolume/DS_FUDGE_FACTOR) * factor ); 
               }
            }
            else
               volume = profile.baseVolume + convertFromLinear(Manager::sm_3dVolumeAtten);           // No attenuation

            // adjust VOLUME for distance
            if ( dist >= profile.maxDistance )
               volume = 1.0f;                      // full attenuation
            else if ( dist > profile.minDistance )
            {
               float factor = (dist - profile.minDistance) / (profile.maxDistance - profile.minDistance);
               //factor = (factor >= RealF(0))  ? m_sqrtf(factor) : 0.0f;
               factor = (factor >= RealF(0))  ? factor : 0.0f;
               volume +=  ((1.0-volume) * factor);
            }

            flags |= SFX_2D_BUFFER_CHANGE;      
         } break;

         case SFX_IS_HARDWARE_3D:
         {
            //-------------------------------------- Update DSound 3D
            if ( flags & SFX_2D_BUFFER_CHANGE )
            {
               pBuffer->SetVolume( (int)((profile.baseVolume + convertFromLinear(Manager::sm_3dVolumeAtten))*-10000.0f) );
               pBuffer->SetFrequency( frequency );
               flags &= ~SFX_2D_BUFFER_CHANGE;      
            }

            // adjust VOLUME for distance (used only for clipping)
            Vector3F relative( transform.p );
            relative -= listenerTransform.p;               
            float dist = relative.lenf();
            if (IsZero(dist))
               dist = 0.00001f;     //don't want any divide by Zero GP faults

            if ( dist >= profile.maxDistance )
               volume = 1.0f;                      // full attenuation
            else if ( dist > profile.minDistance )
            {
               float factor = (dist - profile.minDistance) / (profile.maxDistance - profile.minDistance);
               //factor = (factor >= RealF(0))  ? m_sqrtf(factor) : 0.0f;
               factor = (factor >= RealF(0))  ? factor : 0.0f;
               volume +=  ((1.0-volume) * factor);
            }

            DWORD apply = (!(flags & SFX_PLAYING)) ? DS3D_IMMEDIATE : DS3D_DEFERRED;

            //update cone orientation
            if ( profile.coneInsideAngle != 360 )
            {
               Vector3F cone;
               Vector3F orient;
               orient.set(profile.coneVector.x, profile.coneVector.y, profile.coneVector.z );
               m_mul( orient, (RMat3F)transform, &cone);

               p3DBuffer->SetConeOrientation( cone.x, cone.y, cone.z, apply );
               p3DBuffer->SetConeOutsideVolume((int)profile.coneOutsideVolume*-10000.0f, apply );
            }
            p3DBuffer->SetConeAngles(profile.coneInsideAngle, profile.coneOutsideAngle, apply );
            p3DBuffer->SetMinDistance(profile.minDistance, apply );
            p3DBuffer->SetMaxDistance(profile.maxDistance, apply );
            p3DBuffer->SetVelocity( velocity.x, velocity.y, velocity.z, apply );
            p3DBuffer->SetPosition( transform.p.x, transform.p.y, transform.p.z, apply );
            
         } break;
      }
      flags &= ~SFX_3D_BUFFER_CHANGE;      
   }
}


}; // namespace Sfx
//-----------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#ifndef _SIMSOUNDSEQUENCE_H_
#define _SIMSOUNDSEQUENCE_H_

#include <sim.h>
#include <soundFX.h>

//------------------------------------------------------------------------------
struct SimSoundSequenceEvent: public SimEvent
{
   enum { CHANNEL_BITS = 6 }; // that's one more bit than we actually need (since we need -1)

   DWORD  id;
   bool spatial;
   TMat3F pos;
   Int8 channel;
   
   SimSoundSequenceEvent() {type = SimSoundSequenceEventType;}
   
   DECLARE_PERSISTENT( SimSoundSequenceEvent );
   
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
};

//------------------------------------------------------------------------------
class SimSoundSequence : public SimObject
{
	typedef SimObject Parent;

   public:
      // if maxChannel is changed, modify the pack and unpack methods of SimSoundSequenceEvent
      enum { maxChannel = 32, updateFrequency = 2 };

   private:
   	struct channelInfo
      {
         SFX_HANDLE current;
         Vector<DWORD> queueId;
         Vector<bool> queueSpatial;
         Vector<TMat3F> queuePos;   
      };

   	channelInfo channel[maxChannel];
      bool modified;    // a simple flag to set when a sound is queued or played      

      bool onSimSoundSequenceEvent(const SimSoundSequenceEvent *event);
      bool onSimTimeEvent(const SimTimeEvent *);

   public:
		SimSoundSequence();
      
      bool processEvent(const SimEvent *event);
		bool onAdd();
      
      // only from the client
      static void Queue(SimManager *manager, Int8 channel, DWORD id);
      static void QueueAt(SimManager *manager, Int8 channel, DWORD id, const TMat3F &tmat);   

      // only from the server
      static void BroadcastQueue(SimManager *manager, Int8 channel, DWORD id);
      static void BroadcastQueueAt(SimManager *manager, Int8 channel, DWORD id, const TMat3F &tmat);   
};

//------------------------------------------------------------------------------

#endif
//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simSoundSequence.h>
#include <NetPacketStream.h>
#include <NetEventManager.h>

//------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG( SimSoundSequenceEvent, SimSoundSequenceEventType );

void SimSoundSequenceEvent::pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   SimEvent::pack(manager, ps, bstream);
   
   bstream->write((Int32)id);
   bstream->writeInt(channel, CHANNEL_BITS);   // since there are currently 32 channels (and -1)
   
   bstream->writeFlag(spatial);
   if(spatial)
   {
      bstream->write(sizeof(TMat3F), &pos);
   }
}

void SimSoundSequenceEvent::unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;

   SimEvent::unpack(manager, ps, bstream);
   
   bstream->read((Int32 *)&id);
   channel = bstream->readInt(CHANNEL_BITS);   // since there are currently 32 channels (and -1)
   if( channel == (1 << CHANNEL_BITS) - 1 )
      channel = -1;
   
   spatial = bstream->readFlag();
   if(spatial)
   {
      bstream->read(sizeof(TMat3F), &pos);
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

SimSoundSequence::SimSoundSequence()
{
   for(int c = 0; c < maxChannel; c++)
   {
      channel[c].current = SFX_INVALID_HANDLE;
   }
   
   modified = false;
}

//------------------------------------------------------------------------------

bool SimSoundSequence::onSimSoundSequenceEvent(const SimSoundSequenceEvent *event)
{
   Int8 pipe = event->channel;
   
   if(pipe == -1)
   {     
      // doesn't do velocity yet
      if(event->spatial)
      {      
         Sfx::Manager::PlayAt(manager, event->id, event->pos, Point3F(0, 0, 0));      
      }
      else
      {
         Sfx::Manager::Play(manager, event->id);      
      }
      
      return true;
   }
   
   AssertFatal((pipe >= 0) && (pipe < maxChannel), "SimSoundSequence::onSimSoundSequenceEvent: invalid channel");
   
   modified = true;
   
   channel[pipe].queueId.push_back(event->id);
   channel[pipe].queueSpatial.push_back(event->spatial);
   
   if(event->spatial)
      channel[pipe].queuePos.push_back(event->pos);
   
   return true;
}

//------------------------------------------------------------------------------

bool SimSoundSequence::onSimTimeEvent(const SimTimeEvent *)
{
   if(modified)
   {   
      Sfx::Manager *sfx = static_cast<Sfx::Manager*>(manager->findObject(SimSoundFXObjectId));
      
      AssertFatal(sfx, "SimSoundSequence::onSimTimeEvent: no SimSoundFxObject");
      
      modified = false;
      
      for(int c = 0; c < maxChannel; c++)
      {
         if( channel[c].current != SFX_INVALID_HANDLE )                  
         {
            modified = true;
         
            // check to see if the sound has stopped
            if( sfx->selectHandle( channel[c].current ) == SFX_INVALID_HANDLE ||
                sfx->isDone() )
               channel[c].current = SFX_INVALID_HANDLE;
         }
         
         if( !channel[c].queueId.empty() && (channel[c].current == SFX_INVALID_HANDLE) )
         {
            modified = true;
         
            DWORD id = channel[c].queueId.front();
            bool spatial = channel[c].queueSpatial.front();
            
            channel[c].queueId.pop_front();
            channel[c].queueSpatial.pop_front();
            
            if(spatial)
            {
               TMat3F pos = channel[c].queuePos.front();
               channel[c].queuePos.pop_front();            
               channel[c].current = sfx->playAt(id, pos, Point3F(0, 0, 0));
            }
            else
            {
               channel[c].current = sfx->play(id);
            }
         }
      }
   }
   
   // setup another time event
   SimTimeEvent::post(this, manager->getCurrentTime() + SimTime(updateFrequency));
   
   return true;
}

//------------------------------------------------------------------------------

bool SimSoundSequence::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
   	onEvent(SimSoundSequenceEvent);
      onEvent(SimTimeEvent);
	}
	return false;
}

//------------------------------------------------------------------------------

bool SimSoundSequence::onAdd()
{
	if (!Parent::onAdd())
		return false;
   SimTimeEvent::post(this, manager->getCurrentTime() + SimTime(updateFrequency));
	return true;
}	


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void SimSoundSequence::Queue(SimManager *manager, Int8 pipe, DWORD id)
{
   AssertFatal(!manager->isServer(),"SimSoundSequence::Queue: can only be called from the client");

   SimSoundSequence *sss = (SimSoundSequence *)manager->findObject(SimSoundSequenceObjectId);
   if(sss)
   {
      SimSoundSequenceEvent *newEvent = new SimSoundSequenceEvent();
      newEvent->channel = pipe;
      newEvent->id = id;
      newEvent->spatial = false;
      
      sss->onSimSoundSequenceEvent(newEvent);
      
      delete newEvent;   
   }
}

void SimSoundSequence::QueueAt(SimManager *manager, Int8 pipe, DWORD id, const TMat3F &tmat)   
{
   AssertFatal(!manager->isServer(),"SimSoundSequence::QueueAt: can only be called from the client");

   SimSoundSequence *sss = (SimSoundSequence *)manager->findObject(SimSoundSequenceObjectId);
   if(sss)
   {
      SimSoundSequenceEvent *newEvent = new SimSoundSequenceEvent();
      newEvent->channel = pipe;
      newEvent->id = id;
      newEvent->spatial = true;
      newEvent->pos = tmat;
      
      sss->onSimSoundSequenceEvent(newEvent);
      
      delete newEvent;   
   }
}


//------------------------------------------------------------------------------

void SimSoundSequence::BroadcastQueue(SimManager *manager, Int8 channel, DWORD id)
{
   AssertFatal(manager->isServer(),"SimSoundSequence::BroadcastQueue: can only be called from the server");

   SimSet *packetStreams = static_cast<SimSet *>(manager->findObject(PacketStreamSetId));
   
   if (packetStreams)
	{
		for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
		{
			Net::PacketStream *pStream = dynamic_cast<Net::PacketStream *>(*itr);
			if (pStream)
         {
			   SimSoundSequenceEvent *addEvent = new SimSoundSequenceEvent;
            addEvent->address = SimSoundSequenceObjectId;   // id of object on client
            addEvent->channel = channel;
            addEvent->id = id;
            addEvent->spatial = false;
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }            
		}
	}
}


void SimSoundSequence::BroadcastQueueAt(SimManager *manager, Int8 channel, DWORD id, const TMat3F &tmat)
{
   AssertFatal(manager->isServer(),"SimSoundSequence::BroadcastQueueAt: can only be called from the server");

   SimSet *packetStreams = static_cast<SimSet *>(manager->findObject(PacketStreamSetId));
   
   if (packetStreams)
	{
		for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
		{
			Net::PacketStream *pStream = dynamic_cast<Net::PacketStream *>(*itr);
			if (pStream)
         {
			   SimSoundSequenceEvent *addEvent = new SimSoundSequenceEvent;
            addEvent->address = SimSoundSequenceObjectId;   // id of object on client
            addEvent->channel = channel;
            addEvent->id = id;
            addEvent->spatial = true;
            addEvent->pos = tmat;
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }            
		}
	}
}



//--------------------------------------------------------------------------- 
//
// Persitent sim events
//
//--------------------------------------------------------------------------- 

#include <sim.h>
#include "simbase.h"
#include "simev.h"

//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

int mostSignificantByteBit(int i)
{
   // the fancy version of this had too more loop overhead than it was worth
   if (i < 2)
      return 1;
   else if (i < 4)
      return 2;
   else if (i < 8)
      return 3;
   else if (i < 16)
      return 4;
   else if (i < 32)
      return 5;
   else if (i < 64)
      return 6;
   else if (i < 128)
      return 7;
   return 8;
}

int mostSignificantIntBit(int i)
{
   if (i & 0xffffff00) {
      if (i & 0xffff0000) {
         if (i & 0xff000000)
            return 24 + mostSignificantByteBit((i >> 24) & 0xff);   
         else
            return 16 + mostSignificantByteBit((i >> 16) & 0xff);   
      }
      else 
         return 8 + mostSignificantByteBit((i >> 8) & 0xff);
   }
   else
      return mostSignificantByteBit(i & 0xff);
}

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG( SimMessageEvent, SimMessageEventType );

void SimMessageEvent::unpack(
   SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   if(!verifyNotServer(manager))
      return;

   SimEvent::unpack(manager, ps, bstream);
   
   if (bstream->readFlag())
      message = bstream->readInt(1);
   else if (bstream->readFlag())
      message = bstream->readInt(4);
   else if (bstream->readFlag())
      message = bstream->readInt(8);
   else if (bstream->readFlag())
      message = bstream->readInt(16);
   else if (bstream->readFlag())
      message = bstream->readInt(32);
}

//--------------------------------------------------------------------------- 

void SimMessageEvent::pack(
   SimManager *manager, Net::PacketStream *ps, BitStream *bstream)
{
   SimEvent::pack(manager, ps, bstream);

   // optimized for small messages, like bools
   int sigBits = mostSignificantIntBit(message);
   if (sigBits == 1) {
      bstream->writeFlag(true);
      bstream->writeInt(message, 1);
   }
   else {
      bstream->writeFlag(false);
      if (sigBits <= 4) {
         bstream->writeFlag(true);
         bstream->writeInt(message, 4);
      }            
      else {   
         bstream->writeFlag(false);
         if (sigBits <= 8) {
            bstream->writeFlag(true);
            bstream->writeInt(message, 8);
         }            
         else {   
            bstream->writeFlag(false);
            if (sigBits <= 16) {
               bstream->writeFlag(true);
               bstream->writeInt(message, 16);
            }            
            else {   
               bstream->writeFlag(false);
               bstream->writeInt(message, 32);
            }
         }         
      }
   }   
}

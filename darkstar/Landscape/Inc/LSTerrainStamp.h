//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _LSTERRAINSTAMP_H_
#define _LSTERRAINSTAMP_H_

#include <persist.h>
#include <tVector.h>
#include <LS.h>

class LSTerrainStamp: public Persistent::Base 
{
public:

   class TextureType
   {
   public:
      Point2I  tile;             // x,y position in world coordinates
      BYTE     flags;            // flip flag, rotate, and hole states
      Int32    textureID;          
   };

   Vector<TextureType> texList;// list of textures to apply
   Vector<Point2I> flatList;   // list of tiles to flatten

   Int32 clamp_max_detail;     // max detail levels to clamp after flattening

public:
   LSTerrainStamp() { clamp_max_detail=0; }

   // Persistent functions
   DECLARE_PERSISTENT(LSTerrainStamp);
   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user);
};

#endif //_LSTERRAINSTAMP_H_

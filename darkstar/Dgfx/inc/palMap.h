//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PALMAP_H_
#define _PALMAP_H_

#include <persist.h>
#include <tBitVector.h>


class PaletteMap: public Persistent::VersionedBase
{
public:
   enum constants {
      PMP_VERSION = 1,   
   };
   struct HAZE_SHADE
   {
      BitVector inclusion[256];
      float max;
      int levels;      
   };
   HAZE_SHADE haze;
   HAZE_SHADE shade;
   BYTE hazeR;
   BYTE hazeG;
   BYTE hazeB;
   float gamma;
   bool  useLUV;

   PaletteMap();

   //--------------------------------------
   DECLARE_PERSISTENT(PaletteMap);
   int version();
   Persistent::Base::Error read( StreamIO &, int version, int user = 0 );
   Persistent::Base::Error write( StreamIO &, int version, int user = 0 );
};



#endif //_PALMAP_H_

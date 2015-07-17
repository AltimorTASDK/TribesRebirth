//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRFN_H_
#define _GRFN_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Rendition {

// Local constants, structures, and fileScope data...
//
const UInt32 GR_TEXTURE             = 1 << 0;

const UInt32 GR_SHADE_NONE          = 1 << 1;        // shade state
const UInt32 GR_SHADE_CONSTANT      = 1 << 2;
const UInt32 GR_SHADE_VERTEX        = 1 << 3;

const UInt32 GR_HAZE_NONE           = 1 << 4;        // haze state
const UInt32 GR_HAZE_CONSTANT       = 1 << 5;
const UInt32 GR_HAZE_VERTEX         = 1 << 6;

const UInt32 GR_ALPHA_NONE          = 1 << 7;        // alpha state
const UInt32 GR_ALPHA_CONSTANT      = 1 << 8;
const UInt32 GR_ALPHA_TEXTURE       = 1 << 9;
const UInt32 GR_ALPHA_VERTEX        = 1 << 10;

const UInt32 GR_TRANSPARENCYENABLED = 1 << 11;
const UInt32 GR_PERSPECTIVEENABLED  = 1 << 12;

const UInt32 GR_FILLMASK         = GR_TEXTURE;
const UInt32 GR_SHADEMASK        = (GR_SHADE_NONE | GR_SHADE_CONSTANT | GR_SHADE_VERTEX);
const UInt32 GR_HAZEMASK         = (GR_HAZE_NONE  | GR_HAZE_CONSTANT  | GR_HAZE_VERTEX);
const UInt32 GR_ALPHAMASK        = (GR_ALPHA_NONE | GR_ALPHA_CONSTANT | GR_ALPHA_VERTEX | GR_ALPHA_TEXTURE);
const UInt32 GR_TRANSPARENCYMASK = GR_TRANSPARENCYENABLED;
const UInt32 GR_PERSPECTIVEMASK  = GR_PERSPECTIVEENABLED;



namespace {

// Local utility functions...
//
// FLTTO* functions from RRedline FAQ, converted to inlines for type safety...
//
inline UInt32
FLTOC(const float in_red,
      const float in_green,
      const float in_blue)
{
   DWORD red   = UInt8(in_red   * 255.0f);
   DWORD green = UInt8(in_green * 255.0f);
   DWORD blue  = UInt8(in_blue  * 255.0f);
   
   return ((red << 16) | (green << 8) | blue);
}

inline UInt32
FLTOAC(const float in_red,
       const float in_green,
       const float in_blue,
       const float in_alpha)
{
   DWORD red   = UInt8(in_red   * 255.0f);
   DWORD green = UInt8(in_green * 255.0f);
   DWORD blue  = UInt8(in_blue  * 255.0f);
   DWORD alpha = UInt8(in_alpha * 255.0f);
   
   return ((alpha << 24) | (red << 16) | (green << 8) | blue);
}

inline Int32
FLTOIFIX(const float in_val)
{
   return Int32(in_val * (1 << 16));
}

inline Int32
ITOIFIX(const Int32 in_val)
{
   return (in_val << 16);
}

inline Int32
FLTOQFIX(const float in_val)
{
   return Int32(in_val * (1 << 24) - 1);
}

// For converting RGB vals into dword suitable for BITBLTing...
//
inline v_u32
getRedlineColor(const UInt8 in_red,
                const UInt8 in_green,
                const UInt8 in_blue)
{
   UInt32 temp = (in_red << 11) | (in_green << 5) | in_blue;
   return v_u32(temp | (temp << 16));
}

}; // namespace {}

}; // namespace Rendition

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GRFN_H_

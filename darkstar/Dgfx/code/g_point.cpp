//================================================================
//   
// $Workfile:   g_point.cpp  $
// $Version$
// $Revision:   1.0  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "g_surfac.h"
#include "g_raster.h"


void GFXDrawPoint2d(
   GFXSurface        *io_pSurface,
   const Point2I      *in_at,
   float, // w coord, unused in this function.
   DWORD             in_color )
{
   if (io_pSurface->getFlag( GFX_DMF_RCLIP ))
   {
		register RectI *clipRect = io_pSurface->getClipRect();
      if ( (in_at->x < clipRect->upperL.x) 
         ||(in_at->x > clipRect->lowerR.x)
         ||(in_at->y < clipRect->upperL.y)
         ||(in_at->y > clipRect->lowerR.y) )
            return;
   }
   *(io_pSurface->getAddress(*in_at)) = (BYTE)in_color;
}


//================================================================
//   
// $Workfile:   g_clip.cpp  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#include "ml.h"
#include "g_raster.h"
#include "d_defs.h"

Bool  cliptest( LONG p, LONG q, LONG *u1, LONG *u2 );    // function proto for local function

//================================================================
//                 Clip point to destination rectangle
//================================================================

Bool rectClip(
   const Point2I *in_pt,    // Point to test
   const RectI *in_clip )   // Rectangle to test against
{
   return ( (in_pt->x >= in_clip->upperL.x)
         && (in_pt->x <= in_clip->lowerR.x)
         && (in_pt->y >= in_clip->upperL.y)
         && (in_pt->y <= in_clip->lowerR.y)
         );
}

//========================================================================
//              Clip raster list to destination rectangle
//========================================================================
//
// returns TRUE if object is entirely clipped away
//
// This function takes an object that has been rasterized and sets the 
// the left and right edge clip information and modifies the context
// for the top and bottom clip.
//
// This function assumes that the list has not been clipped yet and 
// contains no dead rasters.  If you want to use this as a post process,
// you would have to check both x and lx values as well as cnt and rx.
// Also, you should add code to skip the left right check if it is a 
// dead raster in the while loop that checks for left/right clip
//

Bool rectClip(
   GFXRasterList *rlist,          // graphics context
   RectI *in_clipRect,               // rectangular region to clip against
   RectI *io_lastRect )              // last bounding rectangle
{
   GFXRaster   *rPtr;               // pointer to raster list line
   GFXRaster   *prevPtr;            // pointer to previous raster list line
   int         newTop=0;            // index to new top, starts at 0 (current top)
   int         bottom;              // y-coordinate of current bottom
   LONG        clx,crx,cty,cby;     // clip left_x, right_x, top_y, and bottom_y
   Bool        topFound=FALSE;      // true if new top has been found
   int         count;               // amount clipped off left or right side

   clx = in_clipRect->upperL.x;     // get clip coordinates
   crx = in_clipRect->lowerR.x;
   cty = in_clipRect->upperL.y;
   cby = in_clipRect->lowerR.y;

   // clip top
   if ( rlist->top_y > cby )
      return TRUE;                 // object is entirely below bottom of clip region

   if ( (rlist->top_y+rlist->height) < cty )
      return TRUE;                 // object is entirely above top of cllp region

   if ( (cty - rlist->top_y) > 0 )
      newTop = cty - rlist->top_y;   // top is above top of clip region
   else                             // top is below top of clip region
      topFound = TRUE;

   bottom = rlist->top_y + newTop;   // seed current bottom y-coordinate

   // clip left and right and check for bottom
   rPtr = &rlist->topRaster[newTop]; // get pointer to top raster
   prevPtr = rPtr;                                 // seed previous raster pointer
   while ( rPtr )
   {
      // if dead raster, put code here to skip this left/right check
      // current routine assumes no dead rasters
      if ( (rPtr->x > crx) || (rPtr->x+rPtr->cnt < clx) )
         rPtr->f |= RASTER_DEAD;       // line is outside rectangle, mark as dead
      else                             // else line lies inside clip region
      {
         if (!topFound)                // if a valid top has not been found yet,
            topFound = TRUE;           // indicate a valid top has been found!

         if ( (count = rPtr->x-clx) < 0 )
         {
            rPtr->x -= count;          // get new left clipped coordinate
            rPtr->cnt += count;        // modify count
            rPtr->lx = rPtr->x;        // set new left clipped coordinate
            rPtr->f |= RASTER_LCLIPPED;   // indicate raster clipped on left
         }
         if ( (count = crx-(rPtr->x+rPtr->cnt)) < 0 )
         {
            rPtr->cnt += count;        // get new clipped count
            rPtr->rx = rPtr->x+rPtr->cnt; // set new right clipped coordinate
            rPtr->f |= RASTER_RCLIPPED;   // indicate raster clipped on right
         }
      }

      if ( rPtr->f & RASTER_STEP_Y )
      {
         if ( (bottom) > cby )       // if next bottom is below bottom clip region
         {
            prevPtr->next = NULL;      // terminate list on previous raster line
            break;                     // do not process next raster line, we're finished
         }
         else
            bottom++;                  // else keep track of bottom coordinate

         if (!topFound)                // if we haven't found a valid top yet,
            newTop++;                  // keep incrmenting top until one is found
      }

      prevPtr = rPtr;                  // save this raster line as previous raster
      rPtr = rPtr->next;               // get pointer to next rast
   }

   if ( topFound )
   {
      rlist->topRaster = &rlist->topRaster[newTop];
      newTop = rlist->top_y + newTop; 
      rlist->top_y = newTop;      // set top to top of clip region
      rlist->height = bottom - newTop; // set new height
      rlist->topRaster->f = 0;    // clear STEP_Y flag on first raster
      io_lastRect->upperL.y = newTop;           // set last rect's top y-coordinate
      io_lastRect->lowerR.y = bottom;           // set last rect's bottom y-coordinate
   }

   return ( !topFound );               // return TRUE if top was NOT found
                                       // this indicates that object was entirely clipped
}

//========================================================================
//                  Clip line to destination rectangle
//========================================================================
//
// Clips lines to rectangle.  Returns TRUE if line outside rectangle.
//
// This uses a Liang and Barsky line-clipping algorithm that is 
// supposed to be pretty fast because it reduces the number of
// calculations required to clip the line.
//
// might convert to assembly for speed
//

inline void swap(LONG &a, LONG &b)
{
   a ^= b;
   b ^= a;
   a ^= b;
}

Bool rectClip(
   Point2I *io_pt1,               // First point of the line to clip
   Point2I *io_pt2,               // Second ponit of the line to clip
   const RectI *in_clip )         // Rectangle to clip against
{
   Bool  flipped = FALSE;        // TRUE if the points are flipped, false if not
   LONG  dx,dy;
   LONG  x1,y1,x2,y2;            // line points
   LONG  xwmin,xwmax,ywmin,ywmax;// rectangle clipping window values
   
   xwmin = in_clip->upperL.x;    // get clip window extents
   xwmax = in_clip->lowerR.x;
   ywmin = in_clip->upperL.y;
   ywmax = in_clip->lowerR.y;


   x1 = io_pt1->x;               // get points
   y1 = io_pt1->y;
   x2 = io_pt2->x;
   y2 = io_pt2->y;

   // check for trivial rejection
   
   if((x1 > xwmax && x2 > xwmax) || (x1 < xwmin && x2 < xwmin))
      return FALSE;
   
   if((y1 > ywmax && y2 > ywmax) || (y1 < ywmin && y2 < ywmin))
      return FALSE;
   
   if(x1 > x2) 
   {
      swap(x1, x2);
      swap(y1, y2);
      flipped = !flipped;
   }
   dx = x2 - x1;
   dy = y2 - y1;
   
   // OK, let's clip the X coords here
   
   float t;  // parametric value
   
   if(x1 < xwmin) 
   {
      // we need to clip point 1 in x
      t = (xwmin - x1) / float(dx);
      x1 = xwmin;
      y1 += LONG(t * dy);
      dx = x2 - x1;
      dy = y2 - y1;
   }
   if(x2 > xwmax)
   {
      t = (xwmax - x1) / float(dx);
      x2 = xwmax;
      y2 = LONG(y1 + t * dy);
      dx = x2 - x1;
      dy = y2 - y1;
   }
   if((y1 > ywmax && y2 > ywmax) || (y1 < ywmin && y2 < ywmin))
      return FALSE;
   if(y1 > y2) 
   {
      swap(x1, x2);
      swap(y1, y2);
      flipped = !flipped;
      dx = x2 - x1;
      dy = y2 - y1;
   }
   if(y1 < ywmin)
   {
      t = (ywmin - y1) / float(dy);
      y1 = ywmin;
      x1 += LONG(t * dx);
      dx = x2 - x1;
      dy = y2 - y1;
   }
   if(y2 > ywmax)
   {
      t = (ywmax - y1) / float(dy);
      y2 = ywmax;
      x2 = LONG(x1 + t * dx);
   }
   if(flipped) 
   {
      io_pt2->x = x1;
      io_pt2->y = y1;
      io_pt1->x = x2;
      io_pt1->y = y2;
   } 
   else 
   {
      io_pt1->x = x1;
      io_pt1->y = y1;
      io_pt2->x = x2;
      io_pt2->y = y2;
   }
   return TRUE;            // TRUE if any part of line is visible
}

//================================================================
//         Clip destination rectangle to output clipping region
//================================================================

Bool rectClip(
   RectI *io_rect,             // Rectangle to clip
   const RectI *in_clip )      // Rectangle to clip against
{
   io_rect->upperL.x = max( io_rect->upperL.x, in_clip->upperL.x );
   io_rect->upperL.y = max( io_rect->upperL.y, in_clip->upperL.y );
   io_rect->lowerR.x = min( io_rect->lowerR.x, in_clip->lowerR.x );
   io_rect->lowerR.y = min( io_rect->lowerR.y, in_clip->lowerR.y );

   return ( ((io_rect->lowerR.x - io_rect->upperL.x) >= 0)
         && ((io_rect->lowerR.y - io_rect->upperL.y) >= 0)
         );
}

//================================================================
//          Clip Source Rect and Dest Rect to clipping region
//================================================================
// This function is used for clipping two non-stretched 
// sub-regions during a rect-to-rect operation.  If the source 
// rect is clipped the corresponding dest rect is clipped to match
// the dimensions of the new source rect.  The converse is true also.

Bool rectClip(
   RectI *io_src,           // Source rectangle to clip
   RectI *io_dest,          // Destination rectangle to clip
   const RectI *in_clip)    // Rectangle to clip against
{
   //clip io_src to top left corner
   if (io_dest->upperL.x < in_clip->upperL.x)
   {
      io_src->upperL.x += in_clip->upperL.x - io_dest->upperL.x;
      io_dest->upperL.x  = in_clip->upperL.x;
   }
   if (io_dest->upperL.y < in_clip->upperL.y)
   {
      io_src->upperL.y += in_clip->upperL.y - io_dest->upperL.y;
      io_dest->upperL.y  = in_clip->upperL.y;
   }

   //clip io_src to lower right corner
   if (io_dest->lowerR.x > in_clip->lowerR.x)
   {
      io_src->lowerR.x -= io_dest->lowerR.x - in_clip->lowerR.x;
      io_dest->lowerR.x  = in_clip->lowerR.x;
   }
   if (io_dest->lowerR.y > in_clip->lowerR.y)
   {
      io_src->lowerR.y -= io_dest->lowerR.y - in_clip->lowerR.y;
      io_dest->lowerR.y  = in_clip->lowerR.y;
   }

   //is there anything left?
   if ( (io_src->upperL.x > io_src->lowerR.x) || (io_src->upperL.y > io_src->lowerR.y) )
      return FALSE;

   return TRUE;
}

//================================================================
//                    Clip Source Rect to Dest Rect
//              Source and Dest clip regions are different
//================================================================
// This function is used for clipping two non-stretched 
// sub-regions during a rect-to-rect operation.  If the source 
// rect is clipped the corresponding dest rect is clipped to match
// the dimensions of the new source rect.  The converse is true also.
// This function is used when the clip regions on the source and
// the dest are not the same.  Such is the case when copying a sub
// region of a bitmap to a buffer of different dimensions

Bool rectClip(
   RectI *io_src,              // Source rectangle to clip
   const RectI *in_srcClip,    // Sources clip region
   RectI *io_dest,             // Dest rectangle to clip
   const RectI *in_clip)       // Dests clip region
{
   // clip source coordinates to source clip rectangle

   //check source left edge
   if ( io_src->upperL.x < in_srcClip->upperL.x )
   {
      io_dest->upperL.x += in_srcClip->upperL.x - io_src->upperL.x;
      io_src->upperL.x =  in_srcClip->upperL.x;
   }
   //check source top edge
   if ( io_src->upperL.y < in_srcClip->upperL.y )
   {
      io_dest->upperL.y += in_srcClip->upperL.y - io_src->upperL.y;
      io_src->upperL.y =  in_srcClip->upperL.y;
   }
   //check source right edge
   if ( io_src->lowerR.x > in_srcClip->lowerR.x )
   {
      io_dest->lowerR.x += in_srcClip->lowerR.x-io_src->lowerR.x;
      io_src->lowerR.x =  in_srcClip->lowerR.x;
   }
   if ( io_src->lowerR.y > in_srcClip->lowerR.y )
   {
      io_dest->lowerR.y += in_srcClip->lowerR.y-io_src->lowerR.y;
      io_src->lowerR.y =  in_srcClip->lowerR.y;
   }

   // clip dest coordinates to dest clip rectangle
   if ( io_dest->upperL.x < in_clip->upperL.x)
   {
      io_src->upperL.x += in_clip->upperL.x - io_dest->upperL.x;
      io_dest->upperL.x =  in_clip->upperL.x;
   }
   if ( io_dest->upperL.y < in_clip->upperL.y )
   {
      io_src->upperL.y += in_clip->upperL.y - io_dest->upperL.y;
      io_dest->upperL.y =  in_clip->upperL.y;
   }
   if ( io_dest->lowerR.x > in_clip->lowerR.x )
   {
      io_src->lowerR.x += in_clip->lowerR.x - io_dest->lowerR.x;
      io_dest->lowerR.x =  in_clip->lowerR.x ;
   }
   if ( io_dest->lowerR.y > in_clip->lowerR.y )
   {
      io_src->lowerR.y += in_clip->lowerR.y - io_dest->lowerR.y;
      io_dest->lowerR.y =  in_clip->lowerR.y;
   }

   //is there anything left?
   if ( (io_src->upperL.x > io_src->lowerR.x) || (io_src->upperL.y > io_src->lowerR.y) )
      return FALSE;

   return TRUE;
}

//================================================================
//     Clip Source Rect and Stretched Dest Rect to clipping region
//================================================================
// This function is used for clipping a non-to-stretched 
// sub-regions during a rect-to-stretchedrect operation.  If the source 
// rect is clipped the corresponding dest rect is clipped to match
// the dimensions of the new source rect.  The converse is true also.

Bool rectClipScaled(
   RectI 	*io_src,        // Source rectangle to clip
   RectI 	*io_dest,       // Destination rectangle to clip
   const 	RectI *in_clip, 	// Rectangle to clip against
   Int32 	*out_xAdj,    	// adjustment to source:dest ratio on X-axis
   Int32 	*out_yAdj,    	// adjustment to source:dest ratio on Y-axis
   GFXFlipFlag	in_flip )	// flip flags
{
   // clip source coordinates to source clip rectangle
   Int32 BDstW = io_dest->lowerR.x - io_dest->upperL.x + 1;
   Int32 BDstH = io_dest->lowerR.y - io_dest->upperL.y + 1;
   Int32 BSrcW = io_src->lowerR.x - io_src->upperL.x + 1;
   Int32 BSrcH = io_src->lowerR.y - io_src->upperL.y + 1;

   // clip dest coordinates to dest clip rectangle
   if ( io_dest->lowerR.x > in_clip->lowerR.x )
   {
      *out_xAdj         = (in_flip&GFX_FLIP_X)? (in_clip->lowerR.x - io_dest->lowerR.x):0;
      io_src->lowerR.x += ((in_clip->lowerR.x - io_dest->lowerR.x) * BSrcW) / BDstW;
      io_dest->lowerR.x =  in_clip->lowerR.x ;
   }
   if ( io_dest->lowerR.y > in_clip->lowerR.y )
   {
      *out_yAdj         = (in_flip&GFX_FLIP_Y)? (in_clip->lowerR.y - io_dest->lowerR.y):0;
      io_src->lowerR.y += ((in_clip->lowerR.y - io_dest->lowerR.y) * BSrcH) / BDstH;
      io_dest->lowerR.y =  in_clip->lowerR.y;
   }
   if ( io_dest->upperL.x < in_clip->upperL.x)
   {
      *out_xAdj         = (in_flip&GFX_FLIP_X)? 0:(in_clip->upperL.x - io_dest->upperL.x);
      io_src->upperL.x += (*out_xAdj * BSrcW) / BDstW;
      io_dest->upperL.x =  in_clip->upperL.x;
   }
   if ( io_dest->upperL.y < in_clip->upperL.y )
   {
      *out_yAdj         = (in_flip&GFX_FLIP_Y)? 0:(in_clip->upperL.y - io_dest->upperL.y);
      io_src->upperL.y += (*out_yAdj * BSrcH) / BDstH;
      io_dest->upperL.y =  in_clip->upperL.y;
   }

   //is there anything left?
   //since it's scaled we must also check the destination rect
   if ( (io_src->upperL.x > io_src->lowerR.x)
     || (io_src->upperL.y > io_src->lowerR.y)
     || (io_dest->upperL.x > io_dest->lowerR.x)
     || (io_dest->upperL.y > io_dest->lowerR.y)
      )
      return FALSE;

   return TRUE;
}

//================================================================
//               Clip Source Rect to Stretched Dest Rect
//              Source and Dest clip regions are different
//================================================================
// This function is used for clipping a non-to-stretched 
// sub-regions during a rect-to-stretchedrect operation.  If the source 
// rect is clipped the corresponding dest rect is clipped to match
// the dimensions of the new source rect.  The converse is true also.
// This function is used when the clip regions on the source and
// the dest are not the same.  Such is the case when copying a sub
// region of a bitmap to a buffer of different dimensions

Bool rectClipScaled(
   RectI *io_src,              // Source rectangle to clip
   const RectI *in_srcClip,    // Sources clip region
   RectI *io_dest,             // Destination rectangle to clip
   const RectI *in_clip,       // Dests clip region
   UInt32 *out_xAdj,          // adjustment to source:dest ratio on X-axis
   UInt32 *out_yAdj)          // adjustment to source:dest ratio on Y-axis
{
   // clip source coordinates to source clip rectangle
   Int32 BDstW = io_dest->lowerR.x - io_dest->upperL.x + 1;
   Int32 BDstH = io_dest->lowerR.y - io_dest->upperL.y + 1;
   Int32 BSrcW = io_src->lowerR.x - io_src->upperL.x + 1;
   Int32 BSrcH = io_src->lowerR.y - io_src->upperL.y + 1;

   //check source left edge
   if ( io_src->upperL.x < in_srcClip->upperL.x )
   {
      //BUG!!!!  out_xAdj needs to be changed to mach the above rectClipScaled stuff
      *out_xAdj         -= io_dest->upperL.x;
      io_dest->upperL.x += ((in_srcClip->upperL.x - io_src->upperL.x) * BDstW)/BSrcW;
      io_src->upperL.x   =  in_srcClip->upperL.x;
   }
   //check source top edge
   if ( io_src->upperL.y < in_srcClip->upperL.y )
   {
      *out_yAdj         -= io_dest->upperL.y;
      io_dest->upperL.y += ((in_srcClip->upperL.y - io_src->upperL.y) * BDstH)/BSrcH;
      io_src->upperL.y   =  in_srcClip->upperL.y;
   }
   //check source right edge
   if ( io_src->lowerR.x > in_srcClip->lowerR.x )
   {
      io_dest->lowerR.x += in_srcClip->lowerR.x-io_src->lowerR.x;
      io_src->lowerR.x   = in_srcClip->lowerR.x;
   }
   if ( io_src->lowerR.y > in_srcClip->lowerR.y )
   {
      io_dest->lowerR.y += in_srcClip->lowerR.y-io_src->lowerR.y;
      io_src->lowerR.y   = in_srcClip->lowerR.y;
   }

   // clip dest coordinates to dest clip rectangle
   if ( io_dest->upperL.x < in_clip->upperL.x)
   {
      *out_xAdj        -= io_dest->upperL.x;
      io_src->upperL.x += (in_clip->upperL.x - io_dest->upperL.x) * BSrcW/BDstW;
      io_dest->upperL.x =  in_clip->upperL.x;
   }
   if ( io_dest->upperL.y < in_clip->upperL.y )
   {
      *out_yAdj        -= io_dest->upperL.y;
      io_src->upperL.y += (in_clip->upperL.y - io_dest->upperL.y) * BSrcH/BDstH;
      io_dest->upperL.y =  in_clip->upperL.y;
   }
   if ( io_dest->lowerR.x > in_clip->lowerR.x )
   {
      io_src->lowerR.x += in_clip->lowerR.x - io_dest->lowerR.x;
      io_dest->lowerR.x = in_clip->lowerR.x ;
   }
   if ( io_dest->lowerR.y > in_clip->lowerR.y )
   {
      io_src->lowerR.y += in_clip->lowerR.y - io_dest->lowerR.y;
      io_dest->lowerR.y = in_clip->lowerR.y;
   }

   //is there anything left?
   //since it's scaled we must also check the destination rect
   if ( (io_src->upperL.x > io_src->lowerR.x)
     || (io_src->upperL.y > io_src->lowerR.y)
     || (io_dest->upperL.x > io_dest->lowerR.x)
     || (io_dest->upperL.y > io_dest->lowerR.y)
      )
      return FALSE;

   return TRUE;
}

//================================================================
// Create a third rectangle from the union of two other rectangles
//================================================================

void rectUnion(
   const RectI *in_a,    // First source rectangle
   const RectI *in_b,    // Second source rectangle
   RectI *out)           // Union result
{
   out->upperL.x = min(in_a->upperL.x, in_b->upperL.x);
   out->upperL.y = min(in_a->upperL.y, in_b->upperL.y);
   out->lowerR.x = max(in_a->lowerR.x, in_b->lowerR.x);
   out->lowerR.y = max(in_a->lowerR.y, in_b->lowerR.y);
}


BOOL validateRect(const RectI *in_rect, const RectI *in_clipRect)
{
   return rectClip(&(in_rect->upperL), in_clipRect) && 
          rectClip(&(in_rect->lowerR), in_clipRect);
}

//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_surfac.h>
#include <g_pal.h>

#include <ptCore.h>
#include <ptControl.h>


//------------------------------------------------------------------------------
SelectBar::SelectBar()
{
   focus = false;
   iSelect = 0;
}   

//------------------------------------------------------------------------------
void SelectBar::render(GFXSurface *surface)
{
   // draw column selection
   Point2I from, to;
   from.set(XOFS+SZ/2+(iSelect*(SZ+SP))-1, 2);
   to.set(from.x, 18*(SZ+SP)-2);
   surface->drawLine2d( &from, &to, COLOR); from.x++; to.x++;
   surface->drawLine2d( &from, &to, COLOR); from.x++; to.x++;
   surface->drawLine2d( &from, &to, COLOR); 
}

//------------------------------------------------------------------------------
void SelectBar::onMouseMove(int x, int y, UINT keyFlags)
{
   y, keyFlags;
   if (focus)
      if (x > XOFS && x < XOFS+16*(SZ+SP))
         iSelect = (x-XOFS)/(SZ+SP);
}


//------------------------------------------------------------------------------
void SelectBar::onLButtonUp(int x, int y, UINT keyFlags)
{
   x, y, keyFlags;
   focus = false;
}


//------------------------------------------------------------------------------
void SelectBar::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   fDoubleClick, keyFlags;
   if ( (y < YOFS) || (y > 17*(SZ+SP)-SP && y <= 18*(SZ+SP)) )
      if (x > XOFS && x < XOFS+16*(SZ+SP))
      {
         focus = true;
         iSelect = (x-XOFS)/(SZ+SP);
      }
}




//------------------------------------------------------------------------------
PaletteGrid::PaletteGrid()
{
   selection.from.setSize(256);
   selection.to.setSize(256);
   selection.from.zero();
   selection.to.zero();
   selection.from_copy.setSize(256);
   selection.from_copy.zero();
   selection.to_copy.setSize(256);
   selection.to_copy.zero();
   selection.from_anchor = -1;
   selection.to_anchor = -1;
   fromColor = 253;
   toColor   = 254;
   shColor   = 252;
   lbutton = rbutton = false;
   iOver = iShadeHaze = -1;
}

//------------------------------------------------------------------------------
void PaletteGrid::render(GFXSurface *surface)
{
   //--------------------------------------
   // draw palette grid
   RectI rect;
   for (int y=0; y<16; y++)
   {
      rect(0,0,SZ,SZ);
      rect += Point2I(XOFS, YOFS);
      rect += Point2I(0, y*(SZ+SP));
      for (int x=0; x<16; x++ )
      {         int color = (x*16)+y;
         surface->drawRect2d_f( &rect, color);

         //adjust rect position...
         rect += Point2I(SZ+SP, 0);
      }
   }

   //--------------------------------------
   // draw from/to selections
   for (int y=0; y<16; y++)
   {
      rect(0,0,SZ+2,SZ+2);
      rect += Point2I(XOFS-1, YOFS-1);
      rect += Point2I(0, y*(SZ+SP));
      for (int x=0; x<16; x++ )
      {
         int index = (x*16)+y;
         if (selection.from.test(index))
         {
            surface->drawLine2d( &rect.upperL, &Point2I(rect.lowerR.x-2, rect.upperL.y), fromColor);  // top
            surface->drawLine2d( &rect.upperL, &Point2I(rect.upperL.x, rect.lowerR.y-2), fromColor);  // left
            rect.upperL += Point2I(1,1); rect.lowerR -= Point2I(1,1);   //shift it in
            surface->drawLine2d( &rect.upperL, &Point2I(rect.lowerR.x-2, rect.upperL.y), fromColor);  // top
            surface->drawLine2d( &rect.upperL, &Point2I(rect.upperL.x, rect.lowerR.y-2), fromColor);  // left
            rect.upperL -= Point2I(1,1); rect.lowerR += Point2I(1,1);   //shift it back out
         }
         if (selection.to.test(index))
         {
            surface->drawLine2d( &Point2I(rect.upperL.x+2, rect.lowerR.y), &rect.lowerR, toColor);    // botttom
            surface->drawLine2d( &Point2I(rect.lowerR.x, rect.upperL.y+2), &rect.lowerR, toColor);    // right
            rect.upperL += Point2I(1,1); rect.lowerR -= Point2I(1,1);   //shift it in
            surface->drawLine2d( &Point2I(rect.upperL.x+2, rect.lowerR.y), &rect.lowerR, toColor);    // botttom
            surface->drawLine2d( &Point2I(rect.lowerR.x, rect.upperL.y+2), &rect.lowerR, toColor);    // right
            rect.upperL -= Point2I(1,1); rect.lowerR += Point2I(1,1);   //shift it back out
         }

         //adjust rect position...
         rect += Point2I(SZ+SP, 0);
      }
   }

   //--------------------------------------
   // draw current haze shde slelection
   if (iShadeHaze != -1)
   {
      int w = SZ/2;
      Point2I pt( XOFS+(SZ/2), YOFS+(SZ/2) );
      pt.x += (iShadeHaze >> 4)*(SZ+SP);
      pt.y += (iShadeHaze & 0xf)*(SZ+SP);
      surface->drawLine2d( &Point2I(pt.x-w, pt.y), &Point2I(pt.x+w, pt.y), shColor);  // horz
      surface->drawLine2d( &Point2I(pt.x, pt.y-w), &Point2I(pt.x, pt.y+w), shColor);  // vert
      iShadeHaze = -1;
   }
}

//------------------------------------------------------------------------------
int PaletteGrid::getOver(int x, int y)
{
   iOver = -1;
   if ( (y >= YOFS) && y < YOFS+16*(SZ+SP))
      if (x > XOFS && x < XOFS+16*(SZ+SP))
      {
         x = (x-XOFS)/(SZ+SP);
         y = (y-YOFS)/(SZ+SP);
         iOver = (x*16)+y;
         AssertFatal( iOver >= 0 && iOver<=255, "out of bounds");
      }
   return (iOver);   
}   


//------------------------------------------------------------------------------
void PaletteGrid::zeroSelection()
{
   lbutton = rbutton = false;   
   selection.from.zero();
   selection.to.zero();
}   

//------------------------------------------------------------------------------
void PaletteGrid::onMouseMove(int x, int y, UINT keyFlags)
{
   keyFlags;

   if (getOver(x,y) == -1)
      return;
   
   if (lbutton) 
   {
      selection.from.zero();
      selection.from.set(selection.from_anchor, iOver); 
      if (keyFlags & MK_CONTROL)
      {
         // toggle on colors in selection
         selection.from.xor(selection.from_copy);
      }
   }
   
   if (rbutton)
   {
      selection.to.zero();
      selection.to.set(selection.to_anchor, iOver);
      if (keyFlags & MK_CONTROL)
      {
         // toggle on colors in selection
         selection.to.xor(selection.to_copy);
      }
   }
}

//------------------------------------------------------------------------------
void PaletteGrid::onLButtonUp(int x, int y, UINT keyFlags)
{
   x; y; keyFlags;
   
   lbutton = false;
}

//------------------------------------------------------------------------------
void PaletteGrid::onRButtonUp(int x, int y, UINT keyFlags)
{
   x; y; keyFlags;

   rbutton = false;
}

//------------------------------------------------------------------------------
void PaletteGrid::onLButtonDown(BOOL, int x, int y, UINT keyFlags)
{
   getOver(x,y);

   if (iOver == -1) 
   {
      lbutton = false;
      return;
   }
   
   lbutton = true;

   if (!(keyFlags & MK_SHIFT) && !(keyFlags & MK_CONTROL))
   {
      // this is a new selection
      selection.from_anchor = iOver;
      onMouseMove(x,y,keyFlags);
      return;
   }

   if (keyFlags & MK_SHIFT)
   {
      // this is a continuing selection,
      onMouseMove(x,y,keyFlags);
      return;
   }

   if (keyFlags & MK_CONTROL)
   {
      // this is a toggling of an
      // additional region - so set a new anchor
      selection.from_anchor = iOver;
      selection.from_copy = selection.from;
      onMouseMove(x,y,keyFlags);
      return;
   }
}

//------------------------------------------------------------------------------
void PaletteGrid::onRButtonDown(BOOL, int x, int y, UINT keyFlags)
{
   getOver(x,y);

   if (iOver == -1) 
   {
      rbutton = false;
      return;
   }
   
   rbutton = true;
   
   if (!(keyFlags & MK_SHIFT) && !(keyFlags & MK_CONTROL))
   {
      // this is a new selection
      selection.to_anchor = iOver;
      onMouseMove(x,y,keyFlags);
      return;
   }

   if (keyFlags & MK_SHIFT)
   {
      // this is a continuing selection,
      onMouseMove(x,y,keyFlags);
      return;
   }

   if (keyFlags & MK_CONTROL)
   {
      // this is a toggling of an
      // additional region - so set a new anchor
      selection.to_anchor = iOver;
      selection.to_copy = selection.to;
      onMouseMove(x,y,keyFlags);
      return;
   }
}


//------------------------------------------------------------------------------
PaletteShade::PaletteShade()
{
   iOver = -1;
   iSelect = 8;   
   iSelectColor = 0;   
   doShade = true;
   doHaze  = true;
   change  = true;
   pal = NULL;
}


//------------------------------------------------------------------------------
void PaletteShade::render(GFXSurface *surface)
{
   GFXPalette *palette = surface->getPalette();
   change |= (pal != palette);
   if (palette && palette->palette[0].shadeMap && palette->palette[0].hazeMap)
   {
      //if (change)
      {
         int sx, sy, sxa, sya;
         int maxShade = palette->getMaxShade();
         int maxHaze  = palette->getMaxHaze();
         int color, mapcolor;
         RectI rect;
         
         if (doHaze && doShade) 
         {
            //--------------------------------------
            for (int haze=0; haze<=maxHaze; haze++)
            {
               sx=SHW/(maxShade+1);
               sy=SHH/(maxHaze+1); 
               rect(0,0,sx,sy);
               rect += Point2I(XOFSS, YOFSS);
               rect += Point2I(0, haze*sy);
               for (int shade=0; shade<=maxShade; shade++ )
               {
                  mapcolor = palette->mapColor(iSelectColor, maxShade-shade, haze);
                  surface->drawRect2d_f( &rect, mapcolor);

                  //adjust rect position...
                  rect += Point2I(sx,0);
               }
            }
         }
         else if (doHaze) 
         {
            //--------------------------------------
            for (int y=0; y<16; y++)
            {
               color = (iSelect*16)+y;
               sx=SHW/(maxShade+1);
               sy=SHH/16; 
               rect(0,0,sx,SZ);
               rect += Point2I(XOFSS, YOFSS);
               rect += Point2I(0, y*sy);
               for (int haze=0; haze<=maxShade; haze++ )
               {
                  mapcolor = palette->palette[0].hazeMap[(256*haze + color)];
                  surface->drawRect2d_f( &rect, mapcolor);

                  //adjust rect position...
                  rect += Point2I(sx,0);
               }
            }
         }
         else if (doShade) 
         {
            //--------------------------------------
            for (int y=0; y<16; y++)
            {
               color = (iSelect*16)+y;
               sx=SHW/(maxShade+1);
               sy=SHH/16; 
               rect(0,0,sx,SZ);
               rect += Point2I(XOFSS, YOFSS);
               rect += Point2I(0, y*sy);
               for (int shade=0; shade<=maxShade; shade++ )
               {
                  mapcolor = palette->mapColor(color, maxShade-shade, 0);
                  surface->drawRect2d_f( &rect, mapcolor);

                  //adjust rect position...
                  rect += Point2I(sx,0);
               }
            }
         }
         change = false;
      }
   }
   if (iOver != -1)
      iOver = *surface->getAddress(ptOver);
}


void PaletteShade::onOver(int x, int y)
{
   iOver = -1;
   if ( (y >= YOFSS) && y < (YOFSS+SHH))
      if (x > XOFSS && x < (XOFSS+SHW))
      {
         ptOver.set( x, y );
         iOver = 1;
      }
}   

//------------------------------------------------------------------------------
void PaletteShade::onMouseMove(int x, int y, UINT keyFlags)
{
   x, y, keyFlags;
   onOver(x,y);
}

void PaletteShade::onLButtonUp(int x, int y, UINT keyFlags)
{
   x, y, keyFlags;
}

void PaletteShade::onRButtonUp(int x, int y, UINT keyFlags)
{
   x, y, keyFlags;
}

void PaletteShade::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   fDoubleClick, x, y, keyFlags;
}

void PaletteShade::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   fDoubleClick, x, y, keyFlags;
}





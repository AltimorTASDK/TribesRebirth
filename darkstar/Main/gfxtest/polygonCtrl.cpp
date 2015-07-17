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
#include <g_bitmap.h>
#include <r_clip.h>
#include <string.h>
#include <m_box.h>
#include <simGuiTestCtrl.h>
#include <simTagDictionary.h>
#include <inspect.h>

#include "polygonCtrl.h"
#include "main.strings.h"

IMPLEMENT_PERSISTENT_TAG( PolygonCtrl,  FOURCC('s','g','p','c') );

extern "C" int __cdecl grabTick();


void PolygonCtrl::setBitmap(Int32 tag)
{
   freeBitmap();
   if (!manager) return;
   
   ResourceManager *rm = SimResource::get(manager);
   if (rm)
   {
      const char* name = SimTagDictionary::getString(manager, tag);
      setBitmap(name);
      textureName[0] = '\0';
      textureTag     = tag;
   }
}  
 
void PolygonCtrl::setBitmap(const char *name)
{
   freeBitmap();
   if (!manager || !name) return;
   
   if (strlen(name) > Inspect::MAX_STRING_LEN)
   {
      AssertWarn(0, "length of Bitmap name is too long");
      return;
   }

   ResourceManager *rm = SimResource::get(manager);
   if (rm)
   {
      strcpy(textureName, name);
      hTexture = rm->load(textureName, true);
      if ( (hTexture->getStride() > 256) || (hTexture->getHeight() > 256) )
      {
         AssertFatal(0, "Error, that size not allowed!");
      }
   }
}   

void PolygonCtrl::freeBitmap()
{
   if ((Bool)hTexture) hTexture.unlock();

   textureTag = 0;
}

void PolygonCtrl::setFont(Int32 tag)
{
   freeFont();
   if (!manager) return;
   
   ResourceManager *rm = SimResource::get(manager);
   if (rm)
   {
      const char* name = SimTagDictionary::getString(manager, tag);
      setFont(name);
      fontName[0] = '\0';
      fontTag  = tag;
   }
}  
 
void PolygonCtrl::setFont(const char* name)
{
   freeFont();
   if (!manager || !name) return;
   
   if (strlen(name) > Inspect::MAX_STRING_LEN)
   {
      AssertWarn(0, "length of font name is too long");
      return;
   }

   ResourceManager *rm = SimResource::get(manager);
   if (rm)
   {
      strcpy(fontName, name);
      hFont = rm->load(fontName, true);
   }
}   

void PolygonCtrl::freeFont()
{
   if ((Bool)hFont) hFont.unlock();

   fontTag = 0;
}

void PolygonCtrl::setTransparent(bool tf)
{
   transparent = tf;
}   

void COREAPI PolygonCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   static int count = 0;
   static int color = 0;
   
   count = (count + 1) % 10;
   if (count == 0) color = (color + 1) % 256;

   RectI r(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);

   Point3F points[4];
   points[0].set(offset.x,   offset.y,   0);
   points[1].set(r.lowerR.x, offset.y,   0);
   points[2].set(r.lowerR.x, r.lowerR.y, 0);
   points[3].set(offset.x,   r.lowerR.y, 0);

   sfc->draw3DBegin();
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setFillColor(color);
   sfc->setZTest(GFX_ZWRITE);
   sfc->addVertex(&points[0]);
   sfc->addVertex(&points[1]);
   sfc->addVertex(&points[2]);
   sfc->addVertex(&points[3]);
   sfc->emitPoly();
   sfc->setZTest(GFX_ZTEST);
   drawPoly(sfc, r);
   sfc->draw3DEnd();
}

void PolygonCtrl::drawPoly(GFXSurface* io_pSfc, const RectI& in_bounds)
{
   RectI clipRegion  = *(io_pSfc->getClipRect());
   RectI clippedRect = in_bounds;
   rectClip(&clippedRect, &clipRegion);

   float insetX = float(clippedRect.len_x()) * 0.1f;
   float insetY = float(clippedRect.len_y()) * 0.1f;

   float lx = float(clippedRect.upperL.x) + insetX;
   float rx = float(clippedRect.lowerR.x) - insetX;
   float ty = float(clippedRect.upperL.y) + insetY;
   float by = float(clippedRect.lowerR.y) - insetY;
   
   Point3F       sVerts[4];
   Point2F       tVerts[4];
   GFXColorInfoF cInfo[4];
   
   ColorF white(1.0f, 1.0f, 1.0f);
   ColorF black(0.0f, 0.0f, 0.0f);
   ColorF grey(0.5, 0.5, 0.5);
   
   cInfo[0].haze = 0.0;
   cInfo[1].haze = 0.5;
   cInfo[2].haze = 1.0;
   cInfo[3].haze = 0.5;
   cInfo[0].color = white;
   cInfo[1].color = black;
   cInfo[2].color = grey;
   cInfo[3].color = white;
   
   io_pSfc->setTexturePerspective(perspectiveCorrect);
   io_pSfc->setTransparency(transparent);
//   io_pSfc->setAlphaSource(GFX_ALPHA_NONE);
   io_pSfc->setAlphaSource(GFX_ALPHA_TEXTURE);

   if (shade == true) {
      if (interpShadeHaze == true) {
         io_pSfc->setShadeSource(GFX_SHADE_VERTEX);
      } else {
         io_pSfc->setShadeSource(GFX_SHADE_CONSTANT);
         io_pSfc->setConstantShade(0.25);
      }
   } else {
      io_pSfc->setShadeSource(GFX_SHADE_NONE);
   }
   
   if (haze == true) {
      if (interpShadeHaze == true) {
         AssertFatal(shade == false,
                     "don't have shade/hazeinterpolating together...");
         io_pSfc->setHazeSource(GFX_HAZE_VERTEX);
      } else {
         io_pSfc->setHazeSource(GFX_HAZE_CONSTANT);
         io_pSfc->setConstantHaze(0.5);
      }
   } else {
      io_pSfc->setHazeSource(GFX_HAZE_NONE);
   }

   if (texture == true && (bool)hTexture != false) {
      io_pSfc->setFillMode(GFX_FILL_TEXTURE);
      io_pSfc->setTextureMap(hTexture);
   } else {
      io_pSfc->setFillMode(GFX_FILL_CONSTANT);
      io_pSfc->setFillColor(249);
   }
   
   if (isTriangle) {
      sVerts[0].x = lx;
      sVerts[0].y = ty;
      sVerts[0].z = 1.0f/1.0f;
      sVerts[1].x = rx;
      sVerts[1].y = ty;
      sVerts[1].z = 1.0f/2.0f;
      sVerts[2].x = lx;
      sVerts[2].y = by;
      sVerts[2].z = 1.0f/1.0f;
   
      tVerts[0].x = 0.0f;
      tVerts[0].y = 0.0f;
      tVerts[1].x = 1.0f;
      tVerts[1].y = 0.0f;
      tVerts[2].x = 0.0f;
      tVerts[2].y = 1.0f;
   } else {
      sVerts[0].x = lx;
      sVerts[0].y = ty;
      sVerts[0].z = 1.0f/1.0f;
      sVerts[1].x = rx;
      sVerts[1].y = ty;
      sVerts[1].z = 1.0f/2.0f;
      sVerts[2].x = rx;
      sVerts[2].y = by;
      sVerts[2].z = 1.0f/2.0f;
      sVerts[3].x = lx;
      sVerts[3].y = by;
      sVerts[3].z = 1.0f/1.0f;

      tVerts[0].x = 0.0f;
      tVerts[0].y = 0.0f;
      tVerts[1].x = 1.0f;
      tVerts[1].y = 0.0f;
      tVerts[2].x = 1.0f;
      tVerts[2].y = 1.0f;
      tVerts[3].x = 0.0f;
      tVerts[3].y = 1.0f;
   }
   
   if (perspectiveCorrect) {
      for (int i = 0; i < 4; i++) {
         tVerts[i].x *= sVerts[i].z;
         tVerts[i].y *= sVerts[i].z;
      }
   }
   
   io_pSfc->addVertex(&sVerts[0], &tVerts[0], &cInfo[0], DWORD(0));
   io_pSfc->addVertex(&sVerts[1], &tVerts[1], &cInfo[1], DWORD(0));
   io_pSfc->addVertex(&sVerts[2], &tVerts[2], &cInfo[2], DWORD(0));
   if (isTriangle == false)
      io_pSfc->addVertex(&sVerts[3], &tVerts[3], &cInfo[3], DWORD(0));
   int tickCount = grabTick();
   io_pSfc->emitPoly();
   tickCount = grabTick() - tickCount;
   
   if ((Bool)hFont && showCycles) {
      char countBuffer[256];
      Point2I p = in_bounds.upperL;
      sprintf(countBuffer, "%d", tickCount);
      io_pSfc->drawText_p(hFont, &p, countBuffer);
   }
}

bool COREAPI PolygonCtrl::onSimInspectQuery(SimInspectQuery *query)
{
   Parent::onSimInspectQuery(query);
   Inspect *insp = query->inspector;

   insp->add(IDDAT_PC_FONTTAG,     true, "IDFNT_*", (Int32)fontTag);
   insp->add(IDDAT_PC_TEXTURETAG,  true, "IDBMP_*", (Int32)textureTag);
   insp->add(IDDAT_PC_SHOWCYCLES,  showCycles);
   insp->add(IDDAT_PC_TRANSPARENT, transparent);
   insp->add(IDDAT_PC_TEXTURE,     texture);
   insp->add(IDDAT_PC_PERSPECTIVE, perspectiveCorrect);
   insp->add(IDDAT_PC_SHADE,       shade);
   insp->add(IDDAT_PC_HAZE,        haze);
   insp->add(IDDAT_PC_INTERP,      interpShadeHaze);
   insp->add(IDDAT_PC_TRIANGLE,    isTriangle);

   return true;   
}   

bool COREAPI PolygonCtrl::onSimAddNotifyEvent(const SimAddNotifyEvent *event)
{
   if (textureTag)
      setBitmap(textureTag);
   if (fontTag)
      setFont(fontTag);

   return Parent::onSimAddNotifyEvent(event);
}

bool COREAPI PolygonCtrl::onSimRemoveNotifyEvent(const SimRemoveNotifyEvent *event)
{
   freeBitmap();
   freeFont();
   return Parent::onSimRemoveNotifyEvent(event);
}   
   
bool COREAPI PolygonCtrl::onSimInspectEvent(const SimInspectEvent *event)
{
   Parent::onSimInspectEvent(event);
   Inspect *insp = event->inspector;
   Int32 bmtag, fonttag;
   
   insp->get(IDDAT_PC_FONTTAG, NULL, NULL, fonttag);
   insp->get(IDDAT_PC_TEXTURETAG, NULL, NULL, bmtag);
   insp->get(IDDAT_PC_SHOWCYCLES,  showCycles);
   insp->get(IDDAT_PC_TRANSPARENT, transparent);
   insp->get(IDDAT_PC_TEXTURE,     texture);
   insp->get(IDDAT_PC_PERSPECTIVE, perspectiveCorrect);
   insp->get(IDDAT_PC_SHADE,       shade);
   insp->get(IDDAT_PC_HAZE,        haze);
   insp->get(IDDAT_PC_INTERP,      interpShadeHaze);
   insp->get(IDDAT_PC_TRIANGLE,    isTriangle);

   if (bmtag)
      setBitmap(bmtag);
   if (fonttag)
      setFont(fonttag);

   return true;
}   

Persistent::Base::Error COREAPI PolygonCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(textureTag);
   sio.write(fontTag);

   sio.write(showCycles);

   sio.write(transparent);
   sio.write(texture);
   sio.write(perspectiveCorrect);
   sio.write(shade);
   sio.write(haze);
   sio.write(interpShadeHaze);
   sio.write(isTriangle);

   return Parent::write(sio, version, user);
}

Persistent::Base::Error COREAPI PolygonCtrl::read( StreamIO &sio, int version, int user)
{
   sio.read(&textureTag);
   sio.read(&fontTag);
   
   sio.read(&showCycles);

   sio.read(&transparent);
   sio.read(&texture);
   sio.read(&perspectiveCorrect);
   sio.read(&shade);
   sio.read(&haze);
   sio.read(&interpShadeHaze);
   sio.read(&isTriangle);

   return Parent::read(sio, version, user);
}

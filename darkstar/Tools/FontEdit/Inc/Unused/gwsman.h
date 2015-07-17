//----------------------------------------------------------------------------

//   $Workfile:   gwtscan.cpp  $
//   $Version$
//   $Revision:   1.1  $
//   $Date:   28 Sep 1995 14:00:02  $

// Initial Author: Mark Frohnmayer 

//----------------------------------------------------------------------------

#ifndef _GWSMAN_H_
#define _GWSMAN_H_

#include <core.h>
#include "g_ddraw.h"
#include "g_cds.h"
#include "g_sub.h"
#include "g_mem.h"


extern FunctionTable software_eclip_table;
extern FunctionTable software_rclip_table;
extern FunctionTable ddraw_eclip_table;
extern FunctionTable ddraw_rclip_table;

   
enum GFX_SM_MODE
{
   GFX_SM_MODE_DIRECT,
   GFX_SM_MODE_WINDOW,
   GFX_SM_MODE_NONE
};

#define GFX_SM_MAX_PAGES     4
#define GFX_SM_MAX_RES_LIST  30


struct Resolution
{
   Point2I size;
   int     bpp;   
};   


class GWSurfaceManager
{
private:
   
   union
   {
      GFXSurface    *generic[GFX_SM_MAX_PAGES];    
      GFXMemSurface *mem[GFX_SM_MAX_PAGES];        //Memory Surfaces
      GFXCDSSurface *dib[GFX_SM_MAX_PAGES];        //Create DIB_Section Surfaces
      GFXDDSurface  *ddraw[GFX_SM_MAX_PAGES];      //Direct Draw surfaces
   }surface;

   GFXSurface  *front;
   GFXSurface  *buff;
   GFXSurface  *back;
   GFXEdgeList *pEdgeList;    
   GFXZBuffer  *pZBuffer;
   GFXPalette  *pPalette;  
   DWORD       flags;
   GFX_SM_MODE mode;
   HWND        hWndApp, hWndSurface;

   static Bool        resolutionListInitialized;
   static Resolution  resList[GFX_SM_MAX_RES_LIST];
   static int         resCount;
   Point2I     resDD;
   Point2I     resCDS;
   Resolution  res;
   static int  totalVideoMemory;

   void pushState();
   void popState();

   Int32 pageCount;
   Int32 buffCount;
   bool  hasDirectDraw;

   static HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC pddsd, LPVOID lpContext);
   static int _USERENTRY compareResolution(const void *_a, const void *_b);
   static int PT_COMP(const Point2I &a, const Point2I &b);
   const Point2I& PT_MIN(const Point2I &a, const Point2I &b);
   const Point2I& PT_MAX(const Point2I &a, const Point2I &b);

   bool setupDDSurfaces(const Point2I &newRes, int bpp);
   bool setupCDSSurfaces(const Point2I &newRes, int bpp);
   bool closestResolution(Point2I *pt, GFX_SM_MODE mode);

public:
   GWSurfaceManager();
   ~GWSurfaceManager();

   void init(HWND hWndApp, HWND hWndSurface, Int32 numBackPages=2,BOOL needBuffPage=TRUE);
   bool setMode(GFX_SM_MODE newMode);
   bool setResolution(const Point2I &size, int depth);
   const Resolution* enumResolution(bool reset);
   const Resolution* getResolution(int i);   

   GFX_SM_MODE getMode();
   GFXSurface *getBackSurface(Int32 backSurfaceNum = 0);
   GFXSurface *getFrontSurface();
   GFXSurface *getBuffSurface();

   void restore();
   HWND getAppHandle();
   HWND getSurfaceHandle();

   Point2I& getSize(Point2I *size);
};

inline void GWSurfaceManager::restore()
{
   if (mode == GFX_SM_MODE_DIRECT)
      surface.ddraw[0]->restore();
}

inline GFXSurface *GWSurfaceManager::getBuffSurface()
{
   AssertFatal(mode != GFX_SM_MODE_NONE,  "GWSurfaceManager::getBuffSurface: No mode set!");
   return buff;
}

inline GFXSurface *GWSurfaceManager::getFrontSurface()
{
   AssertFatal(mode != GFX_SM_MODE_NONE, "GWSurfaceManager::getFrontSurface: No mode set!");
   return front;
}

inline GFXSurface *GWSurfaceManager::getBackSurface(Int32 backSurfaceNum)
{
   AssertFatal(mode != GFX_SM_MODE_NONE, "GWSurfaceManager::getBackSurface: No mode set!");
   return back;
}

inline GFX_SM_MODE GWSurfaceManager::getMode()
{
   return mode;
}

inline HWND GWSurfaceManager::getAppHandle()
{
   return hWndApp;
}

inline HWND GWSurfaceManager::getSurfaceHandle()
{
   return hWndSurface;
}

inline Point2I& GWSurfaceManager::getSize(Point2I *size)
{
   size->set(front->getWidth(), front->getHeight());
   return *size;
}

inline const Point2I& GWSurfaceManager::PT_MIN(const Point2I &a, const Point2I &b)
{
   return(PT_COMP(a,b) <= 0 ? a : b);
}

inline const Point2I& GWSurfaceManager::PT_MAX(const Point2I &a, const Point2I &b)
{
   return(PT_COMP(a,b) <= 0 ? b : a);
}

inline int GWSurfaceManager::PT_COMP(const Point2I &a, const Point2I &b)
{
   //compare based on volume of the surface
   return (a.x*a.y)-(b.x*b.y);
}

#endif

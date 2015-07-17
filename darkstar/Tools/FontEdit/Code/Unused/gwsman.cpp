//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//±   
//± Description 
//±   
//± $Workfile$
//± $Revision$
//± $Author  $
//± $Modtime $
//±
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±


#include <core.h>
#include "gwsman.h"
#include "gwFn_table.h"

static RGBFormat rgb = { 5,11, 6,5, 5,0  };
int        GWSurfaceManager::totalVideoMemory          = 0;
Bool       GWSurfaceManager::resolutionListInitialized = false;
Resolution GWSurfaceManager::resList[GFX_SM_MAX_RES_LIST];
int        GWSurfaceManager::resCount;


//----------------------------------------------------------------------------


GWSurfaceManager::GWSurfaceManager()
{
   for (int i=0; i<GFX_SM_MAX_PAGES; i++)
      surface.generic[i] = NULL;
   pageCount= 0;                                          
   buffCount= 0;                                          
   mode     = GFX_SM_MODE_NONE;                                          
   front    = NULL;
   buff     = NULL;
   back     = NULL;
   resCount = 0;
   res.bpp  = 0;
   hWndApp  = NULL;
   hWndSurface = NULL;
   res.size.set(0,0);
   resDD.set(0,0);
   resCDS.set(0,0);
   hasDirectDraw  = false;
}

GWSurfaceManager::~GWSurfaceManager()
{
   setMode(GFX_SM_MODE_NONE);
}


//----------------------------------------------------------------------------
HRESULT WINAPI GWSurfaceManager::EnumModesCallback(LPDDSURFACEDESC pddsd, LPVOID lpContext)
{
   //Must be Read/Write and not Mode-X
   if (pddsd->ddsCaps.dwCaps & (DDSCAPS_WRITEONLY|DDSCAPS_MODEX))
      return (DDENUMRET_OK);

   //just RGB surfaces with 8 or 16 bits per pixel for now
   if ((pddsd->ddpfPixelFormat.dwFlags & DDPF_RGB) && 
       (pddsd->ddpfPixelFormat.dwRGBBitCount == 8 ||
        pddsd->ddpfPixelFormat.dwRGBBitCount == 16 ) )
   {
      Resolution *list = *((Resolution**)lpContext);
      list->size.x = pddsd->dwWidth;      
      list->size.y = pddsd->dwHeight;      
      list->bpp    = pddsd->ddpfPixelFormat.dwRGBBitCount;
      (*((Resolution**)lpContext))++;
   }
   return (DDENUMRET_OK);
}


const Resolution* GWSurfaceManager::enumResolution(bool reset)
{
   static int ndx = 0;
   if (reset) ndx = 0;
   if (ndx == resCount) return (NULL);
   else return (&resList[ndx++]);
}



//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//± NAME 
//±   const Resolution* GWSurfaceManager::getResolution(int i)
//±   
//± DESCRIPTION 
//±   Get the next largest or smallest resolution
//±   
//± ARGUMENTS 
//±   i  =  if positive return the next largest resolution
//±         if negative resturn the next smallest resolution
//±   
//± RETURNS 
//±   the requested resolution
//±   or the current resolution if no mode set
//±   or the current resolution if request cannot be fulfilled
//±   
//± NOTES 
//±   
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
const Resolution* GWSurfaceManager::getResolution(int i)
{
   if (i==0) return (&res);
   switch (mode)
   {
      case GFX_SM_MODE_WINDOW:
      case GFX_SM_MODE_NONE:
         return (&res);
         
      case GFX_SM_MODE_DIRECT:
         for (int j=0; j<resCount; j++)
         {
            if (resList[j].size == res.size && resList[j].bpp == res.bpp)
            {
               if (i>0)
                  for (int k=j+1; k<resCount; k++)
                  {
                     if (resList[k].bpp == res.bpp)
                        return (&resList[k]);
                  }
               else
                  for (int k=j-1; k>=0; k--)
                  {
                     if (resList[k].bpp == res.bpp)
                        return (&resList[k]);
                  }
               return (&res);
            }
         }
   }   
   return (&res);
}


int _USERENTRY GWSurfaceManager::compareResolution(const void *_a, const void *_b)
{
   //Sort accending by surface volume (ignore bpp)
   return PT_COMP( ((const Resolution*)_a)->size, ((const Resolution*)_b)->size );
}


bool GWSurfaceManager::closestResolution(Point2I *pt, GFX_SM_MODE mode)
{
   Int32  match = -1;
   UInt32 delta = UINT32_MAX;
   switch (mode)
   {
      case GFX_SM_MODE_WINDOW:
         {
         }
         break;

      case GFX_SM_MODE_DIRECT:
         {
            //clip to min/max res
            Int32  ptMem = pt->x * pt->y;
            for (int i = 0; i < resCount; i++) 
            {
               if (res.bpp == resList[i].bpp )
               {
                  int resMem = resList[i].size.x * resList[i].size.y; 
                  UInt32 dx = abs(ptMem - resMem);
                  if (dx <= delta)
                  {
                     delta = dx;
                     match = i;
                  }
               }
            }
            if (match != -1)
               *pt = resList[match].size;   
            else
               return(false);
         }
         break;

      case GFX_SM_MODE_NONE:
      default:
         return(false);
   }
   return(true);
}

void GWSurfaceManager::init(HWND _hWndApp, HWND _hWndSurface, Int32 numBackPages, BOOL needBuffPage)
{
   hWndApp     = _hWndApp;
   hWndSurface = _hWndSurface;
   pageCount = numBackPages;
   buffCount = needBuffPage;

   //Enumerate available DirectDraw Modes
   hasDirectDraw = GFXDDSurface::init(hWndApp);       // initialize DirectDraw Interface
   if (hasDirectDraw)
   {
      resolutionListInitialized = true;
      totalVideoMemory = GFXDDSurface::GetDriverCaps()->dwVidMemTotal;
      IDirectDraw *pDD = GFXDDSurface::GetDD();       // get DirectDraw Interface 
      Resolution *list = resList;      
      if (pDD)
      {
         DDCAPS ddCaps, emddCaps;
         pDD->GetCaps( &ddCaps, &emddCaps );
         if (ddCaps.dwCaps & DDCAPS_BLTCOLORFILL )
         {
            ddraw_rclip_table.fnClear        = GFXDDClearScreen;
            ddraw_rclip_table.fnDrawRect2d_f = GFXDDDrawRect2d_f;
            ddraw_eclip_table.fnClear        = GFXDDClearScreen;
         }

         if (!resolutionListInitialized)
		 {
            pDD->EnumDisplayModes(0, NULL, (void*)&list, EnumModesCallback);
            resCount = (list - resList);
		 }
         GFXDDSurface::free();

         //sort accending by surface area (ignore bpp)
         if(resCount) qsort(resList, resCount, sizeof(Resolution), compareResolution);
      }
   }
}


//----------------------------------------------------------------------------

//----------------------------------------------------------------------------




void GWSurfaceManager::pushState()
{
   if (mode != GFX_SM_MODE_NONE)
   {
      pPalette    =  back->getPalette();
      pZBuffer    =  back->getZBuffer();
      pEdgeList   =  back->getEdgeList();
      flags       =  back->getFlags();
   }
   else
   {
      pPalette    = NULL;
      pZBuffer    = NULL;
      pEdgeList   = NULL;
      flags       = UINT32_MAX;
   }
}


void GWSurfaceManager::popState()
{
   if (mode == GFX_SM_MODE_NONE) return;
   if (pPalette)
   {
      back->setPalette(pPalette);
      if (buff) buff->setPalette(pPalette);
   }
   if (pZBuffer)     back->setZBuffer(pZBuffer);
   if (pEdgeList)    back->setEdgeList(pEdgeList);
   if( flags != (DWORD)-1)  back->setFlags(flags);
}


bool GWSurfaceManager::setMode(GFX_SM_MODE newMode)
{
   Point2I newRes;
   GFX_SM_MODE oldMode = mode;

   AssertFatal(res.size.x, "GFXSurfaceManager::setMode: must use setResolution() first.");
   if (newMode == mode) return (true);

   pushState();
   mode = newMode;
   switch (mode)
   {
      //ÄÄÄÄÄ Dib-Section Surface
      case GFX_SM_MODE_WINDOW:
         if (!resCDS.x) 
            resCDS = res.size;
         if(closestResolution(&resCDS, mode))
            if (!setupCDSSurfaces(resCDS, res.bpp) )
               return (false);
         break;

      //ÄÄÄÄÄ Direct Draw Surface
      case GFX_SM_MODE_DIRECT:
         if (!resDD.x)
            resDD = res.size;
         if ( closestResolution(&resDD, mode) && GFXDDSurface::init(hWndApp))  //get closest resolution
            if ( !setupDDSurfaces(resDD, res.bpp) )         //try and switch
            {                          
               GFXDDSurface::restoreDisplayMode();
               GFXDDSurface::free();
               return (false);
            }
         break;
   }

   getSize(&res.size);
   popState();
   switch (oldMode) 
   {
      case GFX_SM_MODE_WINDOW:
         break;
      case GFX_SM_MODE_DIRECT:
         GFXDDSurface::restoreDisplayMode();
         GFXDDSurface::free();
         break;
      default:
         break;
   }

   return true;
}


bool GWSurfaceManager::setResolution(const Point2I &size, int depth)
{
   Point2I newRes;

   pushState();
   switch (mode)
   {
      //ÄÄÄÄÄ Dib-Section Surface
      case GFX_SM_MODE_WINDOW:
         newRes = size;
         if( closestResolution(&newRes, mode) )
         {
            if (resCDS == newRes) return (true);
            resCDS = newRes;
            if (!setupCDSSurfaces(resCDS, depth) )
               return (false);
         }
         break;

      //ÄÄÄÄÄ Direct Draw Surface
      case GFX_SM_MODE_DIRECT:
         newRes = size;
         if ( closestResolution(&newRes, mode) )            //get closest resolution
         {
            if (resDD == size) return (true);
            if ( !setupDDSurfaces(newRes, res.bpp) )        //try and switch
               if ( !setupDDSurfaces(res.size, res.bpp) )   //switch failed try to restore previous resolution
               {                          
                  GFXDDSurface::restoreDisplayMode();
                  GFXDDSurface::free();
                  return (false);
               }
         }
         break;

      // Idle Mode
      case GFX_SM_MODE_NONE:
         res.size = size;
         res.bpp = depth;
         return (true);
   }
   popState();
   getSize(&res.size);
   res.bpp  = depth;
   return (true);
}



bool GWSurfaceManager::setupCDSSurfaces(const Point2I &newRes, int bpp)
{
   GFXSurface *temporary[GFX_SM_MAX_PAGES] = {NULL,};    
   bool success;

   //do some cleanup first if same mode
   if (mode == GFX_SM_MODE_WINDOW)
   {
      for (int i=0; i<GFX_SM_MAX_PAGES; i++)
      {
         delete surface.dib[i];
         surface.dib[i] = 0;
      }
   }

   success = GFXCDSSurface::create(temporary[0], YES, newRes.x, newRes.y, hWndSurface, &Point2I(0,0), bpp, &rgb);
   if (buffCount)
   {
      temporary[1] = GFXMemSurface::create(newRes.x, newRes.y, bpp);
      success &= (temporary[1] != NULL);
   }
   if (success)
   {
      front = back   = temporary[0];
      buff           = temporary[1];
      for (int i=0; i<GFX_SM_MAX_PAGES; i++)
         surface.generic[i] = temporary[i];
      return (true);
   }
   return (false);
}   


bool GWSurfaceManager::setupDDSurfaces(const Point2I &newRes, int bpp)
{
   GFXSurface *temporary[GFX_SM_MAX_PAGES] = {NULL,};    
   bool success = false;

   //do some cleanup first if same mode
   if (mode == GFX_SM_MODE_DIRECT)
   {
      for (int i=0; i<GFX_SM_MAX_PAGES; i++)
      {
         delete surface.ddraw[i];
         surface.ddraw[i] = NULL;
      }
   }

   if ( GFXDDSurface::setDisplayMode(newRes.x, newRes.y, bpp) )
   {
      while (!success && pageCount >= 2)
      {
         success = GFXDDSurface::create(temporary, pageCount);
         if (!success)
         {
            pageCount--;
         }
      }
      if (success && buffCount)
      {
         if (!GFXDDSurface::create(temporary[pageCount], NO, newRes.x, newRes.y))
            temporary[pageCount] = GFXMemSurface::create( newRes.x, newRes.y, res.bpp);
         success &= (temporary[pageCount] != NULL);
      }
      if (success)
      {
         for (int i=0; i<GFX_SM_MAX_PAGES; i++)
            surface.generic[i] = temporary[i];

         front = temporary[0];
         back  = temporary[1];
         buff  = temporary[pageCount];
         resDD = newRes;
         return (true);
      }
   }
   return (false);
}

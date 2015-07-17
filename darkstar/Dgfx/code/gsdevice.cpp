#include <base.h>
#include "gsdevice.h"
#include "g_ddraw.h"
#include "g_cds.h"
#include "g_mem.h"
#include "g_bitmap.h"

//------------------------------------------------------------------------------

static RES_VECTOR gResVector;

bool GFXSoftwareDevice::ddInitialized     = false;
static HWND hAppWind;

void GFXSoftwareDevice::flip(const RectI *updateRect)
{ 
   if ( m_pSurface != NULL )
   {
      if(updateRect && dynamic_cast<GFXCDSSurface *>(m_pSurface))
         m_pSurface->update(updateRect);
      else 
         m_pSurface->easyFlip(); 
   }
};

void
GFXSoftwareDevice::lock(const GFXLockMode in_lockMode)
{
   if (m_pSurface)
      m_pSurface->lock(in_lockMode);
}

void
GFXSoftwareDevice::unlock()
{
   if (m_pSurface)
      m_pSurface->unlock();
}

GFXSurface*
GFXSoftwareDevice::getSurface()
{
   return m_pSurface;
}

HRESULT WINAPI GFXSoftwareDevice::EnumModesCallback(LPDDSURFACEDESC pddsd, LPVOID lpContext)
{
   //Must be Read/Write and not Mode-X
   //just RGB surfaces with 8 or 16 bits per pixel for now
   if ((pddsd->ddpfPixelFormat.dwFlags & DDPF_RGB) && 
       (pddsd->ddpfPixelFormat.dwRGBBitCount == 8 ) )
   {
      RES_VECTOR *vec = (RES_VECTOR *) lpContext;
      Point2I res( pddsd->dwWidth, pddsd->dwHeight);

      if (pddsd->ddsCaps.dwCaps & (DDSCAPS_WRITEONLY))
         return (DDENUMRET_OK);

      if (res.x > 1024 || res.y > 768)
         return DDENUMRET_OK;
      if (res.x < 512 || res.y < 384)
         return DDENUMRET_OK;

      RES_VECTOR::iterator i;
      for(i = vec->begin(); i != vec->end(); i++)
         if(*i == res)
            return DDENUMRET_OK;
         
      vec->push_back( ResSpecifier(res, false) );
   }
   return (DDENUMRET_OK);
}

void GFXSoftwareDevice::startup(HWND hApp)
{
   hAppWind      = hApp;
   ddInitialized = GFXDDSurface::init(hApp);
   
   extern void setupCacheNoise(const float);
   setupCacheNoise(0.5f);
}

void GFXSoftwareDevice::shutdown()
{
   if(ddInitialized)
      GFXDDSurface::free();
   ddInitialized = FALSE;
}

GFXSoftwareDevice::~GFXSoftwareDevice()
{
   if (enabled)
      disable();
}

void GFXSoftwareDevice::getResVector()
{
   if(ddInitialized)
   {
      // check which fullscreen modes are supported.
      IDirectDraw2 *pDD2 = GFXDDSurface::GetDD2();
      if(pDD2)
      {
         resVector.setSize(0);
         pDD2->EnumDisplayModes(0, NULL, (void*)&resVector, EnumModesCallback);

         int beginSize = resVector.size();
         for (int i = 0; i < beginSize; i++) {
            Point2I res = resVector[i].res;
            if ((res.x / 2) >= 320 && (res.y / 2) >= 240) {
               res.x /= 2;
               res.y /= 2;

               // Do a quick seach to see if we have this res in non-virtual...
               Vector<ResSpecifier>::iterator itr;
               for (itr = resVector.begin(); itr != resVector.end(); itr++) {
                  if (itr->res == res)
                     break;
               }
               if (itr == resVector.end())
                  resVector.push_back(ResSpecifier(Point2I(res.x, res.y), true));
            }
         }

         sortResolutions();

      }
   }
}

bool GFXSoftwareDevice::enumerateModes(GFXDeviceDescriptor *dev)
{
   dev->flags     = 0;
   dev->flags    |= GFXDeviceDescriptor::runsWindowed;
   dev->deviceId  = GFX_SOFTWARE_DEVICE;
   dev->name      = "Software";
   dev->minSize.set(minXSize, minYSize);
   dev->maxSize.set(maxXSize, maxYSize);
   dev->resolutionCount = 0;

   if(ddInitialized == true) {
      // check which fullscreen modes are supported.
      IDirectDraw2 *pDD2 = GFXDDSurface::GetDD2();
      if(pDD2)
      {
         gResVector.setSize(0);
         pDD2->EnumDisplayModes(0, NULL, (void*)&gResVector, EnumModesCallback);

         int beginSize = gResVector.size();
         for (int i = 0; i < beginSize; i++) {
            Point2I res = gResVector[i].res;
            if ((res.x / 2) >= 320 && (res.y / 2) >= 240) {
               res.x /= 2;
               res.y /= 2;

               // Do a quick seach to see if we have this res in non-virtual...
               Vector<ResSpecifier>::iterator itr;
               for (itr = gResVector.begin(); itr != gResVector.end(); itr++) {
                  if (itr->res == res)
                     break;
               }
               if (itr == gResVector.end())
                  gResVector.push_back(ResSpecifier(Point2I(res.x, res.y), true));
            }
         }

         if((dev->resolutionCount = gResVector.size()) != 0) 
         {
            dev->flags |= GFXDeviceDescriptor::runsFullscreen;
            qsort(gResVector.address(), gResVector.size(), sizeof(ResSpecifier), compareResolution);
            dev->resolutionList = gResVector.address();
         }
      }
   }
   return(true);
}


GFXSoftwareDevice::GFXSoftwareDevice( HWND hApp, HWND hSurface )
 : GFXDevice( hApp, hSurface )
{
   m_pSurface = NULL;
   isFullscr = FALSE;

   currentSize.set(640,480);
   getResVector();
}   


//------------------------------------------------------------------------------
GFXDevice* GFXSoftwareDevice::create( HWND hApp, HWND hSurface )
{
   GFXDevice *gwsurface = new GFXSoftwareDevice(hApp, hSurface);
   return gwsurface;
}

//------------------------------------------------------------------------------
bool GFXSoftwareDevice::enable(GFXPalette *palette, DWORD flags)
{
   GetWindowRect( clientWind, &windowRect);
   enabled = true;
   enabled = setResolution( currentSize, palette, flags, isFullscr );
   return ( enabled );
}


//------------------------------------------------------------------------------
void GFXSoftwareDevice::disable()
{
   delete m_pSurface;
   m_pSurface = NULL;
      
   enabled        = false;

   if (restoreWin)
      restoreWindow();
}

bool GFXSoftwareDevice::modeSwitch(bool fullscreen)
{
   isFullscr = fullscreen;
   return true;
}

bool GFXSoftwareDevice::setResolution(Point2I &res)
{
   return (setResolution( res, 
           m_pSurface ? m_pSurface->getPalette() : NULL, 
           m_pSurface ? m_pSurface->getFlags() : GFX_DMF_RCLIP, isFullscr));   
}


//------------------------------------------------------------------------------
const ResSpecifier&
GFXSoftwareDevice::getResSpec(const Point2I& in_rRes)
{
   Vector<ResSpecifier>::iterator itr;
   for (itr = resVector.begin(); itr != resVector.end(); itr++) {
      if (itr->res.x == in_rRes.x && itr->res.y == in_rRes.y)
         return *itr;
   }
   AssertFatal(false, "Should never have passed in a bogus res.");

   return *(resVector.begin());
}

bool GFXSoftwareDevice::setResolution(Point2I &res, GFXPalette *palette, DWORD flags, bool fullscreen)
{
   float gamma = -1.0f;

   if ( !enabled || (enabled && fullscreen != isFullscr)) {
      currentSize = closestRes(res);
      return true;
   }
   if ( res == currentSize && m_pSurface && isFullscr == fullscreen ) return ( true );

   if (m_pSurface)
      gamma = m_pSurface->getGamma();

   //clean up existing surface
   delete m_pSurface;
   m_pSurface = NULL;

   if(!fullscreen) {
      if(res.x < minXSize)
         res.x = minXSize;
      else if(res.x > maxXSize)
         res.x = maxXSize;

      if(res.y < minYSize)
         res.y = minYSize;
      else if(res.y > maxYSize)
         res.y = maxYSize;

      // Find the resolution in our list that corresponds to the res, and determine if
      //  it is virtual....
      //
      Point2I finalRes = res;
      GFXCDSSurface::create( m_pSurface,       // pointer to surface ptr
                             true,             // is primary surface
                             finalRes.x,            // width
                             finalRes.y,            // height
                             clientWind,       // HWND to surface
                             &Point2I(0,0),    // offset in window
                             8);               // bits per pixel
      if ( !m_pSurface )
      {
         //oops try and restore previous size setting
         res = currentSize;
         GFXCDSSurface::create( m_pSurface,       // pointer to surface ptr
                                true,             // is primary surface
                                finalRes.x,            // width
                                finalRes.y,            // height
                                clientWind,      // HWND to surface
                                &Point2I(0,0),    // offset in window
                                8);               // bits per pixel
      }

      if ( m_pSurface )
      {
         currentSize = res;
         if ( palette ) m_pSurface->setPalette(palette);
         m_pSurface->setFlags(flags);
         
         if (gamma > 0.0f)
            m_pSurface->setGamma(gamma);
      }
      isFullscr = FALSE;
      return (m_pSurface != NULL);
   }
   else {
      bool success = FALSE;

      res = closestRes( res );

      if ( res == currentSize && m_pSurface && isFullscr == fullscreen )
         return ( true );
   
      Point2I finalRes = res;
      const ResSpecifier& resSpec = getResSpec(res);
      if (resSpec.isVirtual == true)
         finalRes *= 2;

      success = GFXDDSurface::create(m_pSurface, finalRes.x, finalRes.y);
      
      if ( success )
      {
         if (palette) 
         {
            m_pSurface->setPalette(palette);
            m_pSurface->setFlags( flags );
            if (gamma > 0.0f)
               m_pSurface->setGamma(gamma);
         }
         currentSize = res;
         Point2I winSize(m_pSurface->getWidth(), m_pSurface->getHeight());
         setWindow(&winSize);
      }
      else
         disable();
      return(success);
   }
}

bool
GFXSoftwareDevice::isVirtualFS()
{
   if (isFullscr == false) {
      return false;
   } else {
      const ResSpecifier& resSpec = getResSpec(currentSize);
      return resSpec.isVirtual;
   }
}


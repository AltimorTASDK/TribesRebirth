//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gd3ddev.h"
#include "gdmanag.h"
#include "gd3dsfc.h"

using namespace Direct3D;

bool GFXD3DDevice::d3dInitialized = false;

//------------------------------------------------------------------------------
// NAME 
//    void GFXD3DDevice::startup()
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
GFXD3DDevice::startup(HWND hWnd)
{
   AssertFatal(d3dInitialized == false, "Error, attempted to restart GFXD3DDevice");
   
   d3dInitialized = Direct3D::Surface::init(hWnd);
}


//------------------------------------------------------------------------------
// NAME 
//    void COPEAPI GFXD3DDevice::shutdown()
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void 
GFXD3DDevice::shutdown()
{
   if (d3dInitialized == true) {
      Direct3D::Surface::free();
   }
   
   d3dInitialized = false;
}




GFXD3DDevice::GFXD3DDevice(Int32 in_deviceMinorId, HWND hApp, HWND hSurface )
   : GFXDevice(hApp, hSurface)
{
   AssertFatal(d3dInitialized == true,
               "Error: Attempted to create GFXD3DDevice before calling ::startup()");
   
   surface        = NULL;
   deviceMinorId  = in_deviceMinorId;
   restoreDisplay = false;

   // Get the resolutions...
   //
   Vector<Direct3D::modeDescription> mdVector;
   Direct3D::Surface::getDeviceResolutions(in_deviceMinorId, &mdVector);
   resVector.setSize(mdVector.size());
   for (int i = 0; i < resVector.size(); i++) {
      resVector[i] = mdVector[i].res;
   }
   sortResolutions();
}



GFXD3DDevice::~GFXD3DDevice()
{
   if (enabled == true)
      disable();
}


bool 
GFXD3DDevice::setResolution(Point2I &res, GFXPalette *io_pPalette, DWORD, bool)
{
   float gamma = -1.0f;

   // Modified from gglidedev.cpp & gsdevice.cpp
   //
   if (surface) {
      gamma = surface->getGamma();
      delete surface;
      surface = NULL;
   }
   res = closestRes(res);
   
   restoreDisplay = Direct3D::Surface::setDisplayMode(res.x, res.y, 16, deviceMinorId);
   AssertWarn(restoreDisplay == true, "Failed to set displayMode in GFXD3DDevice::setResolution");
   
   if (restoreDisplay) {
      surface = Direct3D::Surface::create(clientWind, res.x, res.y, deviceMinorId);
      AssertWarn(surface != NULL, "Unable to create surface in GFXD3DDevice::setResolution");
   }
   if (surface) {
      currentSize = res;
      if (io_pPalette)
         surface->setPalette(io_pPalette);

      if (gamma > 0.0f)
         surface->setGamma(gamma);

      return true;
   } else {
      disable();
      return false;
   }
}

GFXDevice* 
GFXD3DDevice::create(Int32 deviceMinorId, HWND hApp, HWND hSurface)
{
   return (new GFXD3DDevice(deviceMinorId, hApp, hSurface));
}


int 
GFXD3DDevice::enumerateModes(GFXDeviceDescriptor *dev)
{
   if (d3dInitialized == false)
      return 0;
      
   int numDevices = Direct3D::Surface::getNumDevices();
   int currDevice = 0;
   for (int i = 0; i < numDevices; i++) {
      if (Direct3D::Surface::describeDevice(i, &dev[currDevice]) == true) {
         currDevice++;
         
         // Sort the reported resolutions...
         qsort(dev->resolutionList, dev->resolutionCount, sizeof(Point2I),
               compareResolution);
         dev->minSize = dev->resolutionList[0];
         dev->maxSize = dev->resolutionList[dev->resolutionCount - 1];
      } else {
         // Error occurred, and device is invalid, do not increment currDevice
      }
   }

   numDevices = currDevice;
   return numDevices;
}


bool 
GFXD3DDevice::enable(GFXPalette *palette, DWORD flags)
{
   enabled = setResolution(currentSize, palette, flags, true);
   return enabled;
}


void 
GFXD3DDevice::disable()
{
   delete surface;
   surface = NULL;
   if (restoreDisplay)
      Direct3D::Surface::restoreDisplayMode(clientWind, deviceMinorId);

   enabled        = false;
   restoreDisplay = false;
}

bool 
GFXD3DDevice::setResolution(Point2I &res)
{
   // Modified from gglidedev.cpp
   //
   return (setResolution(res,
                         (surface != NULL) ? surface->getPalette() : NULL,
                         (surface != NULL) ? surface->getFlags() : GFX_DMF_RCLIP,
                         true));
}


bool 
GFXD3DDevice::modeSwitch(bool in_fullScreen)
{
   // From gglidedev.cpp
   if (in_fullScreen)
      isFullscr = true;
   return in_fullScreen;
}


void 
GFXD3DDevice::flip(const RectI *)
{
   if (surface)
      surface->flip();
}


void 
GFXD3DDevice::lock()
{
   if (surface)
      surface->lock();
}


void 
GFXD3DDevice::unlock()
{
   if (surface)
      surface->unlock();
}


GFXSurface* 
GFXD3DDevice::getSurface()
{
   return surface;
}


void 
GFXD3DDevice::messageSurface(const int   in_argc,
                             const char* in_argv[])
{
   Surface* pSurface = static_cast<Surface*>(surface);

   if (pSurface)
      pSurface->processMessage(in_argc, in_argv);
}


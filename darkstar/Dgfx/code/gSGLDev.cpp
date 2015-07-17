//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "gSGLDev.h"
#include "gSGLSfc.h"

using namespace PowerSGL;

// Singleton sentry...
//
bool SGLDevice::sm_deviceInitialized = false;
bool SGLDevice::sm_deviceCreated     = false;



SGLDevice::SGLDevice(HWND io_hApp, HWND io_hSurface)
 : GFXDevice(io_hApp, io_hSurface),
   m_pSurface(NULL)
{

}



SGLDevice::~SGLDevice()
{
   sm_deviceCreated = false;
}


void 
SGLDevice::startup()
{
   // Detect if the device is present, load any necessary DLLs, etc.  Must fail
   //  gracefully if device is not present.
   //
   sm_deviceInitialized = false;
}


void 
SGLDevice::shutdown()
{
   // Release any resources allocated in startup, and mark device as
   //  uninitialized
   sm_deviceInitialized = false;
}


GFXDevice* 
SGLDevice::create(HWND io_hApp, HWND io_hSurface)
{
   // We want to enforce the condition that there is only one device of a type
   //  at any time...
   //
   if (sm_deviceCreated == true)
      return NULL;
   
   sm_deviceCreated = true;
   return (new SGLDevice(io_hApp, io_hSurface));
}


bool 
SGLDevice::enumerateModes(GFXDeviceDescriptor* /*out_pDevDesc*/)
{
   // Fill in a device descriptor structure if the device is present.  Note: this
   //  function will be called even if the device startup is unsuccessful, so the
   //  device must keep internal state to indicate initialization status.  Return
   //  false in the case the device is not ready or present.
   // GFXDeviceDescriptor (gdmanag.h : 20)
   //
   if (sm_deviceInitialized == false)
      return false;
   
   return false;
}


bool 
SGLDevice::setResolution(Point2I& /*io_rRes*/)
{
   // Set res to io_rRes.(x, y)
   //
   return false;
}


bool 
SGLDevice::enable(GFXPalette* /*io_pPalette*/,
                  DWORD       /*in_flags*/)
{
   // Should create and activate a surface at the current resolution
   //  
   return false;
}


void 
SGLDevice::disable()
{
   // Deactivate and delete the currently active surface, if any...
   //
}


bool 
SGLDevice::modeSwitch(bool /*fullscreen*/)
{
   // Return true if successful, not necessary to handle windowed case...
   //
   return false;
}

//-------------------------------------- Anything below here is probably OK,
//                                        for debugging purposes, it may be
//                                        useful to intercept any surface
//                                        messages the device is interested in
void 
SGLDevice::messageSurface(const int   in_argc,
                          const char* in_argv[])
{
   m_pSurface->processMessage(in_argc, in_argv);
}

void 
SGLDevice::flip(const RectI *)
{
   m_pSurface->flip();
}

void 
SGLDevice::lock(const GFXLockMode in_lockMode)
{
   m_pSurface->lock(in_lockMode);
}

void 
SGLDevice::unlock()
{
   m_pSurface->unlock();
}

GFXSurface* 
SGLDevice::getSurface()
{
   return m_pSurface;
}


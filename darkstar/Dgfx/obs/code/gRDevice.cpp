//------------------------------------------------------------------------------
// Description: 
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------

#include <windows.h>
#include "gdmanag.h"
#include "gRDevice.h"
#include "gRSfc.h"
#include "renditionProxy.h"

using namespace Rendition;

const char* const GFXRenditionDevice::sm_pDeviceName     = "RRedline-Verite";
char*             GFXRenditionDevice::sm_pMicrocodeFile  = ".\\v10003d.uc";
bool              GFXRenditionDevice::sm_started         = false;
HINSTANCE         GFXRenditionDevice::sm_hRedlineDLL     = NULL;
HINSTANCE         GFXRenditionDevice::sm_hVeriteDLL      = NULL;
RenditionProxy*   GFXRenditionDevice::sm_pRenditionProxy = NULL;
Vector<Point2I>   GFXRenditionDevice::sm_rResVector;

void V_CDECL veriteErrorHandler(v_handle  io_vHandle,
                                v_routine io_badRoutine,
                                v_error   io_error,
                                int       io_extendedCode);
void V_CDECL redlineErrorHandler(v_handle   io_vHandle,
                                 vl_routine io_badRoutine,
                                 vl_error   io_error,
                                 int        io_extendedCode);

// Hack to make sure we don't screw up when RRedline sends a WM_SIZE message on switch
//  to fullscreen mode...
//
static bool sg_disablePointRes = false;

void 
GFXRenditionDevice::startup(HWND io_appHWnd)
{
   AssertFatal(io_appHWnd != NULL, "Error, bad hwnd");
   AssertFatal(sm_started == false, "Error, restarted on initialized instance!")

   while (true) {
      // Load the DLLs
      //
      sm_hVeriteDLL  = LoadLibrary("verite.dll");
      sm_hRedlineDLL = LoadLibrary("redline.dll");
      if (sm_hVeriteDLL == NULL || sm_hRedlineDLL == NULL)
         break;
         
      // Create our DLL proxy object
      //
      sm_pRenditionProxy = RenditionProxy::create(sm_hVeriteDLL, sm_hRedlineDLL);
      if (sm_pRenditionProxy == NULL)
         break;

      // Set our error "handlers"
      //
      sm_pRenditionProxy->V_RegisterErrorHandler(veriteErrorHandler);
      sm_pRenditionProxy->VL_RegisterErrorHandler(redlineErrorHandler);
      
      // Test that the verite chip is properly installed...
      //
      v_handle vHandle;
      v_error vError = sm_pRenditionProxy->V_CreateVerite(1, io_appHWnd,
                                                          &vHandle,
                                                          sm_pMicrocodeFile);
      if (vError != V_SUCCESS)
         break;
      sm_pRenditionProxy->V_DestroyVerite(vHandle);
   
      // Success!
      sm_started = true;
      return;
   }
   
   // If we broke out of the above, we are in a failure state, try to recover
   //  nicely..
   //
   if (sm_hVeriteDLL != NULL)  FreeLibrary(sm_hVeriteDLL);
   if (sm_hRedlineDLL != NULL) FreeLibrary(sm_hRedlineDLL);
   sm_hVeriteDLL = sm_hRedlineDLL = NULL;

   delete sm_pRenditionProxy;
   sm_pRenditionProxy = NULL;
}


void 
GFXRenditionDevice::shutdown()
{
   if (sm_started == true) {
      FreeLibrary(sm_hVeriteDLL);
      FreeLibrary(sm_hRedlineDLL);
      sm_hVeriteDLL = sm_hRedlineDLL = NULL;

      delete sm_pRenditionProxy;
      sm_pRenditionProxy = NULL;

      sm_started = false;
   }
}


RenditionProxy* 
GFXRenditionDevice::getRenditionProxy()
{
   if (sm_started == true)
      return sm_pRenditionProxy;

   return NULL;
}


bool 
GFXRenditionDevice::enumerateModes(GFXDeviceDescriptor *out_dev)
{
   if (sm_started == false)
      return false;

   // These are currently the only vidmodes supported by RRedline
   // (RRedline Programming Guide)
   //
   sm_rResVector.setSize(0);
   sm_rResVector.push_back(Point2I(320,200));
   sm_rResVector.push_back(Point2I(320,240));
   sm_rResVector.push_back(Point2I(400,300));
   sm_rResVector.push_back(Point2I(512,384));
   sm_rResVector.push_back(Point2I(640,400));
   sm_rResVector.push_back(Point2I(640,480));

   // DMMTOSTUDY: Run in windowed mode too?
   out_dev->flags = GFXDeviceDescriptor::runsFullscreen;

   out_dev->deviceId = GFX_RENDITION_DEVICE;
   out_dev->name     = sm_pDeviceName;
   out_dev->resolutionCount = sm_rResVector.size();
   out_dev->resolutionList  = sm_rResVector.address();

   return(true);
}





GFXRenditionDevice::GFXRenditionDevice(HWND       io_hApp,
                                       HWND       io_hSurface,
                                       const bool in_windowed)
 : GFXDevice(io_hApp, io_hSurface),
   m_pSurface(NULL),
   m_enabled(false),
   m_windowed(in_windowed),
   m_sfcHWnd(io_hSurface)
{
   AssertFatal(sm_started == true,
               "Attempted to create GFXRenditionDevice before ::startup()");

   isFullscr = true;

   // insert resolutions
   //
   resVector.setSize(sm_rResVector.size());
   for (int i = 0; i < resVector.size(); i++)
      resVector[i] = sm_rResVector[i];
   sortResolutions();
}



GFXRenditionDevice::~GFXRenditionDevice()
{
   if (m_enabled == true)
      disable();
}


bool 
GFXRenditionDevice::setResolution(Point2I&    io_res,
                                  GFXPalette* io_pPalette,
                                  const DWORD /*in_flags*/)
{
   float gamma = -1.0f;
   if (m_pSurface != NULL) {
      gamma = m_pSurface->getGamma();
      
      AssertWarn(0, "SR hack");
      m_pSurface->m_waitForDisplaySwitch = false;
      m_pSurface->clear(0);
      m_pSurface->flip();
      m_pSurface->clear(0);
      m_pSurface->flip();

      AssertWarn(0, avar("SR: Deleteing surface: 0x%p",m_pSurface));
      delete m_pSurface;
      m_pSurface = NULL;
   }
   
   sg_disablePointRes = true;
   AssertWarn(0, avar("SR: Creating surface: (%d x %d)", io_res.x, io_res.y));
   m_pSurface = Surface::create(sm_pRenditionProxy,
                                m_sfcHWnd,
                                io_res.x, io_res.y, 16,
                                m_windowed);
   sg_disablePointRes = false;
   
   if (m_pSurface == NULL) {
      AssertWarn(0, avar("SR: failed surface: 0x%p", m_pSurface));
      return false;
   }
   AssertWarn(0, avar("SR: Created surface: (%d x %d), 0x%p", io_res.x, io_res.y, m_pSurface));
   
   if (io_pPalette != NULL)
      m_pSurface->setPalette(io_pPalette);
   currentSize = io_res;
   
   if (gamma > 0.0f)
      m_pSurface->setGamma(gamma);
   
   // Make sure we start with a clean surface...
   //
   m_pSurface->clear(0);
   m_pSurface->flip();
   m_pSurface->clear(0);
   m_pSurface->flip();
   
   return true;
}


GFXDevice* 
GFXRenditionDevice::create(HWND       io_hApp,
                           HWND       io_hSurface,
                           const bool in_windowed)
{
   return (new GFXRenditionDevice(io_hApp, io_hSurface, in_windowed));
}


bool 
GFXRenditionDevice::enable(GFXPalette* io_pPalette, DWORD io_flags)
{
   AssertWarn(0, "Enabling");
   m_enabled = setResolution(currentSize, io_pPalette, io_flags);
   return m_enabled;
}


void 
GFXRenditionDevice::disable()
{
   sg_disablePointRes = true;

   AssertWarn(0, "Disabling");
   if (m_pSurface) {

      AssertWarn(0, "Disable hack");
      m_pSurface->m_waitForDisplaySwitch = false;
      m_pSurface->clear(0);
      m_pSurface->flip();
      m_pSurface->clear(0);
      m_pSurface->flip();
   }
   
   AssertWarn(0, avar("Disable: Deleting surface: 0x%p", m_pSurface));
   delete m_pSurface;
   m_pSurface = NULL;
   sg_disablePointRes = false;

   m_enabled = false;
}


bool 
GFXRenditionDevice::setResolution(Point2I &io_res)
{
   // Modified from gglidedev.cpp
   //
   if (sg_disablePointRes == false) {
      AssertWarn(0, "SetRes Point");
      return (setResolution(io_res,
                            (m_pSurface != NULL) ? m_pSurface->getPalette() : NULL,
                            (m_pSurface != NULL) ? m_pSurface->getFlags()   : GFX_DMF_RCLIP));
   } else {
      AssertWarn(0, "Prevented SetRes Point");
      return true;
   }
}


bool 
GFXRenditionDevice::modeSwitch(bool io_fullScreen)
{
   if (io_fullScreen == true)
      isFullscr = true;
   return isFullscr;
}


void 
GFXRenditionDevice::flip(const RectI *)
{
   if (m_pSurface)
      m_pSurface->flip();
}


void 
GFXRenditionDevice::lock()
{
   if (m_pSurface)
      m_pSurface->lock();
}


void 
GFXRenditionDevice::unlock()
{
   if (m_pSurface)
      m_pSurface->unlock();
}


GFXSurface* 
GFXRenditionDevice::getSurface()
{
   return m_pSurface;
}


void 
GFXRenditionDevice::messageSurface(const int   in_argc,
                                   const char* in_argv[])
{
   Surface* pSurface = static_cast<Surface*>(m_pSurface);

   if (pSurface)
      pSurface->processMessage(in_argc, in_argv);
}


void V_CDECL
veriteErrorHandler(v_handle  /*io_vHandle*/,
                   v_routine io_badRoutine,
                   v_error   io_error,
                   int       /*io_extendedCode*/)
{
   // Ignore this, it is generated by a cache overflow, which we
   //  will handle ourselves...
   //
   if (io_error == V_OUT_OF_VIDEO_MEMORY)
      return;

   char funcName[256];
   char errorText[256];
   
   RenditionProxy* pProxy = GFXRenditionDevice::getRenditionProxy();
   pProxy->V_GetFunctionName(io_badRoutine, funcName, 255);
   pProxy->V_GetErrorText(io_error, errorText, 255);
   
   AssertFatal(0, avar("Error in Verite.dll::%s : %s", funcName, errorText));
}

void V_CDECL
redlineErrorHandler(v_handle   /*io_vHandle*/,
                    vl_routine io_badRoutine,
                    vl_error   io_error,
                    int        /*io_extendedCode*/)
{
   // Ignore this, it is generated by a cache overflow, which we
   //  will handle ourselves...
   //
   if (io_error == VL_OUT_OF_VIDEO_MEMORY)
      return;

   char funcName[256];
   char errorText[256];
   
   RenditionProxy* pProxy = GFXRenditionDevice::getRenditionProxy();
   pProxy->VL_GetFunctionName(io_badRoutine, funcName, 255);
   pProxy->VL_GetErrorText(io_error, errorText, 255);
   
   AssertFatal(0, avar("Error in Redline.dll::%s : %s", funcName, errorText));
}




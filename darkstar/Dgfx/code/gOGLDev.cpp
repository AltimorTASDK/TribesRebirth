//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <windows.h>

#include "gOGLDev.h"
#include "gOGLSfc.h"

using namespace OpenGL;

// Singleton sentry...
//
bool OGLDevice::sm_deviceInitialized = false;
bool OGLDevice::sm_deviceCreated     = false;

bool g_oglDisabled = false;

RES_VECTOR OGLDevice::sm_resVector;

LONG WINAPI OGLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void initRendererName();

static char g_pRendererNameBuffer[1024];
const char* g_pRendererName = NULL;

OGLDevice::OGLDevice(HWND io_hApp, HWND io_hSurface)
 : GFXDevice(io_hApp, io_hSurface),
   m_pSurface(NULL)
{
   resVector.clear();
   for (int i = 0; i < sm_resVector.size(); i++) {
      resVector.push_back(sm_resVector[i]);
   }
}

OGLDevice::~OGLDevice()
{
   sm_deviceCreated = false;
}

Int32
OGLDevice::getDeviceId()
{
   return GFX_OPENGL_DEVICE;
}

void 
OGLDevice::startup(HWND io_clientHWnd)
{
   // Detect if the device is present, load any necessary DLLs, etc.  Must fail
   //  gracefully if device is not present.
   //
   AssertFatal(sm_deviceInitialized == false,
               "Error, attempted to restart GFXD3DDevice");
   
   sm_deviceInitialized = OpenGL::Surface::init(io_clientHWnd);
}


void 
OGLDevice::shutdown()
{
   // Release any resources allocated in startup, and mark device as
   //  uninitialized
   //
   OpenGL::Surface::shutdown();
   sm_deviceInitialized = false;
}


GFXDevice* 
OGLDevice::create(HWND io_hApp, HWND io_hSurface)
{
   // We want to enforce the condition that there is only one device of a type
   //  at any time...
   //
   if (sm_deviceCreated == true)
      return NULL;
   
   sm_deviceCreated = true;
   return (new OGLDevice(io_hApp, io_hSurface));
}


bool 
OGLDevice::enable(GFXPalette* io_pPalette,
                  DWORD       /*in_flags*/)
{
   enabled = true;
   enabled = setResolution(currentSize, io_pPalette);
   return enabled;
}


void 
OGLDevice::disable()
{
   // Deactivate and delete the currently active surface, if any...
   //
   delete m_pSurface;
   m_pSurface = NULL;
   enabled    = false;
}


bool 
OGLDevice::enumerateModes(GFXDeviceDescriptor* out_pDevDesc)
{
   // Fill in a device descriptor structure if the device is present.  Note: this
   //  function will be called even if the device startup is unsuccessful, so the
   //  device must keep internal state to indicate initialization status.  Return
   //  false in the case the device is not ready or present.
   // GFXDeviceDescriptor (gdmanag.h : 20)
   //
   if (sm_deviceInitialized == false)
      return false;
   
   if (g_pRendererName == NULL) {
      initRendererName();
   }

//   // Disallow switching desktop resolutions...
//	HDC hDC = GetDC( GetDesktopWindow() );
//	int bpp = GetDeviceCaps( hDC, BITSPIXEL );
//   ReleaseDC(GetDesktopWindow(), hDC);
//   if (bpp < 15) {
//      return false;
//   }
   if (g_oglDisabled == true)
      return false;

//   // Disallow software OpenGL
//   if (stricmp(g_pRendererName, "GDI Generic") == 0) {
//      HINSTANCE G200OpenGL = LoadLibrary("MGAICD.DLL");
//      if (G200OpenGL == NULL) {
//         return false;
//      } else {
//         FreeLibrary(G200OpenGL);
//      }
//   }

   refreshResList();
   qsort(sm_resVector.address(), sm_resVector.size(), sizeof(ResSpecifier), compareResolution);

   out_pDevDesc->flags           = GFXDeviceDescriptor::runsFullscreen;
   out_pDevDesc->deviceId        = GFX_OPENGL_DEVICE;
   out_pDevDesc->deviceMinorId   = 0;
   out_pDevDesc->name            = "OpenGL";
   out_pDevDesc->pRendererName   = g_pRendererName;
   out_pDevDesc->resolutionCount = sm_resVector.size();
   out_pDevDesc->resolutionList  = sm_resVector.address();

   return true;
}

void
OGLDevice::refreshResList()
{
   sm_resVector.setSize(0);

   BOOL    test    = TRUE;
   UInt32  modeNum = 0;
   DEVMODE devMode;
   while (test == TRUE) {
      memset(&devMode, 0, sizeof(devMode));
      devMode.dmSize = sizeof(devMode);

      test = EnumDisplaySettings(NULL, modeNum++, &devMode);
      if (devMode.dmPelsWidth  >= 640 && devMode.dmPelsWidth  <= 1600 &&
          devMode.dmPelsHeight >= 480 && devMode.dmPelsHeight <= 1200  &&
          devMode.dmBitsPerPel == 16) {

         bool alreadyThere = false;
         for (int i = 0; i < sm_resVector.size(); i++) {
            if (sm_resVector[i].res.x == (Int32)devMode.dmPelsWidth &&
                sm_resVector[i].res.y == (Int32)devMode.dmPelsHeight) {
               alreadyThere = true;
               break;
            }
         }

         if (alreadyThere == false) {
            ResSpecifier newRes;
            newRes.res.x     = devMode.dmPelsWidth;
            newRes.res.y     = devMode.dmPelsHeight;
            newRes.isVirtual = false;

            sm_resVector.push_back(newRes);
         }
      }
   }
}

bool 
OGLDevice::setResolution(Point2I& io_rRes)
{
   if (enabled == false) {
      currentSize = io_rRes;
      return false;
   }

   float gamma = -1.0f;

   // Set res to io_rRes.(x, y)
   //
   GFXPalette* pPalette = NULL;
   if (m_pSurface != NULL) {
      pPalette = m_pSurface->getPalette();
      gamma = m_pSurface->gamma;
      delete m_pSurface;
      m_pSurface = NULL;
   }

   m_pSurface = Surface::create(clientWind, io_rRes.x, io_rRes.y);
   if (m_pSurface != NULL) {
      if (gamma != -1) {
         m_pSurface->setGamma(gamma);
      }

      if (pPalette != NULL)
         m_pSurface->setPalette(pPalette);
      currentSize = io_rRes;
      return true;
   } else {
      return false;
   }
}


bool 
OGLDevice::setResolution(Point2I&    io_rRes,
                         GFXPalette* io_pPalette)
{
   // Set res to io_rRes.(x, y) w/ palette io_pPalette
   //
   if (setResolution(io_rRes) == false)
      return false;
   else {
      AssertFatal(m_pSurface != NULL, "No surface after successful creation?");
      m_pSurface->setPalette(io_pPalette);
   }

   return true;
}


bool 
OGLDevice::modeSwitch(bool fullscreen)
{
   // Return true if successful, not necessary to handle windowed case...
   //
   isFullscr = true;
   return (fullscreen == true);
}

//-------------------------------------- Anything below here is probably OK,
//                                        for debugging purposes, it may be
//                                        useful to intercept any surface
//                                        messages the device is interested in
void 
OGLDevice::messageSurface(const int   in_argc,
                          const char* in_argv[])
{
   m_pSurface->processMessage(in_argc, in_argv);
}

void 
OGLDevice::flip(const RectI *)
{
   m_pSurface->flip();
}

void 
OGLDevice::lock(const GFXLockMode in_lockMode)
{
   m_pSurface->lock(in_lockMode);
}

void 
OGLDevice::unlock()
{
   m_pSurface->unlock();
}

GFXSurface* 
OGLDevice::getSurface()
{
   return m_pSurface;
}

bool
OGLDevice::isVirtualFS()
{
   // OGL is never virtual...
   return false;
}


HWND
CreateOpenGLWindow(char* title, int x, int y, int width, int height, 
		   BYTE type, DWORD flags)
{
   int         pf;
   HDC         hDC;
   HWND        hWnd;
   WNDCLASS    wc;
   PIXELFORMATDESCRIPTOR pfd;
   static HINSTANCE hInstance = 0;

   /* only register the window class once - use hInstance as a flag. */
   if (!hInstance) {
	   hInstance = GetModuleHandle(NULL);
	   wc.style         = CS_OWNDC;
	   wc.lpfnWndProc   = (WNDPROC)OGLWindowProc;
	   wc.cbClsExtra    = 0;
	   wc.cbWndExtra    = 0;
	   wc.hInstance     = hInstance;
	   wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
	   wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	   wc.hbrBackground = NULL;
	   wc.lpszMenuName  = NULL;
	   wc.lpszClassName = "OpenGL";

	   if (!RegisterClass(&wc)) {
	      return NULL;
	   }
   }

   hWnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
		                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		                 x, y, width, height, NULL, NULL, hInstance, NULL);

   if (hWnd == NULL) {
	   return NULL;
   }

   hDC = GetDC(hWnd);

   /* there is no guarantee that the contents of the stack that become
      the pfd are zeroed, therefore _make sure_ to clear these bits. */
   memset(&pfd, 0, sizeof(pfd));
   pfd.nSize        = sizeof(pfd);
   pfd.nVersion     = 1;
   pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
   pfd.iPixelType   = type;
   pfd.cColorBits   = 16;

   pf = ChoosePixelFormat(hDC, &pfd);
   if (pf == 0) {
      ReleaseDC(hWnd, hDC);
      DestroyWindow(hWnd);
	   return 0;
   } 
 
   if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
      ReleaseDC(hWnd, hDC);
      DestroyWindow(hWnd);
	   return 0;
   } 

   DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

   ReleaseDC(hWnd, hDC);

   return hWnd;
}    

void
initRendererName()
{
   HWND hWnd = CreateOpenGLWindow("wglinfo", 0, 0, 100, 100, PFD_TYPE_RGBA, 0);
   if (hWnd == NULL)
	   return;

   HDC hDC = GetDC(hWnd);
   HGLRC hRC = wglCreateContext(hDC);
   wglMakeCurrent(hDC, hRC);
   ShowWindow(hWnd, SW_HIDE);

   g_pRendererName = (const char *)glGetString(GL_RENDERER);
   if (g_pRendererName != NULL) {
      strcpy(g_pRendererNameBuffer, g_pRendererName);
      g_pRendererName = g_pRendererNameBuffer;
   }

   char bppBuff[32];
	HDC deskHDC = GetDC( GetDesktopWindow() );
	int bpp = GetDeviceCaps( deskHDC, BITSPIXEL );
   ReleaseDC(GetDesktopWindow(), deskHDC);
   wsprintf(bppBuff, " @%d Bits", bpp);
   strcat(g_pRendererNameBuffer, bppBuff);

   wglMakeCurrent(hDC, NULL);
   ReleaseDC(hWnd, hDC);
   wglDeleteContext(hRC);
   DestroyWindow(hWnd);
}

LONG WINAPI
OGLWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    static PAINTSTRUCT ps;

    switch(uMsg) {
    case WM_PAINT:
	BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
	return 0;

    case WM_SIZE:
	PostMessage(hWnd, WM_PAINT, 0, 0);
	return 0;

    case WM_CHAR:
	switch (wParam) {
	case 27:			/* ESC key */
	    PostQuitMessage(0);
	    break;
	}
	return 0;

    case WM_CLOSE:
	PostQuitMessage(0);
	return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam); 
} 


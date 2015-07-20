//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_bitmap.h"
#include "fn_table.h"
#include "gfxMetrics.h"
#include "d_caps.h"

#include "gOGLSfc.h"
#include "gOGLTx.h"
#include <windows.h>
#include <gl/glu.h>

float g_start = 0;
float g_end   = 0;

#ifdef DEBUG
#define OGL_ERROR_CHECK(line) while(checkOGLError(line));
#else
#define OGL_ERROR_CHECK(line) ;
#endif

bool g_oglDisableLightUpdates = false;
bool g_prefOGLOverrideLights  = false;

namespace OpenGL {

Surface* g_surfaceActive = NULL;

namespace {

inline bool
checkOGLError(const int in_lineNo)
{
#ifdef DEBUG
   GLenum error = glGetError();
   AssertWarn(error == GL_NO_ERROR, avar("%s: real line: %d", translateOpenGLError(error), in_lineNo));
   return error != GL_NO_ERROR;
#else
   return false;
#endif   
}

class VidModeChecker
{
   bool m_settingsChanged;

   VidModeChecker() { m_settingsChanged = false; }
  public:
   ~VidModeChecker() {
      if (m_settingsChanged == true)
         resetDisplaySettings();
   }

   void changedDisplaySettings() {
      AssertFatal(m_settingsChanged == false, "Error, settings already changed...");
      m_settingsChanged = true;
   }
   void resetDisplaySettings();


   static VidModeChecker sm_theInstance;
};

VidModeChecker VidModeChecker::sm_theInstance;

void
VidModeChecker::resetDisplaySettings()
{
   if (m_settingsChanged == true) {
      AssertMessage(false, "Resetting display");
      ChangeDisplaySettings(NULL, 0);
   }

   m_settingsChanged = false;
}

} // namespace {}

//-------------------------------------- Static data members...
//
int Surface::sm_maxNumVertices = 100;

//------------------------------------------------------------------------------
//--------------------------------------
// Object proper...
//--------------------------------------
//
Surface* 
Surface::create(HWND        io_clientWnd,
                const Int32 in_width,
                const Int32 in_height)
{
   // Must return a new OpenGL::Surface* of resolution closest to in_w/h.
   //  Should also activate the device.
   //
   AssertFatal(io_clientWnd != NULL, "No client window handle");

   Surface* pRetSurf = new Surface;

#ifndef DEBUG
   BOOL   test    = TRUE;
   bool   found   = false;
   UInt32 modeNum = 0;
   DEVMODE devMode;

   while (test == TRUE) {
      memset(&devMode, 0, sizeof(devMode));
      devMode.dmSize = sizeof(devMode);
      test = EnumDisplaySettings(NULL, modeNum, &devMode);
      if (devMode.dmPelsWidth        == (UInt32)in_width  &&
          devMode.dmPelsHeight       == (UInt32)in_height) {
         found = true;
      }

      modeNum++;
   }

   if (found == false) {
      delete pRetSurf;
      return NULL;
   }

   // Change the window position
   AssertMessage(false, avar("Changing window style (%d, %d)", in_width, in_height));
   GetWindowRect(io_clientWnd, &pRetSurf->m_oldWindowRect);
   LONG style = GetWindowLong(io_clientWnd, GWL_STYLE);
   style &= ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
   SetWindowLong(io_clientWnd, GWL_STYLE, style);

   LONG exStyle = GetWindowLong(io_clientWnd, GWL_EXSTYLE);
   exStyle |= WS_EX_TOPMOST;
   SetWindowLong(io_clientWnd, GWL_EXSTYLE, exStyle);
   BOOL posSuccess = SetWindowPos(io_clientWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
   AssertFatal(posSuccess == TRUE, "Error setting pos");

   // Annnnd, set the new display mode.  Desktop icons?
   //
   memset(&devMode, 0, sizeof(devMode));
   devMode.dmSize = sizeof(devMode);
   devMode.dmPelsWidth  = in_width;
   devMode.dmPelsHeight = in_height;
   devMode.dmFields = DM_PELSWIDTH  |
                      DM_PELSHEIGHT;

#if 1
   AssertMessage(false, avar("Changing display settings: (%d, %d)", devMode.dmPelsWidth, devMode.dmPelsHeight));
   ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
   VidModeChecker::sm_theInstance.changedDisplaySettings();
#endif

   SetForegroundWindow(io_clientWnd);
   posSuccess = SetWindowPos(io_clientWnd, HWND_TOPMOST, 0, 0, in_width, in_height, SWP_FRAMECHANGED);
   AssertFatal(posSuccess == TRUE, "Error setting pos");
   pRetSurf->m_windowStyleChanged = true;
#endif

   pRetSurf->m_hWnd = io_clientWnd;
   pRetSurf->m_hDC = GetDC(pRetSurf->m_hWnd);
   if (pRetSurf->m_hDC == NULL) {
      AssertWarn(0, "Unable to get a DC for the window");
      delete pRetSurf;
      return NULL;
   }

   // Set the Pixel format, first retrieving the old format
   //
   pRetSurf->m_oldPixelFormat = GetPixelFormat(pRetSurf->m_hDC);
   DescribePixelFormat(pRetSurf->m_hDC, pRetSurf->m_oldPixelFormat,
                       sizeof(PIXELFORMATDESCRIPTOR),
                       &pRetSurf->m_oldPixelFormatDescriptor);

   int chosenPixelFormat;
   PIXELFORMATDESCRIPTOR chosenPFD;
   if (pRetSurf->choosePixelFormat(chosenPixelFormat,
                                   chosenPFD,
                                   pRetSurf->m_hWnd,
                                   pRetSurf->m_hDC) == false) {
      AssertWarn(0, "Unable to choose a pixel format");
      delete pRetSurf;
      return NULL;
   }

   BOOL spSuccess = SetPixelFormat(pRetSurf->m_hDC,
                                   chosenPixelFormat,
                                   &chosenPFD);
   if (spSuccess != TRUE) {
      AssertWarn(0, "Unable to set the pixel format");
      delete pRetSurf;
      return NULL;
   }

   // Create the HGLRC
   //
   pRetSurf->m_hGLRC = wglCreateContext(pRetSurf->m_hDC);
   if (pRetSurf->m_hGLRC == NULL) {
      AssertWarn(0, "Unable to create a GL Render context");
      delete pRetSurf;
      return NULL;
   }

   BOOL mcSuccess = wglMakeCurrent(pRetSurf->m_hDC, pRetSurf->m_hGLRC);
   if (mcSuccess != TRUE) {
      AssertWarn(0, "Unable to make the GL Render context current");
      delete pRetSurf;
      return NULL;
   }
   pRetSurf->m_glrcMadeCurrent = true;

   // Retreive the strings associated with this driver, they are useful
   //  for debugging and such...
   //
   const char* pVendor     = (const char*)glGetString(GL_VENDOR);
   const char* pRenderer   = (const char*)glGetString(GL_RENDERER);
   const char* pVersion    = (const char*)glGetString(GL_VERSION);
   const char* pExtensions = (const char*)glGetString(GL_EXTENSIONS);

   if (pVendor != NULL) {
      pRetSurf->m_pVendorString = new char[strlen(pVendor) + 1];
      strcpy(pRetSurf->m_pVendorString, pVendor);
   }
   if (pRenderer != NULL) {
      pRetSurf->m_pRendererString = new char[strlen(pRenderer) + 1];
      strcpy(pRetSurf->m_pRendererString, pRenderer);
   }
   if (pVersion != NULL) {
      pRetSurf->m_pVersionString = new char[strlen(pVersion) + 1];
      strcpy(pRetSurf->m_pVersionString, pVersion);
   }
   if (pExtensions != NULL) {
      pRetSurf->m_pExtensionsString = new char[strlen(pExtensions) + 1];
      strcpy(pRetSurf->m_pExtensionsString, pExtensions);
   }

   // Allocate our vertex arrays...
   //
   pRetSurf->m_pVertexArray    = new DGLVertex4F[sm_maxNumVertices];
   pRetSurf->m_pColorArray     = new DGLColor4F[sm_maxNumVertices];
   pRetSurf->m_pTexCoord0Array = new DGLTexCoord4F[sm_maxNumVertices];
   pRetSurf->m_pHazeStoreArray = new DGLHazeCoordF[sm_maxNumVertices];

   pRetSurf->surfaceWidth  = in_width;
   pRetSurf->surfaceHeight = in_height;

   // Create the texture/handle cache...
   //
   pRetSurf->m_pTextureCache = new TextureCache(pRetSurf);
   pRetSurf->m_pHandleCache  = new HandleCache(2048, 2153);

   // Set up our default state...
   //
   pRetSurf->setFillMode(GFX_FILL_CONSTANT);
   pRetSurf->setFillColor(&ColorF(0, 0, 0));
   pRetSurf->setAlphaSource(GFX_ALPHA_NONE);
   pRetSurf->setShadeSource(GFX_SHADE_NONE);
   pRetSurf->setHazeSource(GFX_HAZE_NONE);
   pRetSurf->setTextureWrap(true);
   pRetSurf->setTransparency(false);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   // We're ready to go...
   //
   return pRetSurf;
}


Surface::Surface()
 : m_hWnd(NULL), m_hDC(NULL), m_hGLRC(NULL),
   m_oldPixelFormat(-1),
   m_pVendorString(NULL),
   m_pRendererString(NULL),
   m_pVersionString(NULL),
   m_pExtensionsString(NULL),
   m_currVertexIndex(0),
   m_pVertexArray(NULL),
   m_pColorArray(NULL),
   m_pTexCoord0Array(NULL),
   m_pHazeStoreArray(NULL),
   m_pTextureCache(NULL),
   m_disableBilinear(false),
   m_glrcMadeCurrent(false),
   m_windowStyleChanged(false),
   m_in3dmode(false),
   m_pHandleCache(NULL)
{
   AssertFatal(g_surfaceActive == NULL, "only 1 opengl surface allowed at a time!");
   g_surfaceActive = this;
   g_oglDisableLightUpdates = true;

   functionTable = &opengl_table;

   m_fillMode    = GFX_FILL_CONSTANT;
   m_shadeSource = GFX_SHADE_NONE;
   m_hazeSource  = GFX_HAZE_NONE;
   m_alphaSource = GFX_ALPHA_NONE;
   m_zTest       = GFX_NO_ZTEST;

   m_nearClipPlane = 0.1f;
   m_farClipPlane  = 10000.0f;

   m_alphaBlend = 0.0f;
   m_alphaColor.set(0.0f, 0.0f, 0.0f);
   m_flareBlend = 0.0f;
   m_flareColor.set(0.0f, 0.0f, 0.0f);
   
   m_currVertexIndex = 0;
}


Surface::~Surface()
{
   AssertFatal(g_surfaceActive == this, "We're not the active surface?");
   g_surfaceActive = NULL;
   g_oglDisableLightUpdates = false;

   // Remove our texture cache...
   //
   delete m_pHandleCache;
   m_pHandleCache = NULL;

   if (m_pTextureCache != NULL)
      m_pTextureCache->flushCache();
   delete m_pTextureCache;
   m_pTextureCache = NULL;


   // Clear out our vertex indices...
   //
   m_currVertexIndex = 0;
   delete [] m_pHazeStoreArray;
   m_pHazeStoreArray = NULL;
   delete [] m_pTexCoord0Array;
   m_pTexCoord0Array = NULL;
   delete [] m_pColorArray;
   m_pColorArray = NULL;
   delete [] m_pVertexArray;
   m_pVertexArray = NULL;

   if (m_hGLRC != NULL) {
      if (m_hDC != NULL) {
         BOOL success = wglMakeCurrent(m_hDC, NULL);
         AssertWarn(success == TRUE, "Couldn't clear the hGLRC?");
      }
      m_glrcMadeCurrent = false;
      BOOL success = wglDeleteContext(m_hGLRC);
      AssertWarn(success == TRUE, "Couldn't delete the hGLRC?");
   }

   // Reset the pixel format...
   //
   if (m_oldPixelFormat != -1) {
      AssertFatal(m_hDC != NULL, "old pixel format with no DC?");

      BOOL success = SetPixelFormat(m_hDC,
                                    m_oldPixelFormat,
                                    &m_oldPixelFormatDescriptor);
      AssertWarn(success == TRUE, "Couldn't set the pixel format to 0?");
   }

   if (m_hDC != NULL && m_hWnd != NULL) {
      ReleaseDC(m_hWnd, m_hDC);
   }

   delete [] m_pVendorString;
   delete [] m_pRendererString;
   delete [] m_pVersionString;
   delete [] m_pExtensionsString;

   if (m_windowStyleChanged == true) {
      AssertMessage(false, "Resetting window style");
      // Reset the window style and position
      //
      LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
      style |= (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME);
      SetWindowLong(m_hWnd, GWL_STYLE, style);

      LONG exStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
      exStyle &= ~(WS_EX_TOPMOST);
      SetWindowLong(m_hWnd, GWL_EXSTYLE, exStyle);

      SetForegroundWindow(m_hWnd);
      SetWindowPos(m_hWnd, HWND_TOP,
                   m_oldWindowRect.left,
                   m_oldWindowRect.top,
                   m_oldWindowRect.right,
                   m_oldWindowRect.bottom,
                   SWP_FRAMECHANGED);
   }

   VidModeChecker::sm_theInstance.resetDisplaySettings();

   m_hGLRC          = NULL;
   m_hDC            = NULL;
   m_hWnd           = NULL;
   m_oldPixelFormat = -1;

   m_pVendorString        =
      m_pRendererString   =
      m_pVersionString    =
      m_pExtensionsString = NULL;
}


void 
Surface::flip()
{
   if (m_flareBlend != 0.0f) {
      m_pTextureCache->enableTexUnits(false);
      m_pTextureCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      m_pTextureCache->enableZBuffering(true, false, GL_ALWAYS);
      m_pTextureCache->setTransparent(false);

      glBegin(GL_POLYGON);
         glColor4f(m_flareColor.red, m_flareColor.green, m_flareColor.blue, m_flareBlend);

         glVertex2f(0, 0);
         glVertex2f(surfaceWidth, 0);
         glVertex2f(surfaceWidth, surfaceHeight);
         glVertex2f(0, surfaceHeight);
      glEnd();
   }
   m_flareBlend = 0.0f;
   if (m_alphaBlend != 0.0f) {
      m_pTextureCache->enableTexUnits(false);
      m_pTextureCache->setBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      m_pTextureCache->enableZBuffering(true, false, GL_ALWAYS);
      m_pTextureCache->setTransparent(false);

      glBegin(GL_POLYGON);
         glColor4f(m_alphaColor.red, m_alphaColor.green, m_alphaColor.blue, m_alphaBlend);

         glVertex2f(0, 0);
         glVertex2f(surfaceWidth, 0);
         glVertex2f(surfaceWidth, surfaceHeight);
         glVertex2f(0, surfaceHeight);
      glEnd();
   }
   m_alphaBlend = 0.0f;

   // Swap buffers
   glFinish();
   BOOL sbSuccess = SwapBuffers(m_hDC);

   if (sbSuccess == FALSE) {
      char buffer[1024];
      buffer[0] = '\0';
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    buffer, 1023, NULL);
      AssertWarn(0, buffer);
   }

   if (m_pTextureCache != NULL) {
      m_pTextureCache->incFrameKey();
      m_pTextureCache->clearState();
   }
}


Bool 
Surface::_setPalette(GFXPalette* lpPalette,
                     Int32       /*in_start*/,
                     Int32       /*in_count*/,
                     Bool        /*in_rsvPal*/)
{
   pPalette = lpPalette;

   m_pTextureCache->setPalette(lpPalette);
   m_pTextureCache->setGamma(gamma);
   setHazeColor(lpPalette->hazeColor);

   return (lpPalette != NULL);
}


void 
Surface::_lock(const GFXLockMode in_lockMode)
{
   AssertFatal(in_lockMode == GFX_NORMALLOCK, "Double lock not supported in OGL mode");

   // Make the GLRC current...
   //
   AssertFatal(m_hDC != NULL && m_hGLRC != NULL, "No DC or no GLRC");

   if (m_glrcMadeCurrent == false) {
      BOOL mcSuccess = wglMakeCurrent(m_hDC, m_hGLRC);
      AssertFatal(mcSuccess == TRUE, avar("Unable to make context current: %d", m_hGLRC));
      m_glrcMadeCurrent = true;
   }

   // This transforms the screen coords that GFX is passed into GL's normalized
   //  screen device coords...
   //
   glViewport(0, 0, surfaceWidth, surfaceHeight);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glTranslatef(-1, -1, 0);
   glScalef(2.0f / float(surfaceWidth),
            2.0f / float(surfaceHeight),
            1);
   glTranslatef(0, (float(surfaceHeight) / 2.0f), 0);
   glScalef(1, -1, 1);
   glTranslatef(0, -(float(surfaceHeight) / 2.0f), 0);

   GFXMetrics.reset();
   GFXMetrics.textureSpaceUsed       = 0;
   GFXMetrics.textureBytesDownloaded = 0;

   m_pTextureCache->enableZBuffering(true, false, GL_ALWAYS);
   glFogi(GL_FOG_MODE,  GL_LINEAR);
   glHint(GL_FOG_HINT, GL_FASTEST);
   glEnable(GL_BLEND);
}

void 
Surface::_unlock()
{
   OGL_ERROR_CHECK(__LINE__);
}

extern "C" {
 bool initLoader();
};

bool
Surface::choosePixelFormat(int&                   out_rPixelFormat,
                           PIXELFORMATDESCRIPTOR& out_rPFD,
                           HWND                   /*io_hWnd*/,
                           HDC                    io_hDC)
{
   // What we want to do here is determine the pixel format that we'll be using
   //
   // This is the desired PFD, 16-bit w/ backbuffer, and 16 bit depth buffer...
   //
   PIXELFORMATDESCRIPTOR pfd = {  
      sizeof(PIXELFORMATDESCRIPTOR),   //  size of this pfd  
      1,                               // version number 
      PFD_DRAW_TO_WINDOW |             // support window 
      PFD_SUPPORT_OPENGL |             // support OpenGL 
      PFD_DOUBLEBUFFER,                // double buffered 
      PFD_TYPE_RGBA,                   // RGBA type 
      24,                              // 24-bit color depth 
      0, 0, 0, 0, 0, 0,                // color bits ignored 
      0,                               // no alpha buffer 
      0,                               // shift bit ignored 

      0,                               // no accumulation buffer 
      0, 0, 0, 0,                      // accum bits ignored 
      16,                              // 16-bit z-buffer	 
      0,                               // no stencil buffer 
      0,                               // no auxiliary buffer 
      PFD_MAIN_PLANE,                  // main layer 
      0,                               // reserved 
      0, 0, 0                          // layer masks ignored 
   }; 

//   AssertMessage(0, "Choosing...");
   out_rPixelFormat = ChoosePixelFormat(io_hDC, &pfd);

   LPVOID lpMsgBuf;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,
                 GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                 (LPTSTR) &lpMsgBuf,
                 0,
                 NULL);

   AssertWarn(out_rPixelFormat != 0, avar("Unable to choose a pixel format: %s", lpMsgBuf));
   LocalFree(lpMsgBuf);

   if (DescribePixelFormat(io_hDC, out_rPixelFormat,
                           sizeof(PIXELFORMATDESCRIPTOR), &out_rPFD) == 0) {
      LPVOID lpMsgBuf;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL);

      AssertWarn(0, avar("Unable to choose a pixel format: %s", lpMsgBuf));
      LocalFree(lpMsgBuf);

      return false;
   }

//   AssertMessage(0, avar("Chosen pixelformat: %d", out_rPixelFormat));
//   AssertMessage(0, avar("\tColor Bits: %d", out_rPFD.cColorBits));
//   AssertMessage(0, avar("\tDepth Bits: %d", out_rPFD.cDepthBits));
//   AssertMessage(0, avar("\tPixel Type: %d", out_rPFD.iPixelType));
//   AssertMessage(0, avar("\tDoubled:    %s", (out_rPFD.dwFlags & PFD_DOUBLEBUFFER) ? "yes" : "no"));
//   AssertMessage(0, avar("\tGeneric:    %s", (out_rPFD.dwFlags & PFD_GENERIC_FORMAT) ? "yes" : "no"));
//   AssertMessage(0, avar("\tICD/MCD:    %s", (out_rPFD.dwFlags & PFD_GENERIC_ACCELERATED) ? "MCD" : "ICD"));

   bool pixelFormatValid;
   if (((out_rPFD.dwFlags & PFD_DOUBLEBUFFER) != 0) &&
       (out_rPFD.iPixelType == PFD_TYPE_RGBA)       &&
       (out_rPFD.cColorBits >= 16)                  &&
       (out_rPFD.cDepthBits >= 16)) {
      pixelFormatValid = true;
   } else {
      out_rPixelFormat = 0;
      pixelFormatValid     = false;
   }

   return pixelFormatValid;
}

bool
Surface::init(HWND /*io_hWnd*/)
{
   if (initLoader() == false)
      return false;

   return true;
}

void
Surface::shutdown()
{
   //
}

void 
Surface::_setGamma()
{
   // Desired gamma value is stored in the gamma member of GFXSurface
   //
   // OGL: extension for this function?
   if (m_pTextureCache != NULL)
      m_pTextureCache->setGamma(gamma);
}


void 
Surface::drawSurface(GFXSurface*    /*lpSourceSurface*/,
                     const RectI*   /*in_subRegion*/,
                     const Point2I* /*in_at*/)
{
   // Would be nice to have this, but it's not particularily necessary...
   //
}


void
Surface::drawSurfaceToBitmap(GFXBitmap *bmp)
{
   AssertFatal(bmp->getBitDepth() == 24, "Wrong bitdepth for a OGL screen read");
   AssertFatal(bmp->getWidth() == surfaceWidth && bmp->getHeight() == surfaceHeight,
               "Wrong size bitmap...");

   int rowSize = ((surfaceWidth * 3) + 0x3) & ~0x3;
   BYTE* pixels = new BYTE[rowSize * surfaceHeight];

   glReadBuffer(GL_FRONT);
   glReadPixels(0, 0, surfaceWidth, surfaceHeight,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
   
   int skipVal = 4 * int(ceil(3.0f * 3.0f * float(surfaceWidth) / 4.0f)) / 3;
   for (int y = 0; y < surfaceHeight; y++) {
      BYTE* currentRow = pixels + (y * skipVal);
      for (int x = 0; x < surfaceWidth; x++) {
         BYTE* pBits = bmp->getAddress(x, surfaceHeight - y - 1);
         pBits[0] = currentRow[2];
         pBits[1] = currentRow[1];
         pBits[2] = currentRow[0];
         currentRow += 3;
      }
   }

   delete [] pixels;
}

void 
Surface::processMessage(int         argc,
                        const char* argv[])
{
   if (argc == 0)
      return;
   
   const char* pMsg = argv[0];
   if (!strcmp(pMsg, "flushCache")) {
      flushTextureCache();
   } else if (!strcmp(pMsg, "bilinear")) {
      m_disableBilinear = m_disableBilinear == false;
   } else {
      // Any useful debugging functions can go here...
   }
}

const char*
translateOpenGLError(const GLenum in_glError)
{
   return reinterpret_cast<const char *>(gluErrorString(in_glError));
//
//   switch (in_glError) {
//     case GL_NO_ERROR:
//      return "GL_NO_ERROR";
//
//     case GL_INVALID_ENUM:
//      return "GL_INVALID_ENUM";
//
//     case GL_INVALID_VALUE:
//      return "GL_INVALID_VALUE";
//
//     case GL_INVALID_OPERATION:
//      return "GL_INVALID_OPERATION";
//
//     case GL_STACK_OVERFLOW:
//      return "GL_STACK_OVERFLOW";
//
//     case GL_STACK_UNDERFLOW:
//      return "GL_STACK_UNDERFLOW";
//
//     default:
//      return "Unknown or out of bounds error value";
//   }
}

DWORD
Surface::getCaps()
{
   return GFX_DEVCAP_SUPPORTS_CONST_ALPHA;
}

void
Surface::setFogDists(float in_start,
                     float in_end)
{
//   glFogi(GL_FOG_MODE,  GL_LINEAR);
//
//   const float fNear = m_nearClipPlane;
//
//   float startFog = fNear / in_start;
//   float endFog   = fNear / in_end;
//
//   glFogf(GL_FOG_START, g_start);
//   glFogf(GL_FOG_END,   g_end);
//   glFogf(GL_FOG_START, startFog);
//   glFogf(GL_FOG_END,   endFog);
//   OGL_ERROR_CHECK(__LINE__);
}

void
Surface::setSurfaceAlphaBlend(ColorF *color, float alpha)
{
   m_alphaBlend = alpha;
   m_alphaColor = *color;
}

void
Surface::setSurfaceFlareBlend(ColorF *color, float alpha)
{
   m_flareBlend = alpha;
   m_flareColor = *color;
}


} // namespace OpenGl


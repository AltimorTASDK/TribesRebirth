//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GOGLDEV_H_
#define _GOGLDEV_H_

//Includes
#include "gdevice.h"
#include "gdmanag.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//--------------------------------------  Forward decls.
//
class GFXPalette;
struct GFXDeviceDescriptor;

namespace OpenGL { class Surface; }

class OGLDevice : public GFXDevice {
  private:
   static bool sm_deviceInitialized;
   static bool sm_deviceCreated;

   OpenGL::Surface* m_pSurface;

   static RES_VECTOR sm_resVector;
   static void refreshResList();

   // Public pure virtuals from GFXDevice
   //
   bool setResolution(Point2I &res, GFXPalette* io_pPalette);
  public:
   bool setResolution(Point2I &res);
   bool enable(GFXPalette *palette, DWORD flags);
   void disable();
   bool modeSwitch(bool fullscreen);

   void flip(const RectI *);
   void lock(const GFXLockMode in_lockMode);
   void unlock();
   GFXSurface* getSurface();
   void messageSurface(const int   in_argc,
                               const char* in_argv[]);
                               
  protected:
   OGLDevice(HWND io_hApp, HWND io_hSurface);
  public:
   ~OGLDevice();

   static void startup(HWND);
   static void shutdown();

   static GFXDevice* create(HWND io_hApp, HWND io_hSurface);
   static bool enumerateModes(GFXDeviceDescriptor *dev);
   bool isVirtualFS();

  Int32 getDeviceId();
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GOGLDEV_H_

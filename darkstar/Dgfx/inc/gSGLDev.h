//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GSGLDEV_H_
#define _GSGLDEV_H_

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

namespace PowerSGL { class Surface; };

class SGLDevice : public GFXDevice {
  private:
   static bool sm_deviceInitialized;
   static bool sm_deviceCreated;

   PowerSGL::Surface* m_pSurface;

   // Public pure virtuals from GFXDevice
   //
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
   SGLDevice(HWND io_hApp, HWND io_hSurface);
  public:
   ~SGLDevice();

   static void startup();
   static void shutdown();

   static GFXDevice* create(HWND io_hApp, HWND io_hSurface);
   static bool enumerateModes(GFXDeviceDescriptor *dev);

  Int32 getDeviceId() {return GFX_POWERSGL_DEVICE;}
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GSGLDEV_H_

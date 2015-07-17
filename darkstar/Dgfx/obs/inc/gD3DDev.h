#ifndef _H_GD3DDEV
#define _H_GD3DDEV

#include "gdevice.h"
#include "gdmanag.h"

class GFXSurface;
struct GFXDeviceDescriptor;

class GFXD3DDevice : public GFXDevice
{
protected:
   static bool d3dInitialized;
   Int32  deviceMinorId;

   bool   restoreDisplay;

   GFXSurface *surface;

   bool setResolution(Point2I &res, GFXPalette *palette, DWORD flags, bool);

   // Must create a GFXD3DDevice through the ::create() method
   //
   GFXD3DDevice(Int32 deviceMinorId, HWND hApp,
                           HWND hSurface);
   ~GFXD3DDevice();
public:
   static void startup(HWND hWnd);
   static void shutdown();

   static GFXDevice* create(Int32 in_deviceMinorId, HWND hApp,
                                    HWND hSurface);
   static int        enumerateModes(GFXDeviceDescriptor *dev);

   bool enable(GFXPalette *palette, DWORD flags);
   void disable();

   bool setResolution(Point2I &res);
   bool modeSwitch(bool);

   void flip(const RectI *rect=NULL);
   void lock();
   void unlock();
   void messageSurface(const int in_argc, const char* in_argv[]);
   GFXSurface* getSurface();

   Int32 getDeviceId() {return GFX_D3D_DEVICE;}
};


#endif

#include "gdevice.h"
#ifndef _H_GSDEVICE
#define _H_GSDEVICE

#include "windows.h"
#include "ddraw.h"
#include "gdmanag.h"
#include "g_surfac.h"

class GFXSoftwareDevice : public GFXDevice
{
protected:
   enum { minXSize = 1, minYSize = 1, maxXSize = 1280, maxYSize = 1024};

   GFXSurface* m_pSurface;

   bool restoreDisplay;
   bool setResolution(Point2I &res, GFXPalette *palette, DWORD flags, bool);

   static HRESULT WINAPI EnumModesCallback(LPDDSURFACEDESC pddsd, LPVOID lpContext);
   void getResVector();

   const ResSpecifier& getResSpec(const Point2I&);
public:
   GFXSoftwareDevice(HWND appWind, HWND clientWind);
   ~GFXSoftwareDevice();

   static GFXDevice* create(HWND appWind, HWND clientWind);
   static bool enumerateModes(GFXDeviceDescriptor *dev);
   static void startup(HWND appWind);
   static void shutdown();
   static bool ddInitialized;

   bool enable(GFXPalette *pal, DWORD flags);
   void disable();
   bool setResolution(Point2I &res);
   bool modeSwitch(bool fullscreen);
   bool isVirtualFS();

   void messageSurface(const int in_argc, const char* in_argv[]);
   void flip(const RectI *rect=NULL);
   void lock(const GFXLockMode in_lockMode);
   void unlock();
   GFXSurface* getSurface();

   Int32 getDeviceId() {return GFX_SOFTWARE_DEVICE;}
};

#endif

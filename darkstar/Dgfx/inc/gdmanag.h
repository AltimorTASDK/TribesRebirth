#ifndef _H_GDMANAG
#define _H_GDMANAG

#include "base.h"
#include "windows.h"
#include "m_point.h"

#define ARBITRARY_MAX_DEVICES  10
enum devices { GFX_SOFTWARE_DEVICE,
               GFX_GLIDE_DEVICE,
               GFX_D3D_DEVICE,
               GFX_RENDITION_DEVICE,
               GFX_POWERSGL_DEVICE,
               GFX_OPENGL_DEVICE };

class GFXDevice;
class GFXMetricsInfo;
class GFXPrefInfo;
class GFXSurface;
class GFXBitmap;
class ResSpecifier;

struct GFXDeviceDescriptor
{
   enum deviceFlags { runsFullscreen = 1, runsWindowed = 2 };
   Int32 flags;
   Int32 deviceId;
   Int32 deviceMinorId;
   HINSTANCE driverLibrary;
   const char* name;
   const char* pRendererName;
   Int32 resolutionCount;
   ResSpecifier* resolutionList;
   Point2I minSize, maxSize;

   GFXDeviceDescriptor() : pRendererName(NULL) { }
};



// all the functions have to be either inline or virtual
// because this is in a dll.

class GFXDeviceManager
{
public:
   virtual void enumerateDevices(Int32 &deviceCount, GFXDeviceDescriptor *&deviceList);
   virtual GFXDevice* allocateDevice(Int32 deviceId, Int32 deviceMinorId,
                                     HWND appWind, HWND clientWind);
   virtual void freeDevice(GFXDevice *);
   virtual GFXMetricsInfo* getMetrics();
   virtual GFXPrefInfo*    getPrefs();
   virtual GFXSurface* createMemSurface(GFXBitmap *bmp);
   virtual void freeMemSurface(GFXSurface *);
};

#ifndef GFX_DLL
extern GFXDeviceManager *GetDeviceManager(HWND appWind);
extern void FreeDeviceManager();
extern "C" {
   void __cdecl UnloadGlide();
}
#endif

#endif

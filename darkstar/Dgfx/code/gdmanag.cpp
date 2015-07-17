#include "gdmanag.h"
#include "gsdevice.h"
#include "gglidedev.h"
#include "gSGLDev.h"
#include "gOGLDev.h"
#include "gfxmetrics.h"
#include "types.h"
#include "windows.h"
#include "g_mem.h"

GFXDeviceDescriptor devList[ARBITRARY_MAX_DEVICES];
GFXDeviceManager manager;
int referenceCount = 0;

typedef GFXDeviceManager* DevManagerPtr;

// Note: DO NOT use DLLAPI in this declaration or in FreeDev..., as DLLAPI is
//  defined as empty for compiling dgfx...
// Note as well: )*!#&$)!&$_*& MSVC!
//
#ifdef GFX_DLL
extern "C" DevManagerPtr __declspec(dllexport) __cdecl
#else
DevManagerPtr
#endif
GetDeviceManager(HWND appWind)
{
   if(!referenceCount++) {
      GFXSoftwareDevice::startup(appWind);
      OGLDevice::startup(appWind);
   }
   
   return(&manager);
}

#ifdef GFX_DLL
extern "C" void __declspec(dllexport) __cdecl 
#else
void
#endif
FreeDeviceManager()
{
   if(!--referenceCount) {
      GFXSoftwareDevice::shutdown();
      OGLDevice::shutdown();
   }
}

void GFXDeviceManager::enumerateDevices(Int32 &deviceCount, GFXDeviceDescriptor *&deviceList)
{
   deviceCount = 0;
   deviceList = devList;

   if(GFXSoftwareDevice::enumerateModes(&devList[deviceCount]) == true) {
      deviceCount++;
   }
   if(GFXGlideDevice::enumerateModes(&devList[deviceCount]) == true) {
      deviceCount++;
   }
   if (OGLDevice::enumerateModes(&devList[deviceCount]) == true) {
      deviceCount++;
   }
}

GFXDevice* 
GFXDeviceManager::allocateDevice(Int32 deviceId, Int32 /*deviceMinorId*/,
                                 HWND appWind, HWND clientWind)
{
   switch(deviceId)
   {
      case GFX_SOFTWARE_DEVICE:
         return GFXSoftwareDevice::create(appWind, clientWind);
      case GFX_GLIDE_DEVICE:
         return GFXGlideDevice::create(appWind, clientWind);
      case GFX_OPENGL_DEVICE:
         return OGLDevice::create(appWind, clientWind);
      default:
         AssertFatal(0, "Invalid device ID.");
         return NULL;
   }
}

void 
GFXDeviceManager::freeDevice(GFXDevice *dev)
{
   delete dev;
}

GFXMetricsInfo* 
GFXDeviceManager::getMetrics()
{ 
   return &GFXMetrics;
}

GFXPrefInfo* 
GFXDeviceManager::getPrefs()
{ 
   return &GFXPrefs;
}

GFXSurface* 
GFXDeviceManager::createMemSurface(GFXBitmap *bmp)
{
   return GFXMemSurface::create(bmp);
}

void 
GFXDeviceManager::freeMemSurface(GFXSurface *sfc)
{
   delete sfc;
}

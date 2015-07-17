#include "windows.h"
#include "gdmanag.h"
#include "gwDeviceManager.h"

GWDeviceManager deviceManager;

#ifdef BORLAND
#ifdef DEBUG
const char *gfxDll = "dbdgfx.dll";
#else
const char *gfxDll = "rbdgfx.dll";
#endif
#else
#ifdef DEBUG
const char *gfxDll = "dmdgfx.dll";
#else
const char *gfxDll = "rmdgfx.dll";
#endif
#endif

GFXDeviceManager* GWDeviceManager::getGFXDeviceManager()
{
   return manager;
}

GWDeviceManager::GWDeviceManager()
{
   dllInst = NULL;
#ifdef GFX_DLL
   GetDeviceManager  = NULL;
   FreeDeviceManager = NULL;
   dllInst = LoadLibrary(gfxDll);
   AssertFatal(dllInst, "could not load rendering dll.");
#ifdef BORLAND
   GetDeviceManager = (GetDeviceManager_fpt) GetProcAddress(dllInst, "_GetDeviceManager");
   FreeDeviceManager = (FreeDeviceManager_fpt) GetProcAddress(dllInst, "_FreeDeviceManager");
   UnloadGlide = (UnloadGlide_fpt) GetProcAddress(dllInst, "_UnloadGlide");
#else
   GetDeviceManager = (GetDeviceManager_fpt) GetProcAddress(dllInst, "GetDeviceManager");
   FreeDeviceManager = (FreeDeviceManager_fpt) GetProcAddress(dllInst, "FreeDeviceManager");
   UnloadGlide = (UnloadGlide_fpt) GetProcAddress(dllInst, "UnloadGlide");
#endif
#endif
   manager = NULL;
   AssertFatal(GetDeviceManager != NULL && FreeDeviceManager != NULL, "missing functions in render dll.");
}

GWDeviceManager::~GWDeviceManager()
{
   if(manager)
      FreeDeviceManager();
   if(dllInst)
      FreeLibrary(dllInst);
}

void GWDeviceManager::freeDevice(GFXDevice *dev)
{
   manager->freeDevice(dev);
}

bool GWDeviceManager::deviceNameValid(const char *name)
{
   return(getDeviceId(name) != -1);
}

void GWDeviceManager::enumerateDevices(int &deviceCount, GFXDeviceDescriptor *&deviceList)
{
   if(!manager)
   {
      deviceCount = 0;
      return;
   }
   Int32 dc;
   GFXDeviceDescriptor *dl;
   manager->enumerateDevices(dc, dl);
   deviceCount = dc;
   deviceList = dl;
}

GFXDevice* GWDeviceManager::allocateDevice(const char *name, HWND appWind, HWND clientWind)
{
   if(!manager)
      manager = GetDeviceManager(appWind);
   int id, minorId;
   if((id = getDeviceId(name)) == -1)
      return NULL;
   minorId = getDeviceMinorId(name);
   return manager->allocateDevice(id, minorId, appWind, clientWind);
}

int GWDeviceManager::getDeviceId(const char *name)
{
   Int32 deviceCount;
   GFXDeviceDescriptor *deviceList;
   manager->enumerateDevices(deviceCount, deviceList);
   AssertFatal(deviceCount, "No available devices in driver dll.");
   int i;
   for(i = 0; i < deviceCount; i++)
   {
      if(!strcmp(deviceList[i].name, name))
         return deviceList[i].deviceId;
   }
   return(-1);
}

const char * GWDeviceManager::getDeviceName(int id)
{
   Int32 deviceCount;
   GFXDeviceDescriptor *deviceList;
   manager->enumerateDevices(deviceCount, deviceList);
   AssertFatal(deviceCount, "No available devices in driver dll.");
   int i;
   for(i = 0; i < deviceCount; i++)
   {
      if (deviceList[i].deviceId == id)
         return deviceList[i].name;
   }
   return(NULL);
}

int GWDeviceManager::getDeviceMinorId(const char *name)
{
   Int32 deviceCount;
   GFXDeviceDescriptor *deviceList;
   manager->enumerateDevices(deviceCount, deviceList);
   AssertFatal(deviceCount, "No available devices in driver dll.");
   int i;
   for(i = 0; i < deviceCount; i++)
   {
      if(!strcmp(deviceList[i].name, name))
         return deviceList[i].deviceMinorId;
   }
   return(-1);
}


GFXMetricsInfo* GWDeviceManager::getMetrics()
{
   if(manager)
      return manager->getMetrics();
   else
      return NULL;
}

GFXPrefInfo* GWDeviceManager::getPrefs()
{
   if(manager)
      return manager->getPrefs();
   else
      return NULL;
}

void
GWDeviceManager::unloadGlide()
{
   if (UnloadGlide != NULL)
      UnloadGlide();
}

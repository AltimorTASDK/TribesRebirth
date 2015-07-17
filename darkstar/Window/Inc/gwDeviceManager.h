#ifndef _H_GWDEVICEMANAGER
#define _H_GWDEVICEMANAGER

class  GFXDeviceManager;
class  GFXDevice;
class  GFXMetricsInfo;
class  GFXPrefInfo;
struct GFXDeviceDescriptor;

class GWDeviceManager
{
private:
   typedef GFXDeviceManager * (__cdecl *GetDeviceManager_fpt)(HWND);
   typedef void (__cdecl *FreeDeviceManager_fpt)();
   typedef void (__cdecl *UnloadGlide_fpt)();
#ifdef GFX_DLL
   GetDeviceManager_fpt GetDeviceManager;
   FreeDeviceManager_fpt FreeDeviceManager;
   UnloadGlide_fpt UnloadGlide;
#endif
   GFXDeviceManager *manager;
   int getDeviceMinorId(const char *name);
   HINSTANCE dllInst;
public:
   GWDeviceManager();
   ~GWDeviceManager();

   void unloadGlide();

   int getDeviceId(const char *name);
   const char *getDeviceName(int id);
   void freeDevice(GFXDevice *dev);
   bool deviceNameValid(const char *name);
   void enumerateDevices(int &deviceCount, GFXDeviceDescriptor *&deviceList);
   GFXDevice* allocateDevice(const char *name, HWND appWind, HWND clientWind);
   GFXMetricsInfo* getMetrics();
   GFXPrefInfo* getPrefs();
   GFXDeviceManager* getGFXDeviceManager();
};

extern GWDeviceManager deviceManager;

#endif

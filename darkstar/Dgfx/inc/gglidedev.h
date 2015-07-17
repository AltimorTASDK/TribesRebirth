#ifndef _H_GGLIDEDEV
#define _H_GGLIDEDEV

#include "gdevice.h"
#include "gdmanag.h"

class GFXSurface;

class GFXGlideDevice : public GFXDevice
{
protected:
   GFXSurface *surface;
   bool setResolution(Point2I &res, GFXPalette *palette, DWORD flags, bool);
public:
   GFXGlideDevice( HWND hApp, HWND hSurface );
   ~GFXGlideDevice();

   static GFXDevice* create( HWND hApp, HWND hSurface );
   static bool enumerateModes(GFXDeviceDescriptor *dev);

   bool enable(GFXPalette *palette, DWORD flags);
   void disable();

   bool setResolution(Point2I &res);
   bool modeSwitch(bool);

   void flip(const RectI *rect=NULL);
   void lock(const GFXLockMode);
   void unlock();
   GFXSurface* getSurface();
   void messageSurface(const int in_argc, const char* in_argv[]);

   Int32 getDeviceId() {return GFX_GLIDE_DEVICE;}

   bool isVirtualFS();
};


#endif

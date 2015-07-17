#ifndef _H_GDEVICE
#define _H_GDEVICE

#include "d_defs.h"
#include "windows.h"
#include "tvector.h"
#include "m_point.h"

class GFXSurface;
class GFXPalette;
class RectI;

class ResSpecifier {
public :
   Point2I res;
   bool    isVirtual;

   ResSpecifier() : res(0, 0), isVirtual(false) { }
   ResSpecifier(const Point2I& in_rRes,
                const bool     in_isVirtual = false) : res(in_rRes), isVirtual(in_isVirtual) { }

   bool operator==(const ResSpecifier& in_rComp) const {
      return (res == in_rComp.res) &&
             (isVirtual == in_rComp.isVirtual);
   }
   bool operator==(const Point2I& in_rRes) const {
      return (res == in_rRes) && (isVirtual == false);
   }
};

typedef Vector<ResSpecifier> RES_VECTOR;

class GFXDevice
{
  protected:
   HWND appWind;
   HWND clientWind;
   Point2I currentSize;
   bool enabled;
   bool isFullscr;
   RES_VECTOR resVector;
   bool restoreWin;
   RECT windowRect;

   void setWindow(Point2I *size = NULL);
   void restoreWindow();
   static int _USERENTRY compareResolution(const void *_a, const void *_b);

  public:
   bool isFullscreen() { return isFullscr; }
   virtual void sortResolutions();

   GFXDevice( HWND aWind, HWND cWind);
   virtual ~GFXDevice() {}

   virtual Point2I closestRes(Point2I &res);
   const Point2I&  getResolution() { return currentSize; };
   virtual bool    isVirtualFS() = 0;
   virtual bool    setResolution(Point2I &res) = NULL;
   virtual bool    nextRes();
   virtual bool    prevRes();

   virtual bool enable(GFXPalette *palette, DWORD flags) = NULL;
   virtual void disable() = NULL;   
   virtual bool modeSwitch(bool fullscreen) = NULL;

   virtual void flip(const RectI *rect=NULL) = NULL;
   virtual void lock(const GFXLockMode in_lockMode = GFX_NORMALLOCK) = NULL;
   virtual void unlock()            = NULL;
   virtual GFXSurface* getSurface() = NULL;
   virtual void messageSurface(const int   in_argc,
                                       const char* in_argv[]) = NULL;

   virtual Int32 getDeviceId() = NULL; 
};

inline 
GFXDevice::GFXDevice (HWND aWind, HWND cWind)
{
   enabled = false;
   appWind = aWind;
   clientWind = cWind;
   currentSize.set(640, 480);
}

#endif
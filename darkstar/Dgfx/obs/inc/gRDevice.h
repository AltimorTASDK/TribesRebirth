//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GRDEVICE_H_
#define _GRDEVICE_H_

//Includes
#include "gdevice.h"
#include "gdmanag.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Rendition { class RenditionProxy; class Surface; };
using namespace Rendition;
struct GFXDeviceDescriptor;

class GFXRenditionDevice : public GFXDevice {
  private:
   // Class members
   //
   static const char* const sm_pDeviceName;
   static bool              sm_started;
   static HINSTANCE         sm_hRedlineDLL;
   static HINSTANCE         sm_hVeriteDLL;
   static RenditionProxy*   sm_pRenditionProxy;
   static Vector<Point2I>   sm_rResVector;
   
   // Instance variables
   //
   HWND        m_sfcHWnd;
   bool        m_enabled;
   bool        m_windowed;
   bool        m_windowedNotDetermined;
   Surface*    m_pSurface;

   // Must create a GFXRenditionDevice through the ::create() method
   //
   GFXRenditionDevice(HWND       io_hApp,
                                 HWND       io_hSurface,
                                 const bool in_windowed);
   ~GFXRenditionDevice();

  protected:
   bool setResolution(Point2I&    io_res,
                              GFXPalette *io_pPalette,
                              const DWORD in_flags);
  public:
   static char* sm_pMicrocodeFile;

   static GFXDevice* create(HWND       io_hApp,
                                    HWND       io_hSurface,
                                    const bool in_windowed = false);
   static void startup(HWND io_appHWnd);
   static void shutdown();

   static RenditionProxy* getRenditionProxy();

   static bool enumerateModes(GFXDeviceDescriptor *out_dev);
   
   bool enable(GFXPalette* io_pPalette, DWORD io_flags);
   void disable();
   
   bool setResolution(Point2I &io_res);
   bool modeSwitch(bool io_fullScreen);
   void flip(const RectI *rect=NULL);
   void lock();
   void unlock();
   GFXSurface* getSurface();
   void messageSurface(const int   in_argc,
                               const char* in_argv[]);

   Int32 getDeviceId() {return GFX_RENDITION_DEVICE;}
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GRDEVICE_H_

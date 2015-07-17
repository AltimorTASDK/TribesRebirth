#include <stdlib.h>
#include <base.h>
#include "gglidedev.h"
#include "g_surfac.h"
#include "glidesfc.h"

#define __MSC__
#define DYNAHEADER
#include "glide.h"


static RES_VECTOR gResVector;
char g_glideVersion[80];


GFXGlideDevice::GFXGlideDevice(HWND hApp, HWND hSurface)
   : GFXDevice(hApp, hSurface)
{
   surface = NULL;
   currentSize.set(640,480);
   resVector.push_back(ResSpecifier(Point2I(512,  384)));
   resVector.push_back(ResSpecifier(Point2I(640,  400)));
   resVector.push_back(ResSpecifier(Point2I(640,  480)));
   resVector.push_back(ResSpecifier(Point2I(800,  600)));
//   resVector.push_back(ResSpecifier(Point2I(856,  480)));
//   resVector.push_back(ResSpecifier(Point2I(960,  720)));
   resVector.push_back(ResSpecifier(Point2I(1024, 768)));
   sortResolutions();
}

GFXGlideDevice::~GFXGlideDevice()
{
   if(enabled)
      disable();
}

void GFXGlideDevice::lock(const GFXLockMode in_lockMode)
{
   if(surface)
      surface->lock(in_lockMode);
}

void GFXGlideDevice::unlock()
{
   
}

bool GFXGlideDevice::modeSwitch(bool fs)
{
   isFullscr = fs;
   return true;
   //if(fs)
   //   isFullscr = true;
   //return(fs);
}

bool GFXGlideDevice::enumerateModes(GFXDeviceDescriptor *dev)
{
   bool runsWindowed;
   if(!Glide::Surface::init(&gResVector, runsWindowed))
      return(false);
   dev->flags = 0;
   dev->flags |= GFXDeviceDescriptor::runsFullscreen;
   if(runsWindowed)
      dev->flags |= GFXDeviceDescriptor::runsWindowed;
   dev->deviceId = GFX_GLIDE_DEVICE;
   dev->name = "Glide";

   if (g_glideVersion[0] == '\0') {
      grGlideGetVersion(g_glideVersion);
   }
   dev->pRendererName = (g_glideVersion[0] != '\0') ? g_glideVersion : NULL;

   dev->resolutionCount = gResVector.size();
   qsort(gResVector.address(), gResVector.size(), sizeof(ResSpecifier), compareResolution);
   dev->resolutionList = gResVector.address();

   return(true);
}
   
GFXDevice* GFXGlideDevice::create(HWND hApp, HWND hSurface)
{
   return(new GFXGlideDevice(hApp, hSurface));
}

void GFXGlideDevice::flip(const RectI *)
{
   if ( surface ) surface->flip();
}


//------------------------------------------------------------------------------
bool GFXGlideDevice::enable(GFXPalette *palette, DWORD flags)
{
   //return FALSE;

   enabled = true;
   enabled = setResolution( currentSize, palette, flags, true);
   return ( enabled );
}


//------------------------------------------------------------------------------
void GFXGlideDevice::disable()
{
   delete surface;
   surface = NULL;
   enabled = false;
   //if ( restoreWin ) restoreWindow();
}


bool GFXGlideDevice::setResolution(Point2I &res)
{
   //return ( false );
   //res;
   return ( setResolution( res, 
            surface ? surface->getPalette() : NULL, 
            surface ? surface->getFlags() : GFX_DMF_RCLIP, true) );   
}


GFXSurface* GFXGlideDevice::getSurface()
{
   return(surface);
}

//------------------------------------------------------------------------------
bool GFXGlideDevice::setResolution(Point2I &res, GFXPalette *palette, DWORD flags, bool)
{
   if (enabled == false) {
      currentSize = closestRes(res);
      return true;
   }

   flags;
   float gamma = -1.0f;

	if(surface)
	{
      gamma = surface->getGamma();
		delete surface;
      surface = NULL;
	}
   res = closestRes(res);
   if(surface && res == currentSize)
      return TRUE;

	surface = Glide::Surface::create(clientWind, res.x, res.y, isFullscr);
	if(!surface)
   {
      res = currentSize;
      surface = Glide::Surface::create(clientWind, res.x, res.y, isFullscr);
   }
   if(!surface)
		return FALSE;
   currentSize = res;
   //setWindow();
	if(palette)
		surface->setPalette(palette);
      
   if (gamma > 0.0f)
      surface->setGamma(gamma);

	return TRUE;
}

namespace Glide
{
   extern void plugCallback(const int in_argc, const char* in_argv[],
                            GFXSurface* pSurface);
};   

void 
GFXGlideDevice::messageSurface(const int   in_argc,
                               const char* in_argv[])
{
   Glide::plugCallback(in_argc, in_argv, surface);
}

bool
GFXGlideDevice::isVirtualFS()
{
   // Glide is never virtual...
   return false;
}

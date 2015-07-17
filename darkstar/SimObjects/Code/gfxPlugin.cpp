//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <sim.h>
#include <gfxMetrics.h>
#include <g_surfac.h>
#include <g_bitmap.h>

#include "simCanvas.h"
#include "simConsolePlugin.h"
#include "gfxPlugin.h"
#include "gdmanag.h"
#include "gwDeviceManager.h"
#include "simGuiCanvas.h"

bool g_dummy = true;

//----------------------------------------------------------------------------
enum CallbackID {
   FlushTextureCache,
   SetGamma,
   SetFullscreenDevice,
   SetWindowedDevice,
   ListDevices,
   TestDevice,
   MessageCanvasDevice,
   SetScreenShotSeq,
   ScreenShot,
   SwapSurfaces,
   IsFullscreenMode,
   SetFullscreenMode,
   SetFSResolution,
   IsVirtualFS,
   NextRes,
   PrevRes,
   LockWindowSize,
   UnlockWindowSize,
   SetWindowSize,
   IsGfxDriver,
   ResetUpdateRegion,
};


static int ScreenShotSeq = 0;
namespace Glide
{
   extern int mipCap;
};

void GFXPlugin::init()
{
   m_fullyInitialized = false;

	console->printf("GFXPlugin");
   console->addCommand(SwapSurfaces,       "swapSurfaces",        this);
   console->addCommand(IsFullscreenMode,   "isFullscreenMode",    this);
   console->addCommand(SetFullscreenMode,  "setFullscreenMode",   this);
   console->addCommand(SetFSResolution,    "setFSResolution",     this);
   console->addCommand(IsVirtualFS,        "isVirtualFS",         this);
   console->addCommand(NextRes,            "nextRes",             this);
   console->addCommand(PrevRes,            "prevRes",             this);
	console->addCommand(ScreenShot,         "screenShot",          this);
	console->addCommand(SetScreenShotSeq,   "setScreenShotSeq",    this);
	console->addCommand(FlushTextureCache,  "flushTextureCache",   this);
	console->addCommand(ResetUpdateRegion,  "resetUpdateRegion",   this);
	console->addCommand(SetGamma,           "setGamma",            this);
	console->addCommand(SetFullscreenDevice,"setFullscreenDevice", this);
	console->addCommand(SetWindowedDevice,  "setWindowedDevice",   this);
	console->addCommand(ListDevices,        "listDevices",         this);
	console->addCommand(TestDevice,         "testDevice",          this);
	console->addCommand(MessageCanvasDevice,"messageCanvasDevice", this);
	console->addCommand(SetWindowSize,      "setWindowSize",       this);
	console->addCommand(LockWindowSize,     "lockWindowSize",      this);
	console->addCommand(UnlockWindowSize,   "unlockWindowSize",    this);
	console->addCommand(IsGfxDriver,        "isGfxDriver",         this);
	console->addVariable(0, "$pref::mipcap", CMDConsole::Int, &Glide::mipCap);

   // Controls for OpenGL Driver
   extern bool g_prefOGLAlwaysRGBA;
   extern bool g_prefOGLUse32BitTex;
   extern bool g_prefOGLNoPackedTex;
   extern bool g_prefOGLNoAddFade;

   extern UInt32 g_texDownloadThisFrame;
   extern UInt32 g_lmDownloadThisFrame;
   extern UInt32 g_lmDownloadBytes;
   extern UInt32 g_oglEntriesTouched;
   extern float  g_handleSearchLen;
   extern float  g_oglAverageFrameKey;

   extern UInt32 g_totalNumBytes;

	console->addVariable(0, "$pref::OpenGL::AlwaysRGBA",         CMDConsole::Bool, &g_prefOGLAlwaysRGBA);
	console->addVariable(0, "$pref::OpenGL::Use32BitTex",        CMDConsole::Bool, &g_prefOGLUse32BitTex);
   console->addVariable(0, "$pref::OpenGL::NoPackedTextures",   CMDConsole::Bool, &g_prefOGLNoPackedTex);
   console->addVariable(0, "$pref::OpenGL::NoAddFade",          CMDConsole::Bool, &g_prefOGLNoAddFade);

	console->addVariable(0, "$OpenGL::TexDL", CMDConsole::Int,   &g_texDownloadThisFrame);
	console->addVariable(0, "$OpenGL::LMDL",  CMDConsole::Int,   &g_lmDownloadThisFrame);
	console->addVariable(0, "$OpenGL::LMB",   CMDConsole::Int,   &g_lmDownloadBytes);
	console->addVariable(0, "$OpenGL::ET",    CMDConsole::Int,   &g_oglEntriesTouched);
	console->addVariable(0, "$OpenGL::HSL",   CMDConsole::Float, &g_handleSearchLen);
	console->addVariable(0, "$OpenGL::AFK",   CMDConsole::Float, &g_oglAverageFrameKey);
	console->addVariable(0, "$OpenGL::TNB",   CMDConsole::Int,   &g_totalNumBytes);
}

const char *GFXPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   argc; // unused parameter
   argv; // unused parameter

	switch(id)
	{
      case ScreenShot:
         if ( argc>1 )
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               char buffer[256];
					const char* fname;

					if (argc > 2)
						fname = argv[2];
					else {
	               do
	               {  // INC seq number until we get a unique name
	                  sprintf(buffer, "temp\\sshot%04d.bmp", ScreenShotSeq);
	                  ScreenShotSeq++;
	               }while ( GetFileAttributes( buffer ) != 0xFFFFFFFF );
						fname = buffer;
					}

               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               GFXSurface *sfc = pc->getSurface();
               GFXBitmap *bmp = GFXBitmap::create(sfc->getWidth(), sfc->getHeight(), 24);
               if(sfc)
               {
                  sfc->drawSurfaceToBitmap(bmp);
                  bmp->write(fname);
               }

               delete bmp;
               return 0;
            }
         break;

      case SetScreenShotSeq:
         if ( argc>1 )
            ScreenShotSeq = atoi( argv[1] );
			return 0;

      case SetFullscreenDevice:
         if ( argc>2 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               if (pc->setFullscreenDevice(argv[2]) == true) {
                  return "true";
               }
               else
                  console->printf("Failed to set Fullscreen Device to %s", argv[2]);
            }
            else
               console->printf("Could not find simCanvas: %s", argv[1]);
         }
			else
	   		console->printf("setFullscreenDevice: simCanvasName deviceName");
         break;
      case SwapSurfaces:
         if ( argc>1 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               pc->swapSurfaces();
		         return 0;
            }
         }
   		console->printf("swapSurfaces: simCanvasName");
         break;
      case IsFullscreenMode:
         if ( argc>1 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               if (pc && pc->isFullScreen()) return "TRUE";
               else return "FALSE";
            }
         }
   		console->printf("swapSurfaces: simCanvasName");
         break;
      case SetFullscreenMode:
         if ( argc>2 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               if (stricmp(argv[2], "true") == 0) {
                  pc->setFullScreen(true);
               } else {
                  pc->setFullScreen(false);
               }
		         return 0;
            }
         }
   		console->printf("setFullscreenMode: simCanvasName (true/false)");
         break;
      case SetFSResolution:
         if ( argc>2 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               Point2I res;
               sscanf(argv[2], "%dx%d", &res.x, &res.y);
               GFXDevice* pDevice = pc->getFullScreenDevice();
               Point2I final = pDevice->closestRes(res);
               pDevice->setResolution(final);
		         return 0;
            }
         }
   		console->printf("setFSResolution: simCanvasName CxC");
         break;
      case IsVirtualFS:
         if (argc > 1) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               GFXDevice* pDevice = pc->getFullScreenDevice();
               if (pDevice == pc->getCurrentDevice()) {
                  if (pDevice->isVirtualFS() == true)
                     return "true";
                  else
                     return "false";
               }
		         return "false";
            }
         }
         console->printf("isVirtualFS: simCanvasName");
         break;
      case NextRes:
         if ( argc>1 )
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               pc->nextRes();
               Point2I res = pc->getClientSize();
               console->printf("Mode: %d x %d", res.x, res.y);
               return 0;
            }
   		console->printf("nextRes: simCanvasName");
         break;
      case PrevRes:
         if ( argc>1 )
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               pc->prevRes();
               Point2I res = pc->getClientSize();
               console->printf("Mode: %d x %d", res.x, res.y);
               return 0;
            }
   		console->printf("prevRes: simCanvasName");
         break;
      case SetWindowedDevice:
         if ( argc>2 ) {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               if (pc->setWindowedDevice(argv[2]) == true) {
                  return 0;
               } else {
                  console->printf("Failed to set windowed Device to %s", argv[2]);
                  return "False";
               }
            } else {
               console->printf("Could not find simCanvas: %s", argv[1]);
               return "False";
            }
         }
   		console->printf("setWindowedDevice: simCanvasName deviceName");
         break;
      case MessageCanvasDevice:
         if(argc > 2)
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               GFXDevice *dev = pc->getCurrentDevice();
               if(dev)
               {
                  dev->messageSurface(argc - 2, argv + 2);
                  return 0;
               }
            }
   		console->printf("messageCanvasDevice: simCanvasName string");
         break;
 	   case FlushTextureCache:
         if(argc > 1)
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
               pc->getSurface()->flushTextureCache();
               return 0;
            }
         }
			else if (SimObject *g = manager->findObject( SimCanvasSetId ))
				if ( SimSet *set = dynamic_cast<SimSet *>(g) )
				{
					Vector<SimObject *>::iterator iter;
					
					for ( iter = set->begin(); iter != set->end(); iter++ )
						if ( SimCanvas *pc = dynamic_cast<SimCanvas*>(*iter) )
		               pc->getSurface()->flushTextureCache();
							
					return 0;							
				}
   		console->printf("flushTextureCache: simCanvasName");
         break;
      case ResetUpdateRegion:
         if(argc > 1)
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimGui::Canvas *pc = dynamic_cast<SimGui::Canvas*>(obj);
               if (pc != NULL) {
                  pc->resetUpdateRegions();
                  return 0;
               } else {
            		console->printf("resetUpdateRegion: %s is not a SimGui::Canvas", argv[1]);
                  break;
               }
            }
         }
			else 
   		console->printf("resetUpdateRegion: simCanvasName");
         break;
         
      case ListDevices:
         GFXDeviceDescriptor *deviceList;
         int deviceCount;
         deviceManager.enumerateDevices(deviceCount, deviceList);
         console->printf("Number of devices: %d", deviceCount);
         int i;
         for(i = 0; i < deviceCount; i++)
         {
            char buffer[1024];
            char *bptr = buffer;

            if (deviceList[i].pRendererName == NULL) {
               bptr += sprintf(bptr, "%s:%s%s ", deviceList[i].name,
                    deviceList[i].flags & GFXDeviceDescriptor::runsFullscreen ? " (FS)" : "",
                    deviceList[i].flags & GFXDeviceDescriptor::runsWindowed ? " (W)" : "");
            } else {
               bptr += sprintf(bptr, "%s (%s):%s%s ", deviceList[i].name, deviceList[i].pRendererName,
                    deviceList[i].flags & GFXDeviceDescriptor::runsFullscreen ? " (FS)" : "",
                    deviceList[i].flags & GFXDeviceDescriptor::runsWindowed ? " (W)" : "");
            }
            if(deviceList[i].resolutionCount)
            {
               int j;
               for(j = 0;;j++)
               {
                  bptr += sprintf(bptr, "%d x %d", 
                     deviceList[i].resolutionList[j].res.x, deviceList[i].resolutionList[j].res.y);
                  if (deviceList[i].resolutionList[j].isVirtual == true) {
                     bptr += sprintf(bptr, "(V)");
                  }
                  if(j == deviceList[i].resolutionCount - 1)
                     break;
                  bptr += sprintf(bptr, ", ");
               }
            }
            console->printf("%s", buffer);
         }
         return 0;
      case TestDevice:
         if (argc == 2) {
            if (deviceManager.deviceNameValid(argv[1])) {
               // device found
               return 0;
            } else {
               // no such device present...
               return "False";
            }
         }
  		   console->printf("testDevice <deviceName>");
			break;
      case SetGamma:
         if (argc == 3)
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);

               float newGamma = atof(argv[2]);
               if (newGamma <= 0.05) {
                  console->printf("Out of bounds gamma, must be between 0.05 and 20, value clamped");
                  newGamma = 0.05f;
               } else if (newGamma >= 20) {
                  console->printf("Out of bounds gamma, must be between 0.05 and 20, value clamped");
                  newGamma = 20;
               }

               console->setFloatVariable("pref::Display::gammaValue", newGamma);
               pc->getSurface()->setGamma(newGamma);

               return 0;
            }
   		console->printf("setGamma: simCanvasName gammaValue");
			break;
		case LockWindowSize:
         if (argc == 2)
            if (SimObject *obj = manager->findObject(argv[1]))
	           	if (SimCanvas *can = dynamic_cast<SimCanvas*>(obj)) {
						can->lockSize();
						return 0;
	           	}
   		console->printf("lockWindowSize: simCanvasName");
			break;
		case UnlockWindowSize:
         if (argc == 2)
            if (SimObject *obj = manager->findObject(argv[1]))
	           	if (SimCanvas *can = dynamic_cast<SimCanvas*>(obj)) {
						can->unlockSize();
						return 0;
	           	}
   		console->printf("unlockWindowSize: simCanvasName");
			break;
		case SetWindowSize:
         if (argc == 4)
            if (SimObject *obj = manager->findObject(argv[1]))
	           	if (SimCanvas *can = dynamic_cast<SimCanvas*>(obj)) {
						Point2I size(atoi(argv[2]),atoi(argv[3]));
						can->setClientSize(size);
	               Point2I res = can->getClientSize();
   	            console->printf("Mode: %d x %d", res.x, res.y);
						return 0;
	           	}
   		console->printf("setWindowSize: simCanvasName width height");
			break;

      case IsGfxDriver:
         if (argc != 3) {
            console->printf("Incorrect syntax for %s(CanvasName, DriverName)", argv[0]);
            return 0;
         }
         if ( SimObject* obj = manager->findObject(argv[1]) )
         {
            SimCanvas *pc = dynamic_cast<SimCanvas*>(obj);
            if (pc->isGfxDriver(argv[2]) == true) {
               return "True";
            } else {
               return "False";
            }
         } else {
            console->printf("Could not find simCanvas: %s", argv[1]);
            return "False";
         }
	}
	// Return error
	return "False";
}

void GFXPlugin::endFrame()
{
	// Update console variables.
   GFXMetricsInfo *gm = deviceManager.getMetrics();
   if(gm && CMDConsole::updateMetrics)
   {
   	CMDConsole* console = CMDConsole::getLocked();
   	console->setIntVariable("GFXMetrics::EmittedPolys",
   		gm->emittedPolys);
   	console->setIntVariable("GFXMetrics::RenderedPolys",
   		gm->renderedPolys);
   	console->setIntVariable("GFXMetrics::RecachedPolys",
   		gm->recachedSurfaces);
   	console->setIntVariable("GFXMetrics::BackfacedPolys",
   		gm->backfacedPolys);
   	console->setIntVariable("GFXMetrics::UsedEdges",
   		gm->numEdgesUsed);
   	console->setIntVariable("GFXMetrics::SharedEdges",
   		gm->numSharedEdges);
   	console->setIntVariable("GFXMetrics::UsedSpans",
   		gm->numSpansUsed);
   	console->setIntVariable("GFXMetrics::pixelsRendered",
   		gm->numPixelsRendered);
   	console->setIntVariable("GFXMetrics::textureSpaceUsed",
   		gm->textureSpaceUsed);
   	console->setIntVariable("GFXMetrics::textureBytesDownloaded",
   		gm->textureBytesDownloaded);
   	console->setIntVariable("GFXMetrics::numTexturesDownloaded",
   		gm->numTexturesDownloaded);
   	console->setIntVariable("GFXMetrics::numTexturesReleased",
   		gm->numReleasedTextures);
   	console->setIntVariable("GFXMetrics::numTexturesCreated",
   		gm->numCreatedTextures);
   	console->setIntVariable("GFXMetrics::numLightMaps",
   		gm->numLightMaps);
   	console->setIntVariable("GFXMetrics::lightMapsInUse",
   		gm->lightMapsInUse);
   	console->setIntVariable("GFXMetrics::numTMU0Wraps",
   		gm->numTMU0Wraps);
   	console->setIntVariable("GFXMetrics::numTMU1Wraps",
   		gm->numTMU1Wraps);
   	console->setIntVariable("GFXMetrics::numPaletteDLs",
   		gm->numPaletteDLs);
   }

   GFXPrefInfo* gp;
   if (m_fullyInitialized == false && (gp = deviceManager.getPrefs()) != NULL) {
	   // Update console variables.
      console->addVariable(0, "pref::useBansheeOverV2", CMDConsole::Bool, &gp->useBanshee);
      console->addVariable(0, "pref::waitForVSync",     CMDConsole::Bool, &gp->waitForVSync);
      console->addVariable(0, "useLowRes3D",            CMDConsole::Bool, &gp->useLowRes3D);
      m_fullyInitialized = true;
   }
}


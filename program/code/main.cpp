//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define INITGUID
#include "objbase.h"
#include <ddraw.h>
#include <g_ddraw.h>

#include <std.h>
#include <signal.h>
#include "simConsolePlugin.h"
#include "gfxPlugin.h"
#include "interiorPlugin.h"
#include "TerrainPlugin.h"
#include "skyPlugin.h"
#include "netPlugin.h"
#include "soundFXPlugin.h"
#include "preferencePlugin.h"
#include "simGuiPlugin.h"
#include "simInputPlugin.h"
#include "simTreePlugin.h"
#include "MissionPlugin.h"
#include "tedPlugin.h"
#include "shellOpenPlugin.h"
#include "simShapePlugin.h"
#include "LSPlugin.h"
#include "RedBookPlugin.h"
#include "aiPlugin.h"
#include "fearMissionPlugin.h"

#include "FearPlugin.h"
#include "esfPlugin.h"
#include "simTelnetPlugin.h"
#include "simWinConsolePlugin.h"
#include <feardynamicdataplugin.h>
#include "simGame.h"
#include "fearglobals.h"
#include "netPacketStream.h"
#include "FearCSDelegate.h"
#include "simCanvas.h"
#include "aiObj.h"
#include <scriptplugin.h>
#include <fearplayerpsc.h>
#include <cpuSpeed.h>
#include "simguicanvas.h"
#include "ircPlugin.h"

// for detecting DirectX requirements
typedef HRESULT (WINAPI *DIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);

WorldGlobals cg;
WorldGlobals sg;
WorldGlobals *wg;

extern "C" {
void __cdecl UnloadGlide();
};

void __cdecl onHeinousError(int)
{
   // Not really much else we can do -- perhaps some cleanup,
   // but at this point the stack can be pretty trashed
   UnloadGlide();

   exit(-1);
}

//----------------------------------------------------------------------------

class FearClientWorld : public SimWorld
{

};

class FearGame: public SimGame
{
public:   
   void init();
   void clientProcess();
   void serverProcess();
   const char *consoleCallback(CMDConsole *console, int id, int argc, const char *argv[]);
};   

class FearMain: public GWMain
{
   HANDLE hSem;
	FearGame *game;
public:
	void onIdle();
	void onExit();
	bool initInstance();
   bool executeConsoleCS();
} Main;

extern "C"
{
   int m_stdval;
};

namespace SimGui {
extern bool g_prefPoliteGui;
}

void FearGame::init()
{
   m_stdval = 0;
#ifndef DEBUG
   signal(SIGSEGV, onHeinousError);
   signal(SIGABRT, onHeinousError);
   signal(SIGFPE,  onHeinousError);
   signal(SIGILL,  onHeinousError);
   signal(SIGTERM, onHeinousError);
#endif

   SimGame::init();
   sg.resManager = resourceObject.get();
   cg.resManager = resourceObject.get();

   SimGui::Canvas::canvasCursorTrapped = true;
   SimGui::g_prefPoliteGui = true;

	// Install all the plugins
	registerPlugin(new GFXPlugin);
	//registerPlugin(new InteriorPlugin);
	registerPlugin(new TerrainPlugin);
	registerPlugin(new LSPlugin);
	//registerPlugin(new SkyPlugin);
   registerPlugin(new NetPlugin );
   registerPlugin(new SoundFXPlugin );
   //registerPlugin(new PreferencePlugin );
   registerPlugin(new SimInputPlugin );
   registerPlugin(new ToolPlugin );
   registerPlugin(new SimTreePlugin );
   //registerPlugin(new MissionPlugin );
   registerPlugin(new SimGui::Plugin );
	registerPlugin(new TedPlugin );
   registerPlugin(new ShellOpenPlugin );
   registerPlugin(new RedbookPlugin );

   //registerPlugin(new SimShapePlugin);

	registerPlugin(new FearPlugin);
	//registerPlugin(new ESFPlugin);
   registerPlugin(new SimTelnetPlugin );
   registerPlugin(new SimWinConsolePlugin );
   
   registerPlugin(new FearDynamicDataPlugin);
   registerPlugin(new ScriptPlugin);
   registerPlugin(new AIPlugin);
   registerPlugin(new FearMissionPlugin );
   registerPlugin(new IRCPlugin);
   
}

//----------------------------------------------------------------------------
// FearMain: The actual application
//----------------------------------------------------------------------------
bool FearMain::executeConsoleCS()
{
   FileRStream consoleCS("console.cs");
   if(consoleCS.getStatus() != STRM_OK)
      return false;

   int size = consoleCS.getSize();
   BYTE *buffer = new BYTE[size+1];
   consoleCS.read(size, buffer);
   buffer[size] = 0;
   Console->evaluate((const char *)buffer, false, "console.cs");
   delete[] buffer;
   return true;
}

bool FearMain::initInstance()
{
	// override tolerance setting for shadows... avoids 'shadow in sky' bug.
	TS::Shadow::tol5 = 0.0025;

   hSem = CreateSemaphore(NULL, 0, 100, "TRIBES_LOCK");

   Net::PacketStream::addToTimerSet = false;
   checkFocus = false;
	game = new FearGame;

	// Set search path to include startup dir.
   char exePath[512];
   char searchPath[1024];
   strcpy(searchPath, ".");
   if (GetModuleFileName(GetModuleHandle(NULL), exePath, 512)) {
      // extract path of .exe from filename
      char *c = strrchr(exePath, '\\');
      if (c)
         *c = '\0';
      // add the path of the .exe to the searchpath
      strcat(searchPath, ";");
      strcat(searchPath, exePath);
   }

   setCPUConsoleVars();

   game->setVariable("CommandLinePath", searchPath);
	game->init();

   cg.resManager->setOutsideSearch(false);
   cg.resManager->setWriteablePath("recordings;temp;config;base\\missions;base\\recordings;base\\gui");
	// Evaluate the default console script
   char varName[256];

   int argc = 0;
   Vector<const char *>::iterator i;   
   for(i = GWMain::processedCmdLine.begin(); i != GWMain::processedCmdLine.end(); i++)
   {
      sprintf(varName, "$cargv%d", argc + 1);
      argc++;
      game->setVariable(varName, *i);
   }
   game->setIntVariable("$cargc", argc);

   executeConsoleCS();
   //--------------------------------------
   OSVERSIONINFO osInfo;
   osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&osInfo);
   if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
   {
      if (osInfo.dwMajorVersion == 4)
      {
         const char * sp = osInfo.szCSDVersion;
         if (*sp)
         {
            const char *spNumStr = strstr(sp,"Service Pack ");
            if (spNumStr && atoi(spNumStr+strlen("Service Pack ")) >= 3)
               return (true);
         }

         if (MessageBox(0, "Tribes requires WinNT 4 with Service Pack 3 or\n"
                           "greater installed.  Proceed at your own risk.\n\n"
                           "Quit now?", 
                           "Error: Required Service Pack is Missing.", MB_YESNO|MB_ICONHAND|MB_DEFBUTTON1 ) == IDYES)
            exit(1);
      }
      else 
      if (osInfo.dwMajorVersion < 4)
      {
         if (MessageBox(0, "Tribes requires WinNT 4 or greater.\n"
                       "Proceed at your own risk.\n\n"
                       "Quit now?", 
                       "Error: Old version of WinNT detected.", MB_YESNO|MB_ICONHAND|MB_DEFBUTTON1 ) == IDYES)
            exit(1);
      }
   }
   else 
   if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
   {
      if (osInfo.dwMajorVersion == 4)
      {
         if (osInfo.dwMinorVersion == 0)  // win 95
         {
            if (GFXDDSurface::GetDD2() != NULL || Console->getBoolVariable("Dedicated"))
					// If we have a surface, everything should be ok.
               return true;

            bool bail = (MessageBox(0, "Tribes requires DirectX5. Either DirectX5 is\n"
                       "not installed or an old verion of DirectX was detected.n\n"
                       "Proceed at your own risk.\n\n"
                       "Quit now?", 
                       "Error: DirextX5 not installed.", MB_YESNO|MB_ICONHAND|MB_DEFBUTTON1 ) == IDYES);
            if (bail)
               exit(1);
         }
      }
      else 
      if (osInfo.dwMajorVersion < 4)
      {
         if (MessageBox(0, "Tribes requires Win95 or greater.\n"
                       "Proceed at your own risk.\n\n"
                       "Quit now?", 
                       "Error: Old version of Windows detected.", MB_YESNO|MB_ICONHAND|MB_DEFBUTTON1 ) == IDYES)
            exit(1);
      }
   }

	return true;
}

//----------------------------------------------------------------------------

const char *FearGame::consoleCallback(CMDConsole *console, int id, int argc, const char *argv[])
{
   WorldType w = SERVER;

   switch(id)
   {
      case NewClient:
         w = CLIENT;
      case NewServer:
         const char *ret;
         ret = SimGame::consoleCallback(console, id, argc, argv);
         SimManager *mgr;
         mgr = world[w]->getManager();
         if(id == NewClient)
         {
            cg.manager = mgr;
            cg.currentTime = DWORD(getTime() * 1000) & ~0x1F;
            cg.timeBase = cg.currentTime;
         }
         else
         {
            sg.manager = mgr;
            sg.currentTime = DWORD(getTime() * 1000) & ~0x1F;
            sg.timeBase = sg.currentTime;
         }
         mgr->addObject(new SimSet(false), MoveableSetId);
         mgr->addObject(new SimSet(false), PlayerSetId);
         mgr->addObject(new SimSet(false), TargetableSetId);
         mgr->addObject(new SimSet(false), ObserverSetId);
         return ret;
      case DeleteServer:
         sg.manager = NULL;
         break;
      case DeleteClient:
         sg.manager = NULL;
         break;
      case FocusServer:
         if(sg.manager)
            wg = &sg;
         break;
      case FocusClient:
         wg = &cg;
         break;
   }
   return SimGame::consoleCallback(console, id, argc, argv);
}

void FearGame::clientProcess()
{
   if(!world[CLIENT])
      return;

   SimManager *cManager = world[CLIENT]->getManager();
   manager = cManager;

   wg = &cg;

   int i;
   for (i = 0; i < plugins.size(); i++)
	   plugins[i]->setManager( cManager );

   // process input and sound
   inputManager->process();
   
	SimSet* grp = static_cast<SimSet*>(cManager->findObject(SimFrameEndNotifySetId)); 
   SimFrameEndNotifyEvent event;
   grp->processEvent(&event);

   cg.lastTime = cg.currentTime;
   double curTime = getTime();
   cg.currentTime = curTime * 1000;
   Net::PacketStream::currentTime = cg.currentTime;

   if(cg.psc)
      cg.psc->clientCollectInput(cg.lastTime, cg.currentTime);

   // process network events
   if(cg.csDelegate)
      cg.csDelegate->process();
   if(cg.packetStream)
      cg.packetStream->processRecorder(cg.currentTime);
   if(cg.packetStream)
      cg.packetStream->checkPacketSend();

   curTime = (cg.currentTime - cg.timeBase) * 0.001;
   cg.clockTime += curTime - cManager->getCurrentTime();
   cManager->advanceToTime(curTime);

   SimSet *set = (SimSet *) cManager->findObject(MoveableSetId);
   int s;

   for(s = 0; s < set->size(); s++)
   {
      GameBase *b = static_cast<GameBase *>((*set)[s]);
      if(b->getLastProcessTime() != cg.currentTime)
         b->clientProcess(cg.currentTime);
   }
   set = (SimSet *) cManager->findObject(PlayerSetId);
   for(s = 0; s < set->size(); s++)
   {
      GameBase *b = static_cast<GameBase *>((*set)[s]);
      if(b->getLastProcessTime() != cg.currentTime)
         b->clientProcess(cg.currentTime);
   }
   set = (SimSet *) cManager->findObject(ObserverSetId);
   for(s = 0; s < set->size(); s++)
   {
      GameBase *b = static_cast<GameBase *>((*set)[s]);
      if(b->getLastProcessTime() != cg.currentTime)
         b->clientProcess(cg.currentTime);
   }

	Sfx::Manager *sfx = Sfx::Manager::find(manager);
   if(sfx && cg.psc)
   {
	   GameBase *fo = cg.psc->getControlObject();
      if(fo)
	      sfx->setListenerTransform(fo->getEyeTransform(), fo->getLinearVelocity());
   }

   SimCanvasSet* cgrp = static_cast<SimCanvasSet*>
		(cManager->findObject(SimCanvasSetId));

   cgrp->render();
}

void FearGame::serverProcess()
{
   if(!world[SERVER])
      return;

   SimManager *sManager = world[SERVER]->getManager();
   manager = sManager;

   int i;
   wg = &sg;

   for (i = 0; i < plugins.size(); i++)
	   plugins[i]->setManager( sManager );

   // server always increments time in 32 ms chunks

   double curTime = getTime();
   sg.lastTime = sg.currentTime;
   DWORD finalTime = DWORD(curTime * 1000) & ~0x1F;

   if(sg.csDelegate)
      sg.csDelegate->process();

   while(sg.currentTime < finalTime)
   {
      sg.lastTime = sg.currentTime;
      sg.currentTime += 32;

      Console->dbprintf(4, "TIME: %d", sg.lastTime >> 5);
      curTime = (sg.currentTime - sg.timeBase) * 0.001;
      sManager->advanceToTime(curTime);

      int s;

      SimSet *set = (SimSet *) sManager->findObject(MoveableSetId);
      for(s = 0; s < set->size(); s++)
      {
         GameBase *b = static_cast<GameBase *>((*set)[s]);
         b->serverProcess(sg.currentTime);
      }
      
      if( AIManager * aim = (AIManager *) sManager->findObject(AIManagerId) )
         aim->doServerProcesses( sg.currentTime );
         
      set = (SimSet *) sManager->findObject(PlayerSetId);
      for(s = 0; s < set->size(); s++)
      {
         GameBase *b = static_cast<GameBase *>((*set)[s]);
         b->serverProcess(sg.currentTime);
      }
      set = (SimSet *) sManager->findObject(ObserverSetId);
      for(s = 0; s < set->size(); s++)
      {
         GameBase *b = static_cast<GameBase *>((*set)[s]);
         b->serverProcess(sg.currentTime);
      }
   }

   SimSet *psset = (SimSet *) sManager->findObject(PacketStreamSetId);
   SimSet::iterator ps;
   for(ps = psset->begin(); ps != psset->end(); ps++)
      ((Net::PacketStream *) (*ps))->checkPacketSend();
}

void FearMain::onIdle()
{
   game->startFrame();
   game->serverProcess();
   if(cg.manager)
      Sleep(0);
   else
      Sleep(1);
   game->clientProcess();
   game->endFrame();
}

//----------------------------------------------------------------------------
// helper function for FearMain::onExit()
void checkHeap()
{
#ifdef _MSC_VER
   int hc = _heapchk();
   AssertFatal((hc == _HEAPOK) || (hc == _HEAPEMPTY), "Heap error.");
#else
   int hc = heapcheck();
   AssertFatal(hc != _HEAPCORRUPT, "Heap error.");
#endif
}

//----------------------------------------------------------------------------

void FearMain::onExit()
{
   checkHeap();

	// Evaluate the default onExit script
   game->executef(3, "GuiLoadContentCtrl", "MainWindow", "gui\\quit.gui");
	game->evaluate("onExit();",false);

   checkHeap();
   
	delete game;
   CloseHandle(hSem);

   checkHeap();
}

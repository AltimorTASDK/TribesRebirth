//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define INITGUID
#include "objbase.h"
#include <ddraw.h>
#include <g_ddraw.h>

#include "simconsoleplugin.h"
#include "sim.h"
#include "gfxPlugin.h"
#include "TerrainPlugin.h"
#include "LSPlugin.h"
#include "skyPlugin.h"
#include "netPlugin.h"
#include "soundFXPlugin.h"
#include "soundfx.h"
#include "simGuiPlugin.h"
#include "simInputPlugin.h"
#include "toolPlugin.h"
#include "simTreePlugin.h"
#include "MissionPlugin.h"
#include "tedPlugin.h"
#include "shellOpenPlugin.h"
#include "simgame.h"
#include "simWinConsolePlugin.h"

//#include "interiorPlugin.h"
//#include "preferencePlugin.h"

//----------------------------------------------------------------------------
class MainGame: public SimGame
{
public:   
   void init();
};   


void MainGame::init()
{
   SimGame::init();

	// Install all the plugins
	registerPlugin(new GFXPlugin);
	registerPlugin(new TerrainPlugin);
	registerPlugin(new LSPlugin);
   registerPlugin(new NetPlugin);
   registerPlugin(new SoundFXPlugin);
   registerPlugin(new SimInputPlugin);
   registerPlugin(new ToolPlugin);
   registerPlugin(new SimTreePlugin);
   registerPlugin(new SimGui::Plugin);
	registerPlugin(new TedPlugin);
   registerPlugin(new ShellOpenPlugin);
   registerPlugin(new SimConsolePlugin);
   registerPlugin(new SimWinConsolePlugin );

//   registerPlugin(new MissionPlugin);
//   registerPlugin(new PreferencePlugin);
//	registerPlugin(new SkyPlugin);
//	registerPlugin(new InteriorPlugin);
}

//----------------------------------------------------------------------------
class CMain: public GWMain
{
	MainGame *game;
    double      frameLast;
    Timer       timer;

public:
	void onIdle();
	void onExit();
	bool initInstance();
} Main;



//----------------------------------------------------------------------------
// CMain: The actual application
//----------------------------------------------------------------------------
bool CMain::initInstance()
{
   game = new MainGame;
   game->init();

	// Set search path to include startup dir.
   char exePath[512];
   char searchPath[1024];
   strcpy(searchPath, ".");
   if (GetModuleFileName(GetModuleHandle(NULL), exePath, 512))
   {
      // extract path of .exe from filename
      char *c = strrchr(exePath, '\\');
      if (c)
         *c = '\0';
      // add the path of the .exe to the searchpath
      strcat(searchPath, ";");
      strcat(searchPath, exePath);
   }
   game->setVariable("CommandLinePath", searchPath);
   
   // set the search path...
   if( ::GetFileAttributes( "prefs.cs" ) == 0xffffffff )
      game->setVariable("ConsoleWorld::DefaultSearchPath", searchPath );
   else
      game->setVariable("ConsoleWorld::DefaultSearchPath", "." );

   // get the prefs ( the path and palette filename )         
   game->evaluate("exec( \"prefs.cs\" );", false );
   
   // tack on the exepath ( could already be there - no harm )
   sprintf( searchPath, "%s;%s", 
      game->getVariable( "ConsoleWorld::DefaultSearchPath" ), exePath );
   game->setVariable( "ConsoleWorld::DefaultSearchPath", searchPath );
   
   // Evaluate the default console script
   char consoleRC[500];
   game->evaluate("exec( \"console.cs\" );", false );
   sprintf( consoleRC, "load( \"%s\" );", cmdLine );
   game->evaluate( consoleRC, false );
   return true;
}


//----------------------------------------------------------------------------

void CMain::onIdle()
{
    game->startFrame();
	
    double frameStart = timer.getElapsed();
    game->advanceToTime(game->getTime());
    frameLast = frameStart;            
   
    game->render();
    game->endFrame();
}


//----------------------------------------------------------------------------

void CMain::onExit()
{
   delete game;
}

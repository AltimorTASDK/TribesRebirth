#define INITGUID
#include <std.h>
#include "simConsolePlugin.h"
#include "simTreePlugin.h"
#include "g_surfac.h"
#include "gfxPlugin.h"
#include "terrainPlugin.h"
#include "LSPlugin.h"
// #include "interiorPlugin.h"
// #include "skyPlugin.h"
#include "soundFXPlugin.h"
#include "toolPlugin.h"
#include "simInputPlugin.h"
#include "simContainer.h"
#include "simWinConsolePlugin.h"
#include "simGuiPlugin.h"
#include "simGuiCanvas.h"
#include "simWorld.h"
#include "simGame.h"
#include "winTools.h"
#include "resource.h"
#include <ia3d.h>
#include <direct.h>
#include <winuser.h>
#include "showPlugin.h"

// Bogus default game GUID
static GUID defaultGuid = { 0xaaaaaaaa, 0xe610, 0x11cf, { 0xb2, 0xa9, 0x44, 0x45, 0x53, 0x54, 0x0, 0x1 } };
    
HMENU       hMenu;
HINSTANCE   theHInstance;
extern bool cameraAttached;

// MyCanvas class.  We need to derive so that we can intercept
// menu bar callback functions and process onCommand messages
class MyCanvas : public SimGui::Canvas
{
      typedef SimGui::Canvas Parent;
   
   public :
      virtual void onInitMenu(HMENU);
      virtual void onCommand (int, HWND, UINT);

      DECLARE_PERSISTENT(MyCanvas);
};

IMPLEMENT_PERSISTENT(MyCanvas);

// Set up our menu to be enabled/disabled as necessary at startup
void MyCanvas::onInitMenu(HMENU hMenu)
{
   MENUITEMINFO info;

   info.cbSize = sizeof(info);
   info.fMask  = MIIM_STATE;
   info.fState = cameraAttached ? MFS_ENABLED : MFS_DISABLED;

   SetMenuItemInfo(hMenu,  IDM_FILE_SAVE_SHAPE,             false,   &info);
   SetMenuItemInfo(hMenu,  IDM_FILE_SAVE_SHAPE_AS,          false,   &info);
   SetMenuItemInfo(hMenu,  IDM_FILE_IMPORT_SEQUENCE,        false,   &info);
   SetMenuItemInfo(hMenu,  IDM_FILE_IMPORT_FIRST_SEQUENCE,  false,   &info);
   SetMenuItemInfo(hMenu,  IDM_FILE_IMPORT_TRANSITIONS,     false,   &info);
   SetMenuItemInfo(hMenu,  IDM_WINDOW_SHAPE,                false,   &info);
   SetMenuItemInfo(hMenu,  IDM_WINDOW_CAMERA,               false,   &info);
   SetMenuItemInfo(hMenu,  IDM_WINDOW_TRANSITION,           false,   &info);
}

void MyCanvas::onCommand(int id, HWND hWndCtl, UINT codeNotify)
{
   CMDConsole     *console;
   char           *cmdArg[2];
   FileInfo       fi;

   if (codeNotify != 0 && codeNotify != 1)
      return;

   console = CMDConsole::getLocked();

   switch (id)
   {
      case IDM_FILE_OPEN_SHAPE :
         cmdArg[0] = "viewShape";
         console->execute(1, (const char **)cmdArg);
         break;

      case IDM_FILE_SAVE_SHAPE :
         cmdArg[0] = "saveShape";
         console->execute(1, (const char **)cmdArg);
         break;

      case IDM_FILE_SAVE_SHAPE_AS :
         cmdArg[0] = "saveShapeAs";
         console->execute(1, (const char **)cmdArg);
         break;

      case IDM_FILE_IMPORT_SEQUENCE :
         cmdArg[0] = "impSeq";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_FILE_IMPORT_FIRST_SEQUENCE :
         cmdArg[0] = "impSeq1";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_FILE_IMPORT_TRANSITIONS :
         cmdArg[0] = "imptrans";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_FILE_LOAD_PALETTE :
         cmdArg[0] = "loadpal";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_FILE_EXIT :
         cmdArg[0] = "quit";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_WINDOW_SHAPE :
         cmdArg[0] = "shapeDialog";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_WINDOW_CAMERA :
         cmdArg[0] = "camDialog";
         console->execute(1, (const char **)cmdArg);
         break;
      
      case IDM_WINDOW_TRANSITION :
         break;
      
      case IDM_WINDOW_TERRAIN :
         cmdArg[0] = "hideToolWin";
         cmdArg[1] = "Terrains";
         console->execute(2, (const char **)cmdArg);
         cmdArg[0] = "showToolWin";
         cmdArg[1] = "Terrains";
         console->execute(2, (const char **)cmdArg);
         break;
      
      default:
         Parent::onCommand(id, hWndCtl, codeNotify);
         break;
   }
}

class ExampleGame: public SimGame
{
      void clientProcess();
   public:   
      void init(const char *startup);
};   


void ExampleGame::clientProcess()
{
   // process input and sound
   // inputManager->process();
}



void ExampleGame::init(const char *startup)
{
   SimGame::init();

   // Install all the plugins
   registerPlugin(new SimTreePlugin);
   registerPlugin(new GFXPlugin);
   // registerPlugin(new InteriorPlugin);
   // registerPlugin(new SkyPlugin);
   registerPlugin(new TerrainPlugin);
   registerPlugin(new LSPlugin);
   registerPlugin(new SoundFXPlugin);
   registerPlugin(new ToolPlugin);
   registerPlugin(new showPlugin);
   registerPlugin(new SimInputPlugin);
   registerPlugin(new SimGui::Plugin);
    
   registerPlugin(new SimWinConsolePlugin );

   cameraAttached = false;
   
   if (startup)
      evaluate( startup, true );
}


//----------------------------------------------------------------------------

class CMain: public GWMain
{
   ExampleGame *game;
   SimWorld    *client;

 public:
   void render() 
   { 
       game->render(); 
   }
   
   void onIdle();
   bool initInstance();
   void onExit();
   void parseCommandLine(char *);
   void exec(char *);
   
} Main;

void hackRender()
{
   Main.render();
}

// ainb resides in showplugin.cpp
extern bool ainb(char *, char *);

// Function to handle single word on cmd line.  
void CMain::exec(char *cmdStr)
{
   // return;

   char *ch, *oldDefaultPath, *newSearchPath, evalStr[120];

   // If we have a path name, temporarily add it to default path
   ch             = strrchr(cmdStr, '\\');
   oldDefaultPath = new char[strlen(game->getVariable("ConsoleWorld::DefaultSearchPath")) + 1];

   strcpy(oldDefaultPath, game->getVariable("ConsoleWorld::DefaultSearchPath"));

   if (ch && *ch == '\\')
   {
       newSearchPath = new char[50 + strlen(oldDefaultPath)];
       *ch='\0';
       
       strcpy(newSearchPath, cmdStr);
       strcat(newSearchPath, ";");
       *ch='\\';

       strcat(newSearchPath, oldDefaultPath);
       
       game->setVariable("ConsoleWorld::DefaultSearchPath", newSearchPath);

       delete [] newSearchPath;
   }

   // Load .cs file, palette, or .dts shape file
   if (ainb(".ppl", cmdStr))
   {
      sprintf ( evalStr, "loadPalette ( \"%s\" );", cmdStr );
      game->evaluate ( evalStr, false );
   }

   else if( ainb( ".dts", cmdStr ) )
   {
      sprintf ( evalStr, "view ( \"%s\" );", cmdStr );
      game->evaluate ( evalStr, false );
   }
   
   // Assume .cs file
   else
   {
      // if ( ! ainb ( ".cs", cmdStr ) )
      //    Add .cs ?  (Extension is added for you)

      sprintf ( evalStr, "exec ( \"%s\" );", cmdStr );

      game->evaluate ( evalStr, false );
   }

   // Return to old default search path
   game->setVariable("ConsoleWorld::DefaultSearchPath", oldDefaultPath);

   delete [] oldDefaultPath;
}

void CMain::parseCommandLine(char *cl)
{
   char *tmpStr;

   while ((tmpStr = strrchr(cl, ' ')) != 0)
   {
      tmpStr[0] = '\0';

      if (tmpStr[1] != '\0')
      {
          exec(tmpStr + 1);
      }
   }

   if (cl[0] != '\0')
   {
      exec(cl);
   }
}

bool CMain::initInstance()
{
   char exePath[512], searchPath[1024], consoleRC[24];

   AssertSetFlags(ASSERT_NO_WARNING);

   game = new ExampleGame;

   // Set the game GUID
   game->setGameGuid(defaultGuid);

   // Set search path to include startup dir.
   strcpy(searchPath, ".");

   if (GetModuleFileName(GetModuleHandle(NULL), exePath, 512)) 
   {
      // Extract path of .exe from filename
      char *c = strrchr(exePath, '\\');
      
      if (c)
         *c = '\0';

      // Make this the current directory
      int err = chdir(exePath);
    
      AssertFatal(!err,"Error changing working directory");
      
      // Add the path of the .exe to the searchpath
      strcat(searchPath, ";");
      strcat(searchPath, exePath);
   }

   // Create a client
   client = new SimWorld;

    // client->getManager()->addObject(new SimDefaultContainer(SimRootContainerId));
   client->getManager()->addObject(new SimDefaultOpenContainer(SimRootContainerId));

   game->setWorld(SimGame::CLIENT, client);
   game->focus(SimGame::CLIENT);
   game->setVariable("CommandLinePath", searchPath);
   game->init(NULL);

   // Evaluate the default console script
   strcpy(consoleRC, "exec(\"show2.cs\");");
   // strcat(consoleRC, cmdLine);
   game->evaluate(consoleRC, true );

   // Now that we have initialized the world, parse the command line
   parseCommandLine(CMain::cmdLine);
   
   return true;
}

void CMain::onIdle()
{
   game->startFrame();
   game->advanceToTime(game->getTime());

   Sleep(0);

   game->render();
   game->endFrame();
}

void CMain::onExit()
{
   delete game;
}

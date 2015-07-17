//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGAME_H_
#define _SIMGAME_H_


#include <console.h>
#include <sim.h>
#include <timer.h>
#include "simWorld.h"
#include "simResource.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimConsolePlugin;
class SimActionMap;
class SimGuiConsolePlugin;

#define SimGameActionMapName "SimGame.sae"

//------------------------------------------------------------------------------
class SimGame: public CMDConsole, public SimObject
{
public:
   enum WorldType
   {
      FOCUSED = 0,
      SERVER,
      CLIENT,        // could easily add 'n' clients :) !!!
      CLIENT2,
      N_WORLDS
   };

   static bool isPlayback;
   static bool isRecording;

protected:
   SimGuiConsolePlugin *sgcp;
   static SimGame *theGame;
   SimWorld    *world[N_WORLDS];
   SimManager  *manager;
   SimResource resourceObject;
   SimInputManager *inputManager;
   Resource<SimActionMap> gameActionMap;

   GUID guid;
   Vector<SimConsolePlugin*> plugins;
   VectorPtr<ActionName*> actionList;
   char* buildPath(SimObject *object, char *path);
   
   char buffer[128]; // for returning strings

   enum CallbackID
   {
      // Vars.
      DefaultSearchPath = 100,      //must start after internal CMDConsole id's
      TimeScale,
      // Commands
      LoadObject,
      StoreObject,
      DeleteObject,
      IsObject,
      IsFile,
      LoadPalette,
      LoadTagDictionary,
      LoadVolume,
      ListVolumes,
      ListObjects,
      NewCanvas,
      NewCamera,
      NewSet,
      NewObject,
      AddToSet,
      PostAction,
      Focus,
      Unfocus,
      PurgeResources,
      FocusClient,
      FocusServer,
      NewClient,
      NewServer,
      DeleteClient,
      DeleteServer,
      RemoteEval,
      InspectObject,
      RenameObject,
      GetSimTime,
      SetObjectId,
      Schedule,
      SetWindowTitle,
      GetPathOf,
      GetGroup,
      IsMember,
      GetNextObject,
      MAX_SIMGAME_CONSOLE_ID,
   };
   const char *consoleCallback(CMDConsole*,int id,int argc, const char *argv[]);

   Timer timer;
   double frameStart;
   double absFrameStart;
   double avgSPF;
   double timeOffset;
   double timeScale;
   int    fpsDelay;

public:
   SimGame();   
   ~SimGame();   

   void  setGameGuid(const GUID &gameGuid);
   GUID& getGameGuid();

   double getTime();

   void init();
   SimGuiConsolePlugin *getConsole();

   virtual void startFrame();
   virtual void endFrame();
   virtual void advanceToTime(double delta);
   virtual void render();

   //-------------------------------------- 
   void registerPlugin(SimConsolePlugin* pl);
   void registerActionList( ActionName *, int numEntries);

   void        focus(SimManager *manager);
   void        focus(WorldType w);
   void        setWorld(WorldType w, SimWorld *world);
   SimWorld*   getWorld(WorldType w=FOCUSED);
   SimManager* getManager(WorldType w=FOCUSED);
   SimInputManager* getInputManager();
   void        destroy();

   const char* parseManagerName(const char* name,SimManager** manager);
   void        setDefaultSearchPath( const char *name, const char *ext=NULL );

   static SimGame* get();
   void setTime(double newTime);
   void forceSetTime(double newTime);
   bool processQuery(SimQuery*);
   bool processEvent(const SimEvent *evt);
};   



//------------------------------------------------------------------------------
inline double SimGame::getTime()
{
   return ( frameStart );   
}


inline GUID& SimGame::getGameGuid()
{
   return ( guid );
}


inline SimInputManager* SimGame::getInputManager()
{
   return (inputManager);      
}   


//------------------------------------------------------------------------------
//Default sim actions for use with generic plugins
//User actions should start at ID = 0

enum GameAction
{ 
   None       = ApplicationActions,
   Pause,         
   Fire,      
   StartFire,     
   EndFire, 
   SelectWeapon, 
   MoveYaw,       
   MovePitch, 
   MoveRoll,      
   MoveForward,
   MoveBackward, 
   MoveLeft,      
   MoveRight, 
   MoveUp,        
   MoveDown,
   ViewYaw,      
   ViewPitch,     
   ViewRoll,  
   IncreaseSpeed, 
   DecreaseSpeed,
   SetSpeed,     
   SetDetailLevel, 
   MoveX,
   MoveY,
   MoveZ,
   MoveR,
   MoveU,
   MoveV,
   Button0,
   Button1,
   Button2,
   Button3,
};



#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif //_SIMGAME_H_

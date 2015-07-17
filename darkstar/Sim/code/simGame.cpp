// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <g_bitmap.h>
#include <sim.h>
#include <simGame.h>
#include <inspectWindow.h>
#include "g_surfac.h"
#include "simTagDictionary.h"
#include "simVolume.h"
#include "simPalette.h"
#include "simPersistTags.h"
#include "simConsoleEvent.h"
#include "simConsolePlugin.h"
#include "simCanvas.h"
#include <simguiconsoleplugin.h>

SimGame* SimGame::theGame = NULL;
bool SimGame::isPlayback = false;
bool SimGame::isRecording = false;

#define SimConsoleEvalVar  "ConsoleWorld::Eval"


namespace SimObjects
{
extern int dummy_link;
};

IMPLEMENT_PERSISTENT_TAGS( SimPalette, FOURCC ('S','p', 'p','l' ), SimPalettePersTag );

static ActionName defaultActionTable[] ={ 

   // Generic Sim actions
   { ActionNext,     "Next" },
   { ActionPrevious, "Previous" },
   { ActionAttach,   "Attach" },

   // Application actions
   { None,           "None" },         
   { Pause,          "Pause" },         
   { Fire,           "Fire" },      
   { StartFire,      "StartFire" },     
   { EndFire,        "EndFire" }, 
   { SelectWeapon,   "SelectWeapon" }, 
   { MoveYaw,        "MoveYaw" },       
   { MovePitch,      "MovePitch" }, 
   { MoveRoll,       "MoveRoll" },      
   { MoveForward,    "MoveForward" },
   { MoveBackward,   "MoveBackward" }, 
   { MoveLeft,       "MoveLeft" },      
   { MoveRight,      "MoveRight" }, 
   { MoveUp,         "MoveUp" },        
   { MoveDown,       "MoveDown" },
   { ViewYaw,        "ViewYaw" },      
   { ViewPitch,      "ViewPitch" },     
   { ViewRoll,       "ViewRoll" },  
   { IncreaseSpeed,  "IncreaseSpeed" }, 
   { DecreaseSpeed,  "DecreaseSpeed" },
   { SetSpeed,       "SetSpeed" },     
   { SetDetailLevel, "SetDetailLevel" },
   { MoveX,          "MoveX" },
   { MoveY,          "MoveY" },
   { MoveZ,          "MoveZ" },
   { MoveR,          "MoveR" },
   { MoveU,          "MoveU" },
   { MoveV,          "MoveV" },
   { Button0,        "Button0" },
   { Button1,        "Button1" },
   { Button2,        "Button2" },
   { Button3,        "Button3" },
};   

#define ActionListSize (sizeof(defaultActionTable) / sizeof(ActionName))


//------------------------------------------------------------------------------
SimGame::SimGame()
{
   SimObjects::dummy_link = 0;
   AssertFatal(theGame == NULL, "SimGame::SimGame: ONLY 1 SimGame instance allowed.");
   theGame    = this;
   manager    = NULL;   //currently focused manager (client OR server)
   frameStart = 0.0;
   avgSPF     = 0.0;
   timeOffset = 0.0;
   timeScale  = 1.0;
   fpsDelay   = 0;

   // Resource manager for console
   setResourceManager(resourceObject.get());
   // clear arrays
   for ( int w=FOCUSED; w < N_WORLDS; w++)  world[w] = NULL;

   registerActionList(defaultActionTable, ActionListSize);
   ResourceManager *rm = resourceObject.get();
   rm->add(ResourceType::typeof(".sae"), SimGameActionMapName, new SimActionMap, true);
   gameActionMap = rm->load(SimGameActionMapName);

   AssertFatal((bool)gameActionMap, "Ack!");

   inputManager = new SimInputManager;

   //resourceObject.get()->setSearchPath(".");
   sgcp = NULL;
}


//------------------------------------------------------------------------------
SimGame::~SimGame()
{
   destroy();
   theGame = NULL;
}


//------------------------------------------------------------------------------
// if you create a SIMGame on the global heap you should call destroy rather
// than letting the destructor clean up the resources because you cannot
// control the order of destruction of other global objects (ie resourceTypes etc).
void SimGame::destroy()
{
   // Delete the input manager
   SimManager *mgr = inputManager->getManager();
   if (inputManager && mgr)
   {
      mgr->lock();
      mgr->unregisterObject( inputManager );
      mgr->unlock();
   }
   delete inputManager;
   inputManager = NULL;

   // Delete the worlds
   for ( int w=SERVER; w < N_WORLDS; w++) 
      setWorld((WorldType) w, NULL);

   // Delete all the plugins.
   for (int i = 0; i < plugins.size(); i++)
      delete plugins[i];
   plugins.clear();

#ifdef _MSC_VER
   int hc = _heapchk();
   AssertFatal((hc == _HEAPOK) || (hc == _HEAPEMPTY), "Heap error.");
#else
   int hc = heapcheck();
   AssertFatal(hc != _HEAPCORRUPT, "Heap error.");
#endif

   resourceObject.get()->purge(true);

#ifdef _MSC_VER
   hc = _heapchk();
   AssertFatal((hc == _HEAPOK) || (hc == _HEAPEMPTY), "Heap error.");
#else
   hc = heapcheck();
   AssertFatal(hc != _HEAPCORRUPT, "Heap error.");
#endif
}   


//------------------------------------------------------------------------------
void SimGame::setGameGuid(const GUID &gameGuid)
{
   guid = gameGuid;   
}


//------------------------------------------------------------------------------

const char* SimGame::parseManagerName(const char* name,SimManager** manager)
{
   const char* p = name;
   while (*p && isspace(*p))
      ++p;
   if (isseperator(p[0]) && isseperator(p[1])) {
      p += 2;
      const char* ptr = p;
      while (*p && !isseperator(*p))
         ++p;
      if (!strnicmp(ptr,"server",p - ptr))
         *manager = getManager(SERVER);
      else
         // Client otherwise, even if name incorrect
         *manager = getManager(CLIENT);
      return p;
   }

   // Default is whatever was passed in or client
   if (!*manager)
      *manager = getManager(CLIENT);
   return p;
}   

//------------------------------------------------------------------------------

void SimGame::setDefaultSearchPath( const char *name, const char *ext )
{
   ext;
   resourceObject.get()->setSearchPath(name);
}   

//------------------------------------------------------------------------------
void SimGame::setTime(double newTime)
{
   if (newTime > getTime()) {
      timeOffset = newTime;
      timer.reset();
   }
}

void SimGame::forceSetTime(double newTime)
{
   // can set backwards in time
   timeOffset = newTime;
   timer.reset();
}

static const char *intToStr(int i)
{
   static char buffer[20];
   sprintf(buffer, "%d", i);
   return buffer;
}

static const char *floatToStr(double f)
{
   static char buffer[20];
   sprintf(buffer, "%g", f);
   return buffer;
}


//------------------------------------------------------------------------------
const char *SimGame::consoleCallback(CMDConsole *console, int id, int argc, const char *argv[])
{
   switch(id) 
   {
      case Schedule: {
         if(argc != 3 && argc != 4)
         {
            Console->printf("schedule(statement, deltaTime, <object>);");
            return "False";
         }
         if (!manager->findObject(SimConsoleSchedulerId)) {
            Console->printf("schedule: scheduler is not running");
            return "False";
         }
         SimConsoleEvent *cevt;
         cevt = new SimConsoleEvent;
         strcpy(cevt->argBuffer, argv[1]);
         double dt = atof(argv[2]);
         if(dt < 0)
            dt = 0;
         cevt->time = manager->getCurrentTime() + dt;
         cevt->argc = -1;
         cevt->echo = false;
         cevt->privilegeLevel = 0;
         SimObject *dest = NULL;

         if(argc == 3)
            dest = manager->findObject(SimConsoleSchedulerId);
         else
            dest = manager->findObject(argv[3]);
         
         if(dest)   
         {
            cevt->address.set(dest);
            manager->postEvent(cevt, false);
         
            sprintf(buffer, "%d", (int)cevt->time);
            return buffer;
         }
         else
         {
            delete cevt;
            return "False";
         }
      }
      case GetSimTime:
         return floatToStr(manager->getCurrentTime());
      case TimeScale:
         timeOffset += timer.getElapsed() * timeScale;
         timer.reset();
         timeScale = atof(argv[1]);
         if(timeScale < .0001)
            timeScale = .0001;
         break;
      case DefaultSearchPath:
         // argv[0] == variable name
         // The volume only stores a pointer to the path.
         // Argv[1] should be valid untill this case is called again.
            setDefaultSearchPath( const_cast<char*>(argv[1]) );
         break;

      case LoadObject:
         if (argc == 3) {
            Persistent::Base::Error err;
            Persistent::Base* base = Persistent::Base::fileLoad( argv[2], &err );
            if (!base) {
               // attempt volume load 2nd
               ResourceManager *rm = SimResource::get(manager);
               if (rm) {
                  void *data = rm->lock(argv[2]);
                  if (data) {
                     MemRWStream sio(rm->getSize(argv[2]), data);
                     base = Persistent::Base::load(sio, &err);
                  }
               }
            }
            
            if (err == Persistent::Base::Ok && base) {
               SimObject* obj = dynamic_cast<SimObject*>(base);
               if (obj) {
                  manager->addObject(obj);
                  manager->assignName(obj, argv[1]);
                  return intToStr(obj->getId());
               }
               else {
                  console->printf ("loadObject: Persistent object not a SimObject");
                  delete base;
               }
            }
            else
               console->printf ("loadObject: Error reading file");
         }
         else
            console->printf ("loadObject: objectName fileName");
         return "0";

      case StoreObject:
         if (argc == 3 || argc == 4) {
            SimObject* obj = manager->findObject(argv[1]);
            if (obj) {
               FileRWStream fs(argv[2]);
               if (fs.getStatus() == STRM_OK) {
                  int user = (argc == 4)? atoi(argv[3]): 0;
                  if (obj->store(fs,user) == Persistent::Base::Ok)
                     return "True";
                  console->printf("storeObject: Error writing object");
               }
            }
            else
               console->printf("storeObject: Could not find object");
         }
         else
            console->printf("storeObject: objectName fileName [userField]");
         return "False";

      case DeleteObject:
         if (argc > 1) {
            for (int i = 1; i < argc; i++) {
               SimObject* obj = manager->findObject(argv[i]);
               if (obj)
               {
                  manager->unregisterObject(obj);
                  delete obj;
                  return "True";
               }
               else
                  console->printf("deleteObject: "
                     "Object \"%s\" doesn't exist",argv[i]);
            }
         }
         else
            console->printf("deleteObject: objectName [objectName] ...");
         return "False";

      case IsObject: {
         if (argc > 1) {
            for (int i = 1; i < argc; i++)
               if (!manager->findObject(argv[i]))
                  return "False";
            return "True";
         }
         else
            console->printf("isObject: objectName [objectName] ...");
         return "False";
      }

      case IsFile: {
         if (argc > 1) {
            for (int i = 1; i < argc; i++) {
               FileRStream tmp(argv[i]);
               if (tmp.getStatus() != STRM_OK)
                  return "False";
            }
            return "True";
         }
         else
            console->printf("isFile: fileName [fileName] ...");
         return "False";
      }

      case ListVolumes: {
         SimResource* rm;
         if ( (rm = SimResource::find(manager)) != NULL ) 
         {
            for (SimResource::iterator itr = rm->begin(); 
               itr != rm->end(); ++itr) {
               // Resource can only have SimVolume objects in it.
               SimVolume* volume = static_cast<SimVolume*>(*itr);
               const char* name = volume->getName();
               console->printf("   \"%s\": \"%s\"",name? name: "",
                  volume->getFileName());
            }
         }
         }
         break;

      case ListObjects: {
         SimObject* obj;
         SimSet* set;
         SimSet::iterator itr;
         if (argc > 1) {
            if ((obj = manager->findObject(argv[1])) == 0) {
               console->printf("listObjects: Could not find object");
               break;
            }
         }
         else
            obj = manager;
         if ( (set = dynamic_cast<SimSet*>(obj)) != NULL )
         {
            for ( itr = set->begin();
                  itr != set->end(); itr++) {
               bool isSet = dynamic_cast<SimSet*>(*itr) != 0;
               const char* name = (*itr)->getName();
               if (name)
                  console->printf("   %d,\"%s\": %s %s",(*itr)->getId(),
                     name,(*itr)->getClassName(),isSet?"(g)":"");
               else
                  console->printf("   %d: %s %s",(*itr)->getId(),
                     (*itr)->getClassName(),isSet?"(g)":"");
            }
         }
         else
            console->printf("listObjects: Object is not a set");
         }
         break;

      case NewObject:
         if (argc >= 3) {
            Persistent::Base* obj = reinterpret_cast<Persistent::Base*>
               (Persistent::create(argv[2]));
            if (obj) {
               SimObject* simObj = dynamic_cast<SimObject*>(obj);
               if (simObj) {
                  if (!manager->registerObject(simObj))
                  {
                     Console->printf("newObject for: %s - failed register object", argv[2]);
                     delete simObj;
                     return "0";
                  }
                  manager->addObject(simObj);
                  if (argv[1][0] != '\0')
                     manager->assignName(simObj,argv[1]);
                  if(!simObj->processArguments(argc - 3, argv + 3))
                  {
                     manager->unregisterObject(simObj);
                     delete simObj;
                     char buf[384];
                     buf[0] = '\0';
                     for (int i = 3; i < argc && (strlen(buf) + strlen(argv[i]) < 384); i++) {
                        strcat(buf, argv[i]);
                        if (i + 1 < argc)
                           strcat(buf, ", ");
                     }
                     console->printf("newObject for: %s failed arg list: %s", argv[2], buf);
                     return "0";
                  }
                  else
                     return intToStr(simObj->getId());
               }
               else {
                  delete obj;
                  console->printf("newObject: object not a simObject");
                  return "0";
               }
            }
            else {
               console->printf("newObject: persistent create failed (%s)", argv[2]);
               return "0";
            }
         }
         else {
            console->printf("newObject: objectName className");
            return "0";
         }
      case AddToSet: {
         if (argc > 2) {
            SimObject* obj;
            if ( (obj = manager->findObject(argv[1])) != NULL )
            {
               SimSet* grp;
               if ( (grp = dynamic_cast<SimSet*>(obj)) != NULL )
               {
                  int error = 0;
                  for (int i = 2; i < argc; i++) 
                  {
                     SimObject* obj = manager->findObject(argv[i]);
                     if (obj && obj != manager)
                        grp->addObject(obj);
                     else 
                     {
                        console->printf("addToSet: "
                           "Object \"%s\" doesn't exist", argv[i]);
                        error++;
                     }
                  }
                  return error ? "False" : "True";
               }
               else
                  console->printf("addToSet: Object \"%s\" is not a set",
                     argv[1]);
            }
            else
               console->printf("addToSet: Set \"%s\" does not exist",
                  argv[1]);
         }
         else
            console->printf("addToSet: setName [objectName] ...");
         }
         return "False";

        case PurgeResources:
         SimResource::get(manager)->purge(true);
         break;

      case PostAction: {
         // post name type value
         if (argc == 4) {
            SimObject* obj = manager->findObject(argv[1]);
            if ( obj != NULL )
            {
               int id = atoi(argv[2]);
               if (id)
               {
                  SimActionEvent* action = new SimActionEvent;
						SimObject* ref = 0;
						char c = argv[3][0];
						if (!isdigit(c) && c != '.' && c != '-')
							// Object ref by name
	                  ref = manager->findObject(argv[3]);
                  if (ref)
                     action->fValue = ref->getId();
                  else
                     action->fValue = atof(argv[3]);
                  action->action = id;
                  obj->postCurrentEvent(action);
                  return "True";
               }
               console->printf(avar("postAction: obsolete action name '%s'.  Convert to TAG",argv[2]));
#ifndef _MSC_VER
               #pragma message "Remove old postAction code..."
#endif
               for (int i = 0; i < actionList.size(); i++)
                  if (!strcmp(actionList[i]->name,argv[2])) 
                  {
                     SimActionEvent* ev = new SimActionEvent;
                     ev->action = actionList[i]->action;

                     // Convert object names to Id's
                     SimObject* ref = manager->findObject(argv[3]);
                     if (ref)
                        ev->fValue = ref->getId();
                     else
                        ev->fValue = atof(argv[3]);

                     obj->postCurrentEvent(ev);
                     return "True";
                  }
               console->printf("postAction: Unknown action name");
               break;
            }
            //console->printf("postAction: Object does not exist");
         }
         else
            console->printf("postAction: objectName actionName value");
         }
         return "False";

      case Focus:
         if (argc > 1)
         {
            SimObject *obj = manager->findObject(argv[1]);
            if (!obj) {
               int id;
               if (sscanf(argv[1], "%d", &id) == 1)
                  obj = manager->findObject(id);
            }
            if (!obj)
               return "False";
            SimGainFocusEvent::post(obj);
         }
         break;

      case Unfocus:
         if (argc > 1)
         {
            SimObject *obj = manager->findObject(argv[1]);
            if (!obj) {
               int id;
               if (sscanf(argv[1], "%d", &id) == 1)
                  obj = manager->findObject(id);
            }
            if (!obj)
               return "False";
            SimLoseFocusEvent::post(obj);
         }
         break;

      case FocusClient:
         {
            if(argc > 1 && argv[1][0] == '2')
            {
               if(getWorld(CLIENT2))
                  focus(CLIENT2);
            }
            else
               focus( CLIENT );
            break;
         }

      case FocusServer: {
         if (getWorld(SERVER)) {
            focus( SERVER );
            return "True";
         }
         return "False";
      }

      case NewClient:
         {
            WorldType world = CLIENT;

            if(argc == 2 && argv[1][0] == '2')
               world = CLIENT2;

            SimWorld *client = new SimWorld;
            setWorld(world, client);
         }
         break;

      case NewServer:
         {
            SimWorld *server = new SimWorld;
            setWorld(SERVER, server);
         }
         break;

      case DeleteClient: setWorld( CLIENT, NULL );  break;
      case DeleteServer: setWorld( SERVER, NULL );  break;

      case RemoteEval:
         {
            if(argc < 3)
               console->printf("%s: <managerId> cmd args...", argv[0]);
            else
            {
               UInt32 managerId = atoi(argv[1]);
               if(managerId == manager->getId() || managerId == 0)
                  console->printf("%s: bad managerId.", argv[0]);
               else
                  SimConsoleEvent::post(manager, managerId, argc-2, argv+2);
            }
         }
         break;
      case InspectObject:
         if (argc == 2) {
            SimObject* obj;
            if ( (obj = manager->findObject(argv[1])) != NULL ) {
               // From SimTreeView::inspectObject
               //
               InspectWindow *oi = dynamic_cast<InspectWindow *>(manager->findObject("objectInspector"));
               if ( !oi )
               {
                  POINT curs;
                  GetCursorPos( &curs );
                  Point2I pos(curs.x,curs.y);
                  Point2I size(300,300);

                  oi = new InspectWindow( (GWWindow*)this, "Object Info", pos, size, (SimTagDictionary *)NULL );
                  manager->addObject(oi,"objectInspector");
               }

               oi->setObjToInspect( obj, obj ? obj->getClassName() : NULL );
            } else {
               console->printf("object %s not found", argv[1]);
            }
         } else {
            console->printf("inspectObject [objectName/Id]");
         }
         break;

      case RenameObject: {
         // post name type value
         if (argc == 3) {
            SimObject* obj = manager->findObject(argv[1]);
            if ( obj != NULL )
            {
               obj->assignName(argv[2]);
               return "True";
            }
            else
            {
               console->printf(avar("renameObject: object does not exist '%s'.", argv[1]));
            }
         }
         else
            console->printf("renameObject: objectName newObjectName");
         }
         return "False";

      //--------------------------------------
      case SetObjectId: {
         // post name type value
         if (argc == 3) {
            SimObject* obj = manager->findObject(argv[1]);
            if ( obj != NULL )
            {
               int id = atoi(argv[2] );
               obj->setId(id);
               return "True";
            }
            else
            {
               console->printf(avar("setObjectId: object does not exist '%s'.", argv[1]));
            }
         }
         else
            console->printf("setObjectId: objectName newObjectId");
         }
         return "False";

      //--------------------------------------
      case SetWindowTitle:
         if (argc <= 3)
         {
            if (argc == 3)            
            {
               SimCanvas *canvas = dynamic_cast<SimCanvas*>(manager->findObject(argv[1]));
               if (canvas)
                  SetWindowText(canvas->getHandle(), argv[2]);
            }
            SetConsoleTitle(argc == 3 ? argv[2] : argv[1]);
         }
         else
            console->printf("SetWindowTitle( {canvas_name,} title  );");

      //--------------------------------------
      case GetPathOf:
         if (argc == 2)
         {
            const char *path = SimResource::get(manager)->getPathOf(argv[1]);
            if (path) return (path);
            return ("");
         }
         else
            console->printf("GetPathof( filename );" );
         break;

      //--------------------------------------
      case GetGroup:
         if (argc == 2)
         {
            SimObject *obj = manager->findObject(argv[1]);
            SimGroup  *grp = obj ? obj->getGroup() : 0;
            if (!grp)
               return ("False");
            static char buffer[24];
            itoa(grp->getId(), buffer, 10);
            return (buffer);
         }
         console->printf("GetGroup( object );" );
         break;

      //--------------------------------------
      case IsMember:
         if (argc == 3)
         {
            SimSet *set = dynamic_cast<SimSet*>(manager->findObject(argv[1]));
            if (!set)
               return ("False"); 

            SimObjectId Id = 0;
            if (argv[2][0] >= '0' && argv[2][0] <= '9')
               Id = atoi(argv[2]);

            // loop though the set and try to find the object
            SimSet::iterator itr = set->begin();
            for (; itr != set->end(); itr++)
            {
               if ( (Id && (*itr)->getId() == Id) ||
                    ((*itr)->getName() && (stricmp((*itr)->getName(), argv[2]) == 0)) )
                  return ("True"); 
            }
            return ("False"); 
         }
         console->printf("IsMember( set/group, object );" );
         break;


      //--------------------------------------
      case GetNextObject:
         if (argc == 3)
         {
            SimSet *set = dynamic_cast<SimSet*>(manager->findObject(argv[1]));
            if (!set)
               return ("False"); 

            SimObjectId Id = 0;
            if (argv[2][0] >= '0' && argv[2][0] <= '9')
               Id = atoi(argv[2]);

            // loop though the set and try to find the object
            SimSet::iterator itr = set->begin();
            SimObject *obj = (itr == set->end()) ? 0 : *itr;

            for (; itr != set->end(); itr++)
            {
               if ( (Id && (*itr)->getId() == Id) ||
                    ((*itr)->getName() && (stricmp((*itr)->getName(), argv[2]) == 0)) )
               {
                  // get the next object
                  itr++;
                  if (itr == set->end())
                     obj = NULL;
                  else
                     obj = *itr;
                  break;
               }
            }
            if (obj)
            {
               static char buffer[24];
               itoa(obj->getId(), buffer, 10);
               return (buffer);
            }
            else
               return ("0");
         }
         console->printf("GetNextObject( set/group, object );" );
         console->printf("GetNextObject( set/group, 0 );  // to retrieve first object in a set" );
         break;

      //--------------------------------------
      default:
         return CMDConsole::consoleCallback(console, id, argc, argv);
   }
   return "True";   
}


//------------------------------------------------------------------------------
void SimGame::init()
{
   //
   setVariable("ConsoleWorld::FrameRate","0");
   setVariable("ConsoleWorld::msecsPerFrame","0");
   addVariable(DefaultSearchPath,"ConsoleWorld::DefaultSearchPath",this,"");
   addVariable(TimeScale,"SimGame::TimeScale",this,"1.0");
   //
   addCommand(LoadObject,     "loadObject",this);
   addCommand(StoreObject,    "storeObject",this);
   addCommand(DeleteObject,   "deleteObject",this);
   addCommand(IsObject,       "isObject", this);
   addCommand(IsFile,         "isFile", this);
   addCommand(ListVolumes,    "listVolumes",this);
   addCommand(ListObjects,    "listObjects",this);
   addCommand(NewObject,      "newObject",this);
   addCommand(AddToSet,       "addToSet",this);
   addCommand(PostAction,     "postAction",this);
   addCommand(Focus,          "focus",this);
   addCommand(Unfocus,        "unfocus",this);
   addCommand(PurgeResources, "purgeResources",this);
   addCommand(FocusClient,    "focusClient",this);
   addCommand(FocusServer,    "focusServer",this);
   addCommand(NewClient,      "newClient",this);
   addCommand(NewServer,      "newServer",this);
   addCommand(DeleteClient,   "deleteClient",this);
   addCommand(DeleteServer,   "deleteServer",this);
   addCommand(RemoteEval,     "remoteEval",this);
   addCommand(InspectObject,  "inspectObject", this);
   addCommand(RenameObject,   "renameObject", this);
   addCommand(SetObjectId,    "setObjectId", this);
   addCommand(Schedule,       "schedule", this);
   addCommand(GetSimTime,     "getSimTime", this);
   addCommand(SetWindowTitle, "setWindowTitle", this);
   addCommand(GetPathOf,      "getPathOf", this);
   addCommand(GetGroup,       "getGroup", this);
   addCommand(IsMember,       "isMember", this);
   addCommand(GetNextObject,  "getNextObject", this);

   sgcp = new SimGuiConsolePlugin;
   registerPlugin(sgcp);
}


//----------------------------------------------------------------------------
void SimGame::registerPlugin(SimConsolePlugin* pl)
{
   // Assign the plugins our own client world manager
   pl->init(this, manager);
   plugins.push_back(pl);
}


//------------------------------------------------------------------------------
void SimGame::registerActionList( ActionName *list, int numEntries)
{
   for ( int i=0; i<numEntries; i++)
      actionList.push_back( &list[i] );
   SimActionEvent::actionTable = (ActionName**)actionList.address();
   SimActionEvent::actionTableSize = actionList.size();
}


//----------------------------------------------------------------------------
void SimGame::startFrame()
{
   // Frame rate counter
   absFrameStart = timer.getElapsed();
   frameStart = timeOffset + absFrameStart * timeScale;

   // Inform all the plugins
   for (int i = 0; i < plugins.size(); i++)
      plugins[i]->startFrame();
}


//------------------------------------------------------------------------------
void SimGame::endFrame()
{
   // Frame rate counter
   const float alpha = 0.02f;
   avgSPF = avgSPF*(1.0-alpha)+(timer.getElapsed()-absFrameStart)*alpha;
   char fps[256];
   char mspf[256];

   fpsDelay = (++fpsDelay)&0x0f;
   if (!fpsDelay)
   {
      sprintf(fps,"%2.2f fps", 1.0/avgSPF );
      sprintf(mspf, "%2.2f", 1000.0 * avgSPF );
      setVariable("ConsoleWorld::FrameRate",fps);
      setVariable("ConsoleWorld::msecsPerFrame", mspf);
   }

   // Inform all the plugins
   for (int i = 0; i < plugins.size(); i++)
      plugins[i]->endFrame();

   // Console Frame Echo
   
   Console->executef(1, "Game::EndFrame");
}   

SimGuiConsolePlugin *SimGame::getConsole()
{
   return (sgcp);
}

//------------------------------------------------------------------------------
void SimGame::advanceToTime(double time)
{
   // Give the Windows Socket thread to time to process async
   // socket messages.  This is a hack for WinNT.
   Sleep(0);

   WorldType w;
   SimManager *oldManager = world[FOCUSED]->getManager();
   int i;

   for(w = SERVER; w < N_WORLDS; w=WorldType(w+1))
   {
      if(world[w])
      {
         // gotta set the manager for all the plugins to this
         // world's manager
         manager = world[w]->getManager();

         for (i = 0; i < plugins.size(); i++)
            plugins[i]->setManager( manager );
         
         world[w]->advanceToTime(time);
      }
   }
   manager = oldManager;
     for (i = 0; i < plugins.size(); i++)
      plugins[i]->setManager( manager );
}


//------------------------------------------------------------------------------
void SimGame::render()
{
   WorldType w;
   for(w = SERVER; w < N_WORLDS; w=WorldType(w+1))
      if(world[w])
         world[w]->render();
}

//------------------------------------------------------------------------------
void SimGame::focus(SimManager *newManager)
{
   WorldType w;
   for(w = SERVER; w < N_WORLDS; w=WorldType(w+1))
      if(world[w]->getManager() == newManager)
         world[FOCUSED] = world[w];

   manager = newManager;
   // Inform all the plugins
   for (int i = 0; i < plugins.size(); i++)
      plugins[i]->setManager( newManager );
}

void SimGame::focus(WorldType w)
{
   world[FOCUSED] = world[w];
   
   manager = world[FOCUSED] ? world[FOCUSED]->getManager() : NULL;
   // Inform all the plugins
   for (int i = 0; i < plugins.size(); i++)
      plugins[i]->setManager( manager );
}

//------------------------------------------------------------------------------
void SimGame::setWorld(WorldType w, SimWorld *_world)
{
   if ( world[w] && inputManager && 
         inputManager->getManager() == world[w]->getManager() ) 
   {
      focus( (w == SERVER)? CLIENT: SERVER );
      inputManager->getManager()->unregisterObject(inputManager);
     }
   delete world[w];
   world[w] = _world;
   if(w == CLIENT && _world)
   {
      SimManager *wmgr = _world->getManager();
      SimSet *sp = (SimSet *) wmgr->findObject(SimInputConsumerSetId);
      sp->addObject(this);
      if (!inputManager->open())
         AssertFatal(0, "DirectInput init failed.");

      wmgr->addObject(inputManager);
   }
}


SimWorld* SimGame::getWorld(WorldType w)
{
   return ( world[w] );
}


SimManager* SimGame::getManager(WorldType w)
{
   return ( world[w] ? world[w]->getManager() : NULL );
}


//------------------------------------------------------------------------------
SimGame* SimGame::get()
{
   return ( theGame );
}

//------------------------------------------------------------------------------
bool SimGame::processEvent(const SimEvent *event)
{
   if(event->type != SimInputEventType)
   {
      return false;
   }

   const SimInputEvent *ev = (const SimInputEvent*)event;

   if(ev->deviceType == SI_KEYBOARD && ev->deviceInst == 0 && ev->objType == SI_KEY)
   {
      if (ev->ascii == '`' && ev->action == SI_MAKE)
      {
         sgcp->activate(sgcp->enabled() && !sgcp->active());
         return (true);
      }
      else if (sgcp->active() && sgcp->enabled() && sgcp->onSimInputEvent(ev))
      {
         return (true);
      }
   }
   SimActionMap::Node *n = gameActionMap->findMatch(ev);

   // If it's not a console command, we don't handle it
   if(!n || !n->action.consoleCommand)
   {
      return false;
   }

   // Pass it to the normal console
   Console->evaluate(n->action.consoleCommand);
   return (true);
}

bool SimGame::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
         q->priority = SI_PRIORITY_FILTER;
      return (true);
   }
   return false;
}

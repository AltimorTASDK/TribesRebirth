//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <SimInput.h>
#include <SimInputDevice.h>
#include <simConsolePlugin.h>
#include <SimInputPlugin.h>
#include <SimTagDictionary.h>
#include <simAction.h>
#include <simResource.h>
#include <simGame.h>

#define IM_NAME          "InputManager"
#define ECHO_FILTER_NAME "InputEcho"

static char mapName[256] = {0,};

enum CallbackID
{
   NewInput,
   ListDevices,
   GetDeviceInfo,
   SaveDeviceInfo,
   Open,
   Close,
   Capture,
   Release,
   Activate,
   Deactivate,
   EditActionMap,
   NewActionMap,
   BindAction,
   BindCommand,
   Bind,
   SaveActionMap,
};



//------------------------------------------------------------------------------
void SimInputPlugin::init()
{
   console->printf("SimInputPlugin");
   console->addCommand(NewInput,             "newInputManager",      this);
   console->addCommand(ListDevices,          "listInputDevices",     this);
   console->addCommand(GetDeviceInfo,        "getInputDeviceInfo",     this);
   console->addCommand(GetDeviceInfo,        "getInputDeviceInfo",     this);
   console->addCommand(SaveDeviceInfo,       "saveInputDeviceInfo",     this);
   console->addCommand(Open,                 "inputOpen",     this);
   console->addCommand(Close,                "inputClose",     this);
   console->addCommand(Capture,              "inputCapture",     this);
   console->addCommand(Release,              "inputRelease",     this);
   console->addCommand(Activate,             "inputActivate",     this);
   console->addCommand(Deactivate,           "inputDeactivate",     this);
   console->addCommand(EditActionMap,          "editActionMap", this );
   console->addCommand(NewActionMap,          "newActionMap", this );
   console->addCommand(BindAction,             "bindAction", this );
   console->addCommand(BindCommand,          "bindCommand", this );
   console->addCommand(Bind,                  "bind", this );
   console->addCommand(SaveActionMap,          "saveActionMap", this );
}


//----------------------------------------------------------------------------
void SimInputPlugin::startFrame()
{
}

void SimInputPlugin::endFrame()
{
}

SimInputManager* SimInputPlugin::getInput(bool msg)
{
   SimInputManager *im = dynamic_cast<SimInputManager*>(manager->findObject(SimInputManagerId));
   if (!im && msg)
      CMDConsole::getLocked()->printf("Error: must 'newInputManager' first.");
   return (im);
}   


//------------------------------------------------------------------------------
static void addExtension(char *name, char *ext)
{
   char *period = strrchr(name, '.');
   if (period)
   {
      if (stricmp(period, ext) != 0)
         strcat(name, ext);
   }
   else
      strcat(name, ext);
}   


//------------------------------------------------------------------------------
static void writeString(StreamIO &s, const char *str)
{
   s.write(strlen(str), str);
}   


//----------------------------------------------------------------------------
const char *SimInputPlugin::consoleCallback(CMDConsole *, int id,int argc,const char *argv[])
{
   argv;
   if (!manager) return "False";
   switch(id) 
   {
      //--------------------------------------
      case NewInput:
         if (argc == 1 )
         {
            SimInputManager *im = getInput(false);
            if (!im)
            {
               im = new SimInputManager;
               if (im)
               {
                  manager->addObject(im, IM_NAME);
                  return (0);
               }
            }
         }
         else
            console->printf("newInputManager");
         return "False";

      //--------------------------------------
      case Open:
         if (argc == 1 )
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";

            if( im->open() )  return (0);
            else              CMDConsole::getLocked()->printf("Error opening SimInputManager");
         }
         else
            console->printf("inputOpen");
         return "False";

      //--------------------------------------
      case Close:
         if (argc == 1 )
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";

            im->close();
            return (0);
         }
         else
            console->printf("inputClose");
         return "False";
            
      //--------------------------------------
      case ListDevices:
         if (argc == 1)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            
            SimSet::iterator i = im->begin();
            for (; i != im->end() ; i++)
            {
               SimInputDevice *id = dynamic_cast<SimInputDevice*>(*i);
               if (id)
                  console->printf("  %s - %s (%s%s)",
                     id->getDeviceName(),
                     id->getProductName(),
                     (id->isActive() ? "active" : "deactivated"),
                     (id->isCaptured() ? "/captured" : "")
                     );
            }
            console->printf("done.");
         }
         else
            console->printf("inputListDevices");
         return "False";

      //--------------------------------------
      case GetDeviceInfo:
         if (argc == 2)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            
            SimSet::iterator i = im->begin();
            for (; i != im->end() ; i++)
            {
               SimInputDevice *id = dynamic_cast<SimInputDevice*>(*i);
               if (id && stricmp(argv[1], id->getDeviceName())==0)
               {
                  console->printf("Device Name: %s", id->getDeviceName());
                  console->printf("Long Name: %s", id->getProductName());
                  console->printf("  type, name");
                  int i=0;
                  const SimInputDevice::ObjInfo *devObj = id->getObject(i);
                  while (devObj)
                  {
                     switch (devObj->objType)
                     {
                        case SI_UNKNOWN:  console->printf("  unknown unknown%d", devObj->objInst); break;
                        case SI_BUTTON:   console->printf("  button button%d", devObj->objInst); break;
                        case SI_POV:      console->printf("  pov pov%d", devObj->objInst); break;
                        case SI_XAXIS:    console->printf("  axis xAxis%d", devObj->objInst); break;
                        case SI_YAXIS:    console->printf("  axis yAxis%d", devObj->objInst); break;
                        case SI_ZAXIS:    console->printf("  axis zAxis%d", devObj->objInst); break;
                        case SI_RXAXIS:    console->printf("  axis RxAxis%d", devObj->objInst); break;
                        case SI_RYAXIS:    console->printf("  axis RyAxis%d", devObj->objInst); break;
                        case SI_RZAXIS:    console->printf("  axis RzAxis%d", devObj->objInst); break;
                        case SI_SLIDER:    console->printf("  slider slider%d", devObj->objInst); break;
                        case SI_KEY:   
                           {
                              const DIKCode *dik = SimInputManager::getDIK_STRUCT(devObj->objInst);
                              if (dik)
                              {
                                 if (dik->name)
                                    console->printf("  key %s", dik->name); 
                                 else if (dik->lower)
                                    console->printf("  key %c", dik->lower); 
                              }
                              else
                                 console->printf("  key unknown");
                           }
                           break;
                     }
                     devObj = id->getObject(++i);
                  }
                  return (0);   
               }
            }
            console->printf("unknown device: %s", argv[1]);
         }
         else
            console->printf("getInputDeviceInfo deviceName");
         return "False";

      //--------------------------------------
      case SaveDeviceInfo: 
         if (argc == 2)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            FileWStream stream;
            if (!stream.open(argv[1]))
               return ("False");

            SimSet::iterator i = im->begin();
            for (; i != im->end() ; i++)
            {
               char buffer[1024];
               SimInputDevice *id = dynamic_cast<SimInputDevice*>(*i);
               if (!id)
                  continue;

               sprintf(buffer, "Device Name: %s\r\nLong Name:   %s\r\n", id->getDeviceName(), id->getProductName());
               writeString(stream,buffer);
               writeString(stream,"  Type    Name\r\n");
               writeString(stream,"  ---------------------\r\n");

               int i=0;
               const SimInputDevice::ObjInfo *devObj = id->getObject(i);
               while (devObj)
               {
                  switch (devObj->objType)
                  {
                     case SI_UNKNOWN:  sprintf(buffer,"  unknown unknown%d\r\n", devObj->objInst); break;
                     case SI_BUTTON:   sprintf(buffer,"  button  button%d\r\n", devObj->objInst); break;
                     case SI_POV:      sprintf(buffer,"  pov     povx%d\r\n"
                                                      "  pov     povy%d\r\n", devObj->objInst, devObj->objInst); break;
                     case SI_XAXIS:    sprintf(buffer,"  axis    xAxis%d\r\n", devObj->objInst); break;
                     case SI_YAXIS:    sprintf(buffer,"  axis    yAxis%d\r\n", devObj->objInst); break;
                     case SI_ZAXIS:    sprintf(buffer,"  axis    zAxis%d\r\n", devObj->objInst); break;
                     case SI_RXAXIS:   sprintf(buffer,"  axis    RxAxis%d\r\n", devObj->objInst); break;
                     case SI_RYAXIS:   sprintf(buffer,"  axis    RyAxis%d\r\n", devObj->objInst); break;
                     case SI_RZAXIS:   sprintf(buffer,"  axis    RzAxis%d\r\n", devObj->objInst); break;
                     case SI_SLIDER:   sprintf(buffer,"  slider  slider%d\r\n", devObj->objInst); break;
                     case SI_KEY:   
                        {
                           const DIKCode *dik = SimInputManager::getDIK_STRUCT(devObj->objInst);
                           if (dik)
                           {
                              if (dik->name)
                                 sprintf(buffer,"  key    \"%s\"\r\n", dik->name); 
                              else if (dik->lower)
                                 sprintf(buffer,"  key    \"%c\"\r\n", dik->lower); 
                           }
                           else
                              strcpy(buffer, "  key    Unknown\r\n");
                        }
                        break;
                  }
                  writeString(stream,buffer);
                  devObj = id->getObject(++i);
               }
               writeString(stream,"\r\n");
            }
         }
         else
            Console->printf("saveInputDeviceInfo( filename );");
         break;

      //--------------------------------------
      case Capture:
         if (argc == 1)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            im->capture();
         }
         else
            console->printf("inputCapture");
         return "False";

      //--------------------------------------
      case Release:
         if (argc == 1)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            im->release();
         }
         else
            console->printf("inputRelease");
         return "False";


      //--------------------------------------
      case Activate:
         if (argc == 2)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            
            if (stricmp(argv[1], "all") == 0)
            {
               im->activate(SI_ANY, SI_ANY);
               return (0);
            }

            SimSet::iterator i = im->begin();
            for (; i != im->end() ; i++)
            {
               SimInputDevice *id = dynamic_cast<SimInputDevice*>(*i);
               if (id && stricmp(argv[1], id->getDeviceName())==0)
               {
                  id->activate();
                  return (0);
               }
            }
            console->printf("unknown device: %s", argv[1]);
         }
         console->printf("inputActivate {[all/joystick/mouse/keyboard/device][0-n]}");
         return "False";

      //--------------------------------------
      case Deactivate:
         if (argc == 2)
         {
            SimInputManager *im = getInput(false);
            if (!im)
               return "False";
            
            SimSet::iterator i = im->begin();
            for (; i != im->end() ; i++)
            {
               SimInputDevice *id = dynamic_cast<SimInputDevice*>(*i);
               if (id && stricmp(argv[1], id->getDeviceName())==0)
               {
                  id->deactivate();
                  return (0);
               }
            }
            console->printf("unknown device: %s", argv[1]);
         }
         console->printf("inputDeactivate {[joystick/mouse/keyboard/device][0-n]}");
         return "False";

      //--------------------------------------
      case Bind:
      {
         if (argc > 4)
         {
            Resource<SimActionMap> hMap = SimResource::get( manager )->load( SimGameActionMapName );
            if (!(bool)hMap)
            {
               console->printf( "bind error: No SimGame action map!!");
               return 0;
            }
            argc--; argv++;
            int to = 0;
            for (; to < argc; to++)
               if (stricmp(argv[to], "to") == 0)
                  break;
            SimInputEvent event;
            SimActionEvent action;
            BitSet32 flags;
            if (parseSimInputEvent( to, argv, &event ))
            {
               to++;
               action.consoleCommand = new char[strlen(argv[to]) + 1];
               strcpy(action.consoleCommand, argv[to]);
               hMap->mapEvent( &event, &action, flags );
            }
            else
               console->printf( "bind event TO command - error: malformed 'Device section'." );
         }
         else
         {
            console->printf( "bind keyboard[0-n] [make, break] {ctrl, alt, shift} {ascii, keyName}");
            console->printf( "bind joystick[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bind joystick[0-n] move[x,y,z,r,u,v] ");
            console->printf( "bind mouse[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bind mouse[0-n] move[x,y,z,r,u,v] ");
         }
         return "False";
      }
      case BindCommand:
      {
         if (argc > 4)
         {
            Resource<SimActionMap> hMap = SimResource::get( manager )->load( mapName );
            if (!(bool)hMap)
            {
               console->printf( "bindAction error: must call 'editActionMap' or 'newActionMap' first. (%s)", mapName );
               return 0;
            }
            argc--; argv++;
            int to = 0;
            for (; to < argc; to++)
               if (stricmp(argv[to], "to") == 0)
                  break;
            SimInputEvent event;
            SimActionEvent action;
            BitSet32 flags;
            if (parseSimInputEvent( to, argv, &event ))
            {
               to++;
               action.consoleCommand = new char[strlen(argv[to]) + 1];
               strcpy(action.consoleCommand, argv[to]);
               hMap->mapEvent( &event, &action, flags );
            }
            else
               console->printf( "bindCommand event TO command - error: malformed 'Device section'." );
         }
         else
         {
            console->printf( "bindCommand keyboard[0-n] [make, break] {ctrl, alt, shift} {ascii, keyName}");
            console->printf( "bindCommand joystick[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bindCommand joystick[0-n] move[x,y,z,r,u,v] ");
            console->printf( "bindCommand mouse[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bindCommand mouse[0-n] move[x,y,z,r,u,v] ");
         }
         return "False";
      }
      case BindAction:
      {
         int _argc = argc;
         const char **_argv = argv;
         if (argc > 4)
         {
            Resource<SimActionMap> hMap = SimResource::get( manager )->load( mapName );
            if (!(bool)hMap)
            {
               console->printf( "bindAction error: must call 'editActionMap' or 'newActionMap' first. (%s)", mapName );
               return 0;
            }

            argc--; argv++;
            int to = 0;
            for (; to < argc; to++)
               if (stricmp(argv[to], "to") == 0)
                  break;

            SimInputEvent event;
            SimActionEvent action;
            BitSet32 flags;
            if (parseSimInputEvent( to, argv, &event ))
            {
               to++;
               if(parseSimActionEvent( argc - to, &argv[ to ], &action, &flags ))
                  hMap->mapEvent( &event, &action, flags );
               else
               {
                  char buf[256] = "-->bindAction ";
                  for (int i=0; i<_argc; i++)
                  {
                     strcat(buf, _argv[i]);
                     strcat(buf, " ");
                  }

                  console->printf( "bindAction event TO action - error: malformed 'TO section'." );
                  console->printf( buf );
                  return 0;
               }
            }
            else
            {
               char buf[256] = "-->bindAction ";
               for (int i=0; i<_argc; i++)
               {
                  strcat(buf, _argv[i]);
                  strcat(buf, " ");
               }

               console->printf( "bindAction event TO action - error: malformed 'Device section'." );
               console->printf( buf );
               return 0;
            }
         }
         else
         {
            console->printf( "bindAction keyboard[0-n] [make, break] {ctrl, alt, shift} {ascii, keyName}");
            console->printf( "bindAction joystick[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bindAction joystick[0-n] move[x,y,z,r,u,v] ");
            console->printf( "bindAction mouse[0-n] [make, break] {ctrl, alt, shift} button[0-n]");
            console->printf( "bindAction mouse[0-n] move[x,y,z,r,u,v] ");
         }
         return "False";
      }

      //--------------------------------------
      case EditActionMap:
      {
         if (argc == 2)
         {
            strcpy(mapName, argv[1]);
            addExtension(mapName, ".sae");
            ResourceManager *rm = SimResource::get( manager );
            ResourceObject *obj = rm->find( mapName );
            if ( !obj  ||  !obj->resource )
                rm->add( ResourceType::typeof( ".sae" ), mapName, new SimActionMap );
         }
         else
            mapName[0] = 0;
         return "False";
      }


      //--------------------------------------
      case NewActionMap:
      {
         if (argc == 2)
         {
            strcpy(mapName, argv[1]);
            addExtension(mapName, ".sae");
            ResourceManager *rm = SimResource::get( manager );
            ResourceObject *obj = rm->find( mapName );
            if ( !obj  ||  !obj->resource )
                rm->add( ResourceType::typeof( ".sae" ), mapName, new SimActionMap , true);
            else
               static_cast<SimActionMap*>(obj->resource)->clear();
         }
         else
            mapName[0] = 0;
         return "False";
      }

      //--------------------------------------
      case SaveActionMap:
      {
         if (argc > 2)
         {
            FileWStream s;
            char file[256];
            char name[256];
            strcpy(file, argv[1]);
            addExtension(file, ".cs");
            Resource<SimActionMap> hTemp;
            for (int i=2; i<argc; i++)
            {
               if (s.getStatus() == STRM_CLOSED)
                  if ( !s.open(file) )
                  {
                     console->printf(avar("Error creating file '%s'", file));
                     return (0);   
                  }

               strcpy(name, argv[i]);
               addExtension(name, ".sae");
               hTemp = SimResource::get( manager )->load( name );
               if ((bool)hTemp)
               {
                  sprintf(file,"NewActionMap(\"%s\");\n", name);
                  s.write(strlen(file), file);
                  writeActionMap(s, hTemp);
               }
               else
                  console->printf(avar("Unknown map '%s'", name));
            }   
         }
         else
            console->printf("saveactionMap filename.cs mapname1 {mapname2 ...}");
         return "False";
      }


   }
   return (0);
}




//----------------------------------------------------------------------------
struct PairDef
{
   char *name;
   BYTE  type;
};

static PairDef DeviceTypes[] = 
{
   { "keyboard",  SI_KEYBOARD },
   { "mouse",     SI_MOUSE },
   { "joystick",  SI_JOYSTICK },
   { "device",    SI_DEVICE },
   { 0,           0 }
};

static PairDef ActionTypes[] =
{
   { "make",      SI_MAKE },
   { "break",     SI_BREAK },
   { "move",      SI_MOVE },
   { 0,           0 }
};   

static PairDef ObjTypes[] =
{
   { "xaxis",     SI_XAXIS },
   { "yaxis",     SI_YAXIS },
   { "zaxis",     SI_ZAXIS },
   { "rxaxis",    SI_RXAXIS },
   { "ryaxis",    SI_RYAXIS },
   { "rzaxis",    SI_RZAXIS },
   { "slider",    SI_SLIDER },
   { "pov",       SI_POV },
   { "xpov",      SI_XPOV },
   { "ypov",      SI_YPOV },
   { "upov",      SI_UPOV },
   { "dpov",      SI_DPOV },
   { "lpov",      SI_LPOV },
   { "rpov",      SI_RPOV },
   { "button",    SI_BUTTON },
   { "key",       SI_KEY },
   { 0,           0 }
};   

static PairDef ModifierTypes[] =
{
   { "shift",     SI_SHIFT },
   { "control",   SI_CTRL },
   { "alt",       SI_ALT },
   { "l_shift",   SI_LSHIFT },
   { "r_shift",   SI_RSHIFT },
   { "l_control", SI_LCTRL },
   { "r_control", SI_RCTRL },
   { "l_alt",     SI_LALT },
   { "r_alt",     SI_RALT },
   { 0,           0 }
};   

//------------------------------------------------------------------------------
static BYTE getPairType( PairDef *table, const char *str )
{
   for(;table->name; table++)
      if (strnicmp( str, table->name, strlen(table->name) ) == 0)
         break;
   return (table->type);
}   


//------------------------------------------------------------------------------
static char* getPairStr( PairDef *table, BYTE type )
{
   for(;table->name; table++)
      if (table->type == type)
         break;
   return (table->name);
}   


//------------------------------------------------------------------------------
static BYTE getInst( const char *str )
{
   const char *end = str+strlen(str);
   do {
      end--;
   }while (*end >= '0' && *end <= '9');
   end++;
   return (*end ? atoi(end) : 0);
}   


//------------------------------------------------------------------------------
static void getModifiers( int &argc, const char **&argv, SimInputEvent *event )
{
   event->modifier = 0;
   for (;argc; argc--, argv++)
   {
      BYTE val = getPairType(ModifierTypes, *argv);
      if (!val)
         break;
      event->modifier |= val;   
   }
}   


//------------------------------------------------------------------------------
static void getModifiers( char *buffer, BYTE flags )
{
   PairDef *table = ModifierTypes;
   for(;table->name; table++)
      if ( (flags & table->type) == table->type)
      {
         strcat(buffer, avar("%s, ", table->name));
         flags &= ~table->type;
      }
}   

char *SimInputPlugin::buildConsoleCommand(int argc, const char **argv)
{
   int dstlen = 0;
   int i;
   for(i = 0; i < argc; i++)
      dstlen += strlen(argv[i]);
   char *comstr = new char[dstlen + 1];
   char *ret = comstr;
   ret[0] = 0;

   for(i = 0; i < argc; i++)
   {
      if(strchr(argv[i], ' '))
      {
         strcat(comstr, "\"");
         strcat(comstr, argv[i]);
         strcat(comstr, "\" ");
      }
      else
      {
         strcat(comstr, argv[i]);
         strcat(comstr, " ");
      }
   }
   return ret;
}

//------------------------------------------------------------------------------
bool SimInputPlugin::parseSimInputEvent(int argc, const char *argv[], SimInputEvent *event)
{
   // argument format:
   // bindAction keyboard[0-n] [make, break] {ctrl, alt, shift} {ascii, keyName}
   // bindAction joystick[0-n] [make, break] {ctrl, alt, shift} button[0-n]
   // bindAction joystick[0-n] move[x,y,z,r,u,v] 
   // bindAction mouse[0-n] [make, break] {ctrl, alt, shift} button[0-n]
   // bindAction mouse[0-n] move[x,y,z,r,u,v] 

   if (!argc) return (false);

   // get device type/inst
   event->deviceType = getPairType(DeviceTypes, *argv);
   event->deviceInst = getInst(*argv);
   argc--; argv++;
   if (!argc || event->deviceType == 0) return (false);

   // get action
   event->action = getPairType(ActionTypes, *argv);
   if (event->action == SI_MAKE || event->action == SI_BREAK)
   {
      argc--; 
      argv++;
   }
   if (!argc) return (false);

   // get modifier keys
   getModifiers(argc, argv, event);
   
   // check if we want an event from this modifier key
   if( !argc ) 
   {
      // has to be keyboard - make or break
      if( ( event->deviceType != SI_KEYBOARD ) || 
         ( ( event->action != SI_MAKE ) && 
         ( event->action != SI_BREAK ) ) )
         return( false );
      
      // check if we are wanting the control/alt/shift key's bound
      // -- individual r/l[key] are grabbed through the getDIK_CODE below
      switch( event->modifier )
      {
         case SI_SHIFT:
         case SI_ALT:
         case SI_CTRL:
         
            // set as a key and as an invalid entry into the 
            // keyboard list 
            event->objType = SI_KEY;
            event->objInst = 0;
            
            return( true );
            
         default:
            return( false );
      }
   }

   // process button, key or axis
   switch ( event->action )
   {
      case SI_MAKE:
      case SI_BREAK:
         if (strnicmp("button", *argv, 6) == 0)
         {  // was a button
            event->objType = SI_BUTTON;
            event->objInst = getInst(*argv);
         }
         else if (strlen(*argv) >= 4 && strnicmp("pov", *argv+1, 3) == 0)
         {  // was POV button
            event->objType = getPairType(ObjTypes, *argv);
            event->objInst = getInst(*argv);
         }
         else 
         {  // was a key
            event->objType = SI_KEY;
            if (strlen(*argv) == 1)
            {  // ascii key
               event->objInst = SimInputManager::getDIK_CODE( **argv );
               if (!event->objInst)
                  return (false);
            }
            else
            {  // string key name
               event->objInst = SimInputManager::getDIK_CODE( *argv );
               if (!event->objInst)
                  return (false);
            }
         }
         break;

      case SI_MOVE:
      default:
         {
            // must have been an object and not an action
            event->action  = SI_MOVE;
            event->objType = getPairType(ObjTypes, *argv);
            event->objInst = getInst(*argv);
            if (!event->objType)
               return (false);
         }
         break;
   }
   return (true);
}   


//------------------------------------------------------------------------------
static struct ActionFlagDef
{
    char  *name;
    Int32  flag;
}ActionFlag[] = 
{
    { "None",     SimActionFlagNone },
    { "Value",    SimActionFlagValue },
    { "Flip",     SimActionFlagFlip },
    { "Center",   SimActionFlagCenter },
    { "Square",   SimActionFlagSquare },
    { "Cube",     SimActionFlagCube },
    { "DeadZone", SimActionFlagDeadZone },
    { "Scale",    SimActionFlagScale }
};
#define NUM_ACTION_FLAGS ( sizeof(ActionFlag)/sizeof(ActionFlagDef) )


//------------------------------------------------------------------------------
bool SimInputPlugin::parseSimActionEvent(int argc, const char *argv[], SimActionEvent *action, BitSet32 *flags)
{
   if (!argc) return (false);

   action->action = atoi(*argv);
   if (!action->action) return (false);
   argc--; argv++;

   flags->clear();
   action->fValue = 0.0f;
   action->deadZone = 0.0f;
   action->scale = 1.0f;

   if (argc)
   {
      if (sscanf(*argv, "%f", &action->fValue))
      {
         flags->set(SimActionFlagValue);
         argc--; argv++;
      }
   }
   
   for (; argc; argc--, argv++)
   {
      int i;
      for (i=0; i<NUM_ACTION_FLAGS; i++)
      {
         if (stricmp(*argv, ActionFlag[i].name) == 0)
         {
            flags->set( ActionFlag[i].flag );

            // An extra argument for these guys
            if (ActionFlag[i].flag == SimActionFlagDeadZone) 
            {
               if (!--argc)
                  return false;

               float f = float(atof(*++argv));
               f = min (f, 1.f);

               action->deadZone = max(f, 0.0f);
            }
            else if (ActionFlag[i].flag == SimActionFlagScale) 
            {
               if (!--argc)
                  return false;

               float f = float(atof(*++argv));
               f = min (f, 1.f);

               action->scale = max(f, 0.0f);
            }

            break;
         }
      }
      if (i==NUM_ACTION_FLAGS)
         return (false);
   }

   return (true);
}   


extern void expandEscape(char *dest, const char *src);

//------------------------------------------------------------------------------
bool SimInputPlugin::writeActionMap(StreamIO &s, SimActionMap *actionMap)
{
   SimActionMap::Map::iterator i = actionMap->map.begin();
   for (; i != actionMap->map.end(); i++)
   {
      char buffer[1024];

      if(i->action.consoleCommand)
         strcpy(buffer, "bindCommand(");
      else
         strcpy(buffer, "bindAction(");
      strcat(buffer, avar("%s%d, ",
          getPairStr(DeviceTypes, i->event.deviceType),
          i->event.deviceInst) );

      if (i->event.action == SI_MAKE || i->event.action == SI_BREAK)
      {
         //ISA key
         strcat(buffer, avar("%s, ", getPairStr(ActionTypes, i->event.action)) );
         getModifiers(buffer, i->event.modifier);
         if (i->event.objType == SI_KEY)
         {
            // check for a modifier
            if( i->event.objInst )
            {
               const DIKCode *code = SimInputManager::getDIK_STRUCT(i->event.objInst);
               if (code->name)
                  strcat(buffer, avar("\"%s\", ", code->name ));
               else
               {
                  // check for backslash
                  if(code->lower == '\\')
                     strcat(buffer,"\"\\\\\", ");
                  else
                     strcat(buffer, avar("\"%c\", ", code->lower ));
               }
            }
         }
         else
         {
            // ISA button
            strcat( buffer, avar("%s%d, ",
               getPairStr(ObjTypes, i->event.objType),
               i->event.objInst ) );
         }
      }
      else
      {
            // ISA move object
            strcat( buffer, avar("%s%d, ",
               getPairStr(ObjTypes, i->event.objType),
               i->event.objInst ) );
      }

      strcat(buffer, "TO, ");
      if(i->action.consoleCommand)
      {
         char tbuf[1024];
         expandEscape(tbuf, i->action.consoleCommand);
         strcat(buffer, avar("\"%s\"", tbuf));
      }
      else
      {      
         strcat(buffer, SimTagDictionary::getDefineString(i->action.action));

         BitSet32 flags = i->flags;
         if (flags.test(SimActionFlagValue))
            strcat(buffer, avar(", %f", i->action.fValue));

         for (int n=0; n<NUM_ACTION_FLAGS; n++)
            if (flags.test(ActionFlag[n].flag) && !flags.test(SimActionFlagValue)) {
               strcat(buffer, avar(", %s", ActionFlag[n].name));
               if (ActionFlag[n].flag == SimActionFlagScale)
                  strcat(buffer, avar(", %f", i->action.scale));
               else
                  if (ActionFlag[n].flag == SimActionFlagDeadZone)
                     strcat(buffer, avar(", %f", i->action.deadZone));
            }
      }
      strcat(buffer, ");\r\n");
      s.write( strlen(buffer), buffer );
   }      
   return (true);
}   

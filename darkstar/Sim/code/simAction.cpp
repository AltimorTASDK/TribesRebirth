//--------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------

#include <sim.h>
#include "new.h"        //needed for in-place new
#include "simBase.h"
#include "simAction.h"
#include "console.h"
#include "simGame.h"

//------------------------------------------------------------------------------
// see if a system key ( 'a' always is... ) matches a input device event
static bool compareSystemKeyEvents( const SimInputEvent * a, const SimInputEvent * b, 
   const bool checkAction, const bool checkBound )
{
   // check that these events are possible matches.. compare all but objInst
   bool result = (a->deviceType == b->deviceType)
      && (a->deviceInst == b->deviceInst)
      && (a->objType == b->objType);

   // we know that we found a correct event pairing at this point
   if( !result )
      return( false );

   // check for exact matches... if !checkaction, then a is make and
   // b is break
   if( checkAction && ( a->action != b->action ) )
      return( false );

   // see if this is a test if the key is already bound..
   if( checkBound )
   {
      if( a->objInst || b->objInst || ( a->modifier != b->modifier ) )
         return( false );
      return( true );   
   }
      
   // now check for the system keys
   switch( a->modifier )
   {
      case SI_SHIFT:
         if( ( b->objInst == DIK_LSHIFT ) || ( b->objInst == DIK_RSHIFT ) )
            return( true );
         break;

      case SI_CTRL:
         if( ( b->objInst == DIK_LCONTROL ) || ( b->objInst == DIK_RCONTROL ) )
            return( true );
         break;

      case SI_ALT:
         if( ( b->objInst == DIK_LMENU ) || ( b->objInst == DIK_RMENU ) )
            return( true );
         break;
   }
         
   return( false );
}

//------------------------------------------------------------------------------
// check that this event is a modifier type key event ( cntl,alt,shift...)
// that is not assigned a single key ( so control and not rcontrol... )
static bool isSystemKeyEvent( const SimInputEvent * e )
{
   // check for an instance of 0, from the keyboad, make/break,
   // a key, and needs to have a modifier flag ( actual modifier
   // is checked in compareSystemKeyEvents )
   return( ( e->deviceType == SI_KEYBOARD ) && ( e->modifier ) &&
      ( !e->objInst ) && ( e->objType == SI_KEY ) &&
      ( ( e->action == SI_MAKE ) || ( e->action == SI_BREAK ) ) );
}

//------------------------------------------------------------------------------
bool SimActionMap::compareModifiers(const SimInputEvent *a, const SimInputEvent *b)
{
   static bool lookup[4][4] =  
   {  {1,0,0,0},        // I just reduced all the possible combinations into
      {0,1,0,1},        // a quick lookup table
      {0,0,1,1},
      {0,1,1,1} };

   return (lookup[a->modifier & SI_SHIFT][b->modifier & SI_SHIFT]
            && lookup[(a->modifier & SI_CTRL) >>2][(b->modifier & SI_CTRL) >>2]
            && lookup[(a->modifier & SI_ALT) >>4 ][(b->modifier & SI_ALT) >>4 ]
         );
}  
 
//------------------------------------------------------------------------------
static bool compareInputEvent(const SimInputEvent *a, const SimInputEvent *b, const bool boundCheck )
{
   // event a is from the action map, b is from input device
   //  - check if we are testing against a generic system key...
   if( isSystemKeyEvent( a ) )
      return( compareSystemKeyEvents( a, b, true, boundCheck ) );
   
  bool result = (a->deviceType == b->deviceType)
             && (a->deviceInst == b->deviceInst)
             && (a->objType == b->objType)
             && (a->action  == b->action)
             && (a->objInst == b->objInst);
   if (result && a->deviceType == SI_KEYBOARD)   
      return SimActionMap::compareModifiers(a, b);
   else return result;
}   

//------------------------------------------------------------------------------
bool SimActionMap::isMakeBreakPair(const SimInputEvent *a, const SimInputEvent *b)
{
   if (a->action == SI_MAKE && b->action == SI_BREAK)
   {
      // check for a system modifier key we are testing for
      //  - event a is from the action map, b is from input device
      if( isSystemKeyEvent( a ) )
         return( compareSystemKeyEvents( a, b, false, false ) );

      return ( (a->deviceType == b->deviceType)
            && (a->deviceInst == b->deviceInst)
            && (a->objType == b->objType)
            && (a->objInst == b->objInst)  );
   }
   return (false);
}   


//------------------------------------------------------------------------------
SimActionEvent::SimActionEvent()
{
   type = SimActionEventType;
   consoleCommand = NULL;
}

SimActionEvent::SimActionEvent(const SimActionEvent &event)
{
   *this = event;
}

SimActionEvent &SimActionEvent::operator=(const SimActionEvent &event)
{
   action = event.action;
   device = event.device;
   fValue = event.fValue;
   deadZone = event.deadZone;
   scale = event.scale;
   type = SimActionEventType;

   if(event.consoleCommand)
   {
      char *newCmd = new char[strlen(event.consoleCommand) + 1];
      strcpy(newCmd, event.consoleCommand);
      consoleCommand = newCmd;
   }
   else
      consoleCommand = NULL;
   return *this;
}

SimActionEvent::~SimActionEvent()
{
   if(consoleCommand)
      delete[] consoleCommand;
   consoleCommand = NULL;
}

static ResourceTypeSimActionMap	_ressae(".sae");

//--------------------------------------------------------------------------------

static int actionIdLookup(char *item, ActionName **table, int size)
{
   int i;
   for ( i=0; i<size; i++, table++)
   {
      if (strcmp(item, (*table)->name) == 0)
         return ( (*table)->action );
   }
   
   // the item wasn't in the table, let's see if it is actually an int
   if (sscanf(item, "%d", &i) == 1)
     return (i);

   // oh well, guess we couldn't decode it
   return (-1);
}


static char* actionNameLookup(int id, ActionName **table, int size)
{
   for ( int i=0; i<size; i++, table++)
   {
      if ( id == (*table)->action )
         return ( (*table)->name );
   }
   return ( NULL );
}


//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
//	SimActionEvent 
//  Persistent Functions
//---------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS(SimActionEvent, FOURCC('A','C','E','V'),
   SimActionEventType);

//---------------------------------------------------------------------------
ActionName **SimActionEvent::actionTable = NULL;
int SimActionEvent::actionTableSize = 0;

//---------------------------------------------------------------------------
Persistent::Base::Error SimActionEvent::read(StreamIO &sio, int version, int user)
{
	if (SimEvent::read(sio, version, user) != Ok) return(ReadError);
	if (!sio.read(&action))                       return(ReadError);
	if (!sio.read(&fValue))                       return(ReadError);   

	return(Ok);
}

//---------------------------------------------------------------------------
Persistent::Base::Error SimActionEvent::write(StreamIO &sio, int version, int user)
{
	if (SimEvent::write(sio, version, user) != Ok) return(WriteError);
	if (!sio.write(action))                        return(WriteError);
	if (!sio.write(fValue))                        return(WriteError);   
	return(Ok);
}



//--------------------------------------------------------------------------------
//	SimActionMap
//  Persistent Functions
//---------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAG(SimActionMap, FOURCC('A','C','M','A'));


//---------------------------------------------------------------------------
Persistent::Base::Error SimActionMap::read(StreamIO &sio, int version, int user)
{
	int mapSize;
	BitSet32 flags;
	SimInputEvent           event;
	SimActionEvent          action;

	// read the number of map entries that will be coming in
	if (!sio.read(&mapSize)) return(ReadError);
	// read each map entry
	for (int i = 0; i < mapSize; i++)
	{
      if (event.read(sio, version, user) != Ok)    return (ReadError);
      if (action.read(sio, version, user) != Ok)   return (ReadError);
      if (!sio.read(sizeof(BitSet32), &flags))     return (ReadError);

		// add the entry to the map
		mapEvent(&event, &action, flags);
	}
	return(Ok);
}


//---------------------------------------------------------------------------
Persistent::Base::Error SimActionMap::write(StreamIO &sio, int version, int user)
{
	int mapSize = map.size();

	// write an indication of how large the map is
	if (!sio.write(mapSize)) return(WriteError);

	// write out each individual Node in the map
	for (int i = 0; i < mapSize; i++)
	{
      if (map[i].event.write(sio, version, user) != Ok)    return (WriteError);
      if (map[i].action.write(sio, version, user) != Ok)   return (WriteError);
      if (!sio.write(sizeof(BitSet32), &map[i].flags))                                return (WriteError);
	}
	return(Ok);
}


//---------------------------------------------------------------------------
struct ActionFlagEntry
{
    char  *name;
    Int32  flag;
};
static ActionFlagEntry actionFlagTable[] = 
{
    "ActionFlagNone",     SimActionFlagNone,
    "ActionFlagValue",    SimActionFlagValue,
    "ActionFlagFlip",     SimActionFlagFlip,
    "ActionFlagCenter",   SimActionFlagCenter,
    "ActionFlagSquare",   SimActionFlagSquare,
    "ActionFlagCube",     SimActionFlagCube,
    "ActionFlagDeadZone", SimActionFlagDeadZone
};
#define actionFlagTableSize ( sizeof(actionFlagTable)/sizeof(ActionFlagEntry) )


//------------------------------------------------------------------------------
SimActionMap* SimActionMap::resLoad (StreamIO &strm)
{
	Persistent::Base::Error err;
	Persistent::Base* bptr;
	
	bptr = Persistent::Base::load(strm,&err);

	if (bptr)
		return (dynamic_cast<SimActionMap *>(bptr));
	else
	   return (NULL);
}


//--------------------------------------------------------------------------------
SimActionMap::Node* SimActionMap::findMatch(const SimInputEvent *event, bool boundCheck )
{
   if (!checkDevice(event->deviceType)) return (NULL);

   Node *node = map.begin();
   for(;node != map.end(); node++)
   {
      if (compareInputEvent(&node->event, event, boundCheck ))
         return (node);
   }   
   return (NULL);
}

//--------------------------------------------------------------------------------
SimActionMap::SimActionMap()
{
   deviceTypeMask = 0;
}

SimActionMap::~SimActionMap()
{
   clear();
}

void SimActionMap::clear()
{
   for(Node *node = map.begin();node != map.end(); node++)
      node->~Node();
   map.clear();
}

//------------------------------------------------------------------------------
void SimActionMap::_mapEvent(const SimInputEvent *event, const SimActionEvent *action, BitSet32 flags)
{
   map.increment();                 
   Node *node   = (map.end()-1);    
   node->event  = *event;
   node->action = *action;
   node->flags  = flags;
   node->extraDataA = NULL;
   node->extraDataB = NULL;
   addDeviceType(event->deviceType);
}

//------------------------------------------------------------------------------
void SimActionMap::mapEvent(const SimInputEvent *event, const SimActionEvent *action, BitSet32 flags)
{
   unmapEvent( event );
   _mapEvent(event, action, flags);
}


//------------------------------------------------------------------------------
void SimActionMap::unmapEvent(const SimInputEvent *event)
{
   Node *iter = findMatch( event, true );
   if (iter)                        //if the key exists... destroy it!
   {
      // erase doesn't actually destroy the object
      iter->~Node();

      map.erase( iter );

      //rebuild the deviceTypeMask
      deviceTypeMask = 0;
      for (Node *i=map.begin(); i != map.end(); i++)
         addDeviceType( i->event.deviceType );
   }
}


//--------------------------------------------------------------------------------

SimActionHandler::SimActionHandler()
{
   id = SimActionHandlerId;
}


//------------------------------------------------------------------------------
void SimActionHandler::setActionValue(SimActionEvent *action, const SimInputEvent *event, const SimActionMap::Node &node)
{
   // if the SimActionFlagValue flag is not set, use the value from the InputEvent
   if (!node.flags.test(SimActionFlagValue) )
      action->fValue = event->fValue;
   else
      action->fValue = node.action.fValue;
}

//------------------------------------------------------------------------------
void SimActionHandler::scaleActionValue(SimActionEvent *action, const SimInputEvent *event, const SimActionMap::Node &node)
{
   // If action value present, exit
   if ( node.flags.test(SimActionFlagValue) ) return;

   // The order of flag processing is important, be careful.
   // CENTER
	if ( node.flags.test(SimActionFlagCenter) && event->deviceType != SI_MOUSE )
		action->fValue = 1.0f - (action->fValue * 2.0f);
	
   // DEADZONE
	if ( node.flags.test(SimActionFlagDeadZone))
		if (action->fValue > 0.0f) 
		{
			if (action->fValue < node.action.deadZone)
				action->fValue = 0.0f;
         else
         {
            action->fValue -= node.action.deadZone;
            action->fValue *= 1.0/(1.0-node.action.deadZone);
         }
		}
		else
			if (action->fValue > -node.action.deadZone)
				action->fValue = 0.0f;
         else
         {
            action->fValue += node.action.deadZone;
            action->fValue *= 1.0/(1.0-node.action.deadZone);
         }

   // FLIP
	if ( node.flags.test(SimActionFlagFlip) )
	{
      if (!node.flags.test(SimActionFlagCenter) && event->deviceType != SI_MOUSE)
		   action->fValue = 1.0f - action->fValue;
      else
		   action->fValue = -action->fValue;
	}

   // SQUARE
	if ( node.flags.test(SimActionFlagSquare) ) {
		bool neg = action->fValue < 0.0f;
		action->fValue = action->fValue * action->fValue;
		if (neg) action->fValue = -action->fValue;
	}

   // CUBE
	if ( node.flags.test(SimActionFlagCube) ) 
		action->fValue = action->fValue * action->fValue * action->fValue;

   // SCALE
	if ( node.flags.test( SimActionFlagScale ) )
		action->fValue *= node.action.scale;
}   


//------------------------------------------------------------------------------
const SimInputEvent* SimActionHandler::updateMakeList(const SimInputEvent *event)
{
   static SimInputEvent brk;
   if (event->action == SI_MAKE)
   {
      makeList.push_back(*event);
      SimGame::get()->getInputManager()->addMakeEvent((SimInputEvent *) event);
      return (event);
   }

   if (event->action == SI_BREAK)
   {
      // in order for a BREAK to be passed through the system
      // a matching MAKE must be located.
      int i;
      for(i = 0; i < makeList.size(); i++)
         if (SimActionMap::isMakeBreakPair(&makeList[i], event))
         {
            brk = *event;
            brk.modifier = makeList[i].modifier;
            makeList.erase(i);
            SimGame::get()->getInputManager()->removeMakeEvent(i);
            
            return (&brk);
         }
      return (NULL);
   }
   return (event);
}   


//------------------------------------------------------------------------------
bool SimActionHandler::onSimInputEvent(const SimInputEvent *ev)
{
   const SimInputEvent *event = updateMakeList(ev);
   if (!event)
      return (false);

   static SimInputEvent evt;

   //see if there is a map in the stack that's interested
   for(;;)
   {
      MapList::iterator map = mapList.begin();
      for (; map != mapList.end(); map++)
      {
         SimActionMap::Node *node;
         node = map->map->findMatch( event );
         if(node)
         {
            if(node->action.consoleCommand)
            {
               CMDConsole::getLocked()->evaluate(node->action.consoleCommand, false);
               return true;
            }
            else
            {
               SimActionEvent action;

               action.action = node->action.action;
               action.time   = manager->getCurrentTime();

               switch (event->action)
               {
                  case SI_MAKE:
                  case SI_BREAK:
                     action.device = SimActionTypeDigital;
                     break;
                  case SI_MOVE:
                  default:
                     if (event->deviceType == SI_MOUSE)
                        action.device = SimActionTypeDelta;
                     else
                     {
                        if (event->objType == SI_POV || 
                           event->objType == SI_XPOV ||
                           event->objType == SI_YPOV)
                           action.device = SimActionTypeDigital|SimActionTypePOV;
                        else
                           action.device = SimActionTypeAnalog;
                     }
                     break;
               }

               setActionValue(&action, event, *node);
               scaleActionValue(&action, event, *node);

               if ( map->object->isRemoved() || map->object->processEvent( &action ) )
                  return (true);
            }
         }
      }
      if((event->action != SI_MAKE && event->action != SI_BREAK) || event->modifier == 0)
         break;

      evt = *event;
      event = &evt;
      evt.modifier = 0;
   }
   return (false);
}


//------------------------------------------------------------------------------
bool SimActionHandler::onAdd()
{
   addToSet(SimInputConsumerSetId);
   return Parent::onAdd();
}   


//------------------------------------------------------------------------------
void SimActionHandler::onDeleteNotify(SimObject *object)
{
    _pop( object );
    Parent::onDeleteNotify(object);
}   


//------------------------------------------------------------------------------
bool SimActionHandler::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimInputEvent);
   }
   return (false);
}

//------------------------------------------------------------------------------
bool SimActionHandler::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
      q->priority = SI_PRIORITY_NORMAL;
      return (true);
   }
   return (false);
}

//------------------------------------------------------------------------------
void SimActionHandler::breakKey(SimInputEvent *event)
{
   // NOTE updateMakeList will remove the corresponding make 
   // from the make list.
   SimInputEvent brk = *event;
   brk.action = SI_BREAK;
   brk.fValue = 0.0f;
   onSimInputEvent(&brk);        
}   


//------------------------------------------------------------------------------
void SimActionHandler::breakAllMadeKeys()
{
   while (makeList.size())
      breakKey(&makeList.last());
}   


//------------------------------------------------------------------------------
void SimActionHandler::breakMadeKeys(SimActionMap *actionMap)
{
   SimInputEventList::iterator i = makeList.begin();
   for (; i != makeList.end(); i++)
   {
      if (actionMap->findMatch(i))   
      {
         MapList::iterator map = mapList.begin();
         for (; map != mapList.end(); map++)
         {
            if (map->map == actionMap)
            {  // looks like the make belongs to actionMap: close it
               breakKey(i);
					i--;
               break;
            }
            else
            {  // the make does not belong to actionMap: next make
               if (map->map->findMatch(i))
                  break;
            }
         }
      }
   }
}   


//------------------------------------------------------------------------------
void SimActionHandler::push( SimObject *object, SimActionMap *actionMap)
{
   //push a map
   pop( object, actionMap );     //remove it if it already exists
   
   // scan through the make list and break any keys that this map overrides
   SimInputEventList::iterator i = makeList.begin();
   for (; i != makeList.end(); )
   {
      if (actionMap->findMatch(i))   
         breakKey(i);
      else
         i++;
   }

   MapObjectPair pair( object, actionMap );
   mapList.push_front( pair );

   deleteNotify(object);
}


//------------------------------------------------------------------------------
void SimActionHandler::pop( SimObject *object, SimActionMap *actionMap )
{
   int mapCount = 0;
   int mapsRemoved = 0;

   //remove a specific map associated with a SimObject
   MapList::iterator map = mapList.begin();
   while(map != mapList.end())
   {
      if ( map->object == object )
      {
         if (map->map == actionMap)
         {
            breakMadeKeys(map->map);
            mapList.erase( map );
            mapsRemoved++;
            continue;
         }
         else
            mapCount++;
      }
      map++;
   }

   if (mapsRemoved && (mapCount == 0))
      clearNotify(object);
}


//------------------------------------------------------------------------------
int SimActionHandler::_pop( SimObject *object )
{
   int mapCount = 0;

   //remove all maps associated with a particular SimObject
   MapList::iterator map = mapList.begin();
   while(map != mapList.end())
   {
      if ( map->object == object )
      {
         breakMadeKeys(map->map);
         mapList.erase( map );
         mapCount++;
         continue;
      }
      map++;
   }

   if (mapCount != 0)
      clearNotify(object);

   return (mapCount);
}

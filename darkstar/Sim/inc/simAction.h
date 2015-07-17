//--------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------

#ifndef _SIMACTION_H_
#define _SIMACTION_H_

#include <ResManager.h>
#include <tVector.h>
#include <simbase.h>
#include <simev.h>
#include <simInput.h>
#include "genericActions.h"

#define SimActionFlagNone              (1<<0)
#define SimActionFlagValue             (1<<1)
#define SimActionFlagFlip              (1<<2)
#define SimActionFlagCenter            (1<<3)
#define SimActionFlagSquare            (1<<4)
#define SimActionFlagCube              (1<<5)
#define SimActionFlagDeadZone          (1<<6)   // fValue is += dead zone
#define SimActionFlagScale			      (1<<7)   // scale input value by action value

#define SimActionTypeAnalog      (1<<0)   // action value has analog properties
#define SimActionTypeDigital     (1<<1)   // action value is ON/OFF
#define SimActionTypeDelta       (1<<2)   // action value represents delta movements
#define SimActionTypePOV         (1<<3)   // not actually a type itself is helpful to know


struct ActionName
{
	int action;
	char* name;
};


//----------------------------------------------------------------------------
// Event type SimActionEventType declared in simevdcl.h
//
class DLLAPI SimActionEvent : public SimEvent
{
  public:
	SimActionEvent();
	SimActionEvent(const SimActionEvent &event);
   ~SimActionEvent();
   SimActionEvent &operator=(const SimActionEvent &event);

   static ActionName **actionTable;
   static int         actionTableSize;
   int   action;
   int   device;
	float fValue;
	float deadZone;
	float scale;
   char *consoleCommand;

	// Persistent functions
	DECLARE_PERSISTENT(SimActionEvent);
	static bool idFunction(StreamIO &sio);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
};

//------------------------------------------------------------------------------
class DLLAPI SimActionMap : public Persistent::Base
{
private:
   void _mapEvent(const SimInputEvent *event, const SimActionEvent *action, BitSet32 flags);

public:
   struct Node
   {
      BitSet32       flags;
      SimActionEvent action;   
      SimInputEvent  event;
      void *extraDataA;  // optional pointers which can be used for
      void *extraDataB;  // various other information (ie, to aid in
                         // gui configuration)
   
   };
   typedef Vector<Node> Map;
   Map map;

   SimObject *object;   
   DWORD deviceTypeMask;

   SimActionMap();
   ~SimActionMap();
   void mapEvent(const SimInputEvent *event, const SimActionEvent *action, BitSet32 flags);
   void unmapEvent(const SimInputEvent *event);
   int  size()   { return (map.size()); }
   void clear();
   void addDeviceType(BYTE deviceType) { deviceTypeMask |= (1<<deviceType); };
   bool checkDevice(BYTE deviceType)   { return (deviceTypeMask & (1<<deviceType)); };

	static SimActionMap* resLoad (StreamIO &);
   SimActionMap::Node* findMatch(const SimInputEvent *event, bool boundCheck = false );

   static bool isMakeBreakPair(const SimInputEvent *makeEv, const SimInputEvent *breakEv);
   static bool compareModifiers(const SimInputEvent *a, const SimInputEvent *b);

	// Persistent functions
	DECLARE_PERSISTENT(SimActionMap);
	static bool idFunction(StreamIO &sio);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
};   


//------------------------------------------------------------------------------
class DLLAPI ResourceTypeSimActionMap: public ResourceType
{
public:   
   ResourceTypeSimActionMap(const char *ext = ".sae"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int) { return (void*)SimActionMap::resLoad(*stream); }
   void destruct(void *p)            { delete (SimActionMap*)p; }
};   


//----------------------------------------------------------------------------
class DLLAPI SimActionHandler: public SimObject
{
   typedef SimObject Parent;
private:
   struct MapObjectPair
   {
      MapObjectPair(SimObject *o, SimActionMap *m)  { object = o; map = m; }
      SimActionMap *map;
      SimObject    *object;   
   };
   typedef Vector<MapObjectPair> MapList;
   MapList mapList;

   typedef Vector<SimInputEvent> SimInputEventList;
   SimInputEventList makeList;

   int _pop( SimObject *object );
   void setActionValue(SimActionEvent *action, const SimInputEvent *event, const SimActionMap::Node &node);
   void scaleActionValue(SimActionEvent *action, const SimInputEvent *event, const SimActionMap::Node &node);

   const SimInputEvent* updateMakeList(const SimInputEvent *event);
   void breakMadeKeys(SimActionMap *actionMap);
   void breakKey(SimInputEvent *event);


public:
	SimActionHandler();

   static SimActionHandler* find( SimManager *manager );

	bool processQuery(SimQuery*);
	bool processEvent(const SimEvent*);
   bool onSimInputEvent(const SimInputEvent *event);

   bool onAdd();
   void onDeleteNotify(SimObject *object);

   void push( SimObject *object, SimActionMap *map);
   void pop( SimObject *object, SimActionMap *map );
   void pop( SimObject *object );
   
   void breakAllMadeKeys();
};

inline SimActionHandler* SimActionHandler::find( SimManager *manager )
{
   return ( static_cast<SimActionHandler*>(manager->findObject(SimActionHandlerId)) );
}

inline void SimActionHandler::pop( SimObject *object)
{
   if (_pop( object ))
      clearNotify(object);
}

#endif //_ACTION_H_

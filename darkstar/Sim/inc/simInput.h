//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SimInput_H_
#define _SimInput_H_


#include <dInput.h>
#include <simBase.h>
#include <simEv.h>
#include <simInputDevice.h>
#include <sim3Dev.h>


#define DIK_LAST DIK_APPS     // greatest DIK code defined

#define SI_PRIORITY_INPUT_DETECT (5000)    // 
#define SI_PRIORITY_FILTER       (4000)    // filters
#define SI_PRIORITY_HIGH         (3000)    // simCanvas mouse traps
#define SI_PRIORITY_NORMAL       (2000)    // simActionHandler
#define SI_PRIORITY_LOW          (1000)


#define SI_DEBUG 0            // enables error echoing to console if == 1

class SimActionHandler;

//------------------------------------------------------------------------------
class SimInputEvent: public SimEvent
{
public:
   SimInputEvent() { type = SimInputEventType; }
   DWORD deviceInst; // device instance joystick0, joystick1 etc
   float fValue;     // value -1.0 to 1.0
   BYTE deviceType;  // mouse, keyboard, joystick, device( ie unidentified)
   BYTE objType;     // SI_XAXIS, SI_BUTTON, SI_KEY ...
   BYTE objInst;     // which type instance OR DIK_CODE
   BYTE action;      // MAKE/BREAK/MOVE
   BYTE ascii;       // ascii character code 'a', 'A', 'b', '*', etc (if device==keyboard)
   BYTE modifier;    // SI_LSHIFT, etc

   // Persistent I/O functions
   DECLARE_PERSISTENT(SimInputEvent);
   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user); 
};   


//------------------------------------------------------------------------------
struct SimInputActivateEvent: public SimEvent
{
   SimInputActivateEvent() { type = SimInputActivateEventType; }
   BYTE deviceType;  // mouse, keyboard, joystick, device( ie unidentified)
   BYTE deviceInst;  // device instance joystick0, joystick1 etc
   bool activate;    // activate or deactivate
};


//------------------------------------------------------------------------------
class SimInputPriorityQuery: public SimQuery
{
public :
   int priority;
   SimInputPriorityQuery() 
   { 
      type = SimInputPriorityQueryType; 
      priority = SI_PRIORITY_NORMAL;
   }   
};   


//------------------------------------------------------------------------------
struct DIKCode
{
   BYTE lower;
   BYTE upper;
   char *name;
};

extern DIKCode DIKCodeList[DIK_LAST+1];

//------------------------------------------------------------------------------
class SimInputConsumerSet: public SimSet
{
private:
   typedef SimSet Parent;
   enum {
      ATTACH_DELAY = 2,    // n seconds before reattach attempt after  
   };                      // losing track of SimInputManager
   bool attached;
   bool sorted;
   void sort();
   int  getPriority(SimObject *object);

public:
   SimInputConsumerSet();

   SimObject* addObject(SimObject*);

   void resort() { sorted = false; }
   bool processEvent(const SimEvent*);
   bool onSimInputEvent(const SimInputEvent *event);
   bool onSim3DMouseEvent(const Sim3DMouseEvent *event);
};   


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class SimInputManager: public SimGroup
{
private:
   typedef SimGroup Parent;
   typedef HRESULT (WINAPI *FN_DirectInputCreate)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT * lplpDirectInput,  LPUNKNOWN punkOuter);
   HMODULE hDll;
   FN_DirectInputCreate fnDirectInputCreate;
   IDirectInput *pDirectInput;
   SimObject    *consumer;
   SimActionHandler *simActionHandler;
   Vector<SimInputEvent> makeList;
   
   BYTE modifierKeys;

   static BOOL CALLBACK EnumDevicesProc( const DIDEVICEINSTANCE *pddi, LPVOID pvRef );

public:
   SimInputManager();
   ~SimInputManager();
   
   IDirectInput* getDInput();

   bool open();
   void close();
   void enumerate();

   void capture();   // capture all devices
   void release();   // release all devices
   
   void activate(BYTE deviceType, BYTE device);
   void deactivate(BYTE deviceType, BYTE device);

   void process();
   bool processEvent(const SimEvent*);

   void onDeleteNotify(SimObject *object);
   void addMakeEvent(SimInputEvent *ev);
   void removeMakeEvent(int i);
   void onRemove();
   bool onAdd();

   bool onSimInputEvent(const SimInputEvent *event);
   bool onSimInputActivateEvent(const SimInputActivateEvent *event);

   BYTE getModifierKeys();

   static BYTE getAscii(BYTE dik, BYTE mod);
   static BYTE getDIK_CODE(const char *str);
   static BYTE getDIK_CODE(const char c);
   static const DIKCode* getDIK_STRUCT(BYTE dik);

   static SimInputManager* find(SimManager *manager);
};


//------------------------------------------------------------------------------
inline IDirectInput* SimInputManager::getDInput()
{
   return (pDirectInput);
}   


inline SimInputManager* SimInputManager::find(SimManager *manager)
{
   return static_cast<SimInputManager*>
      (manager->findObject(SimInputManagerId));
}

inline BYTE SimInputManager::getModifierKeys()
{
   return modifierKeys;
}

#if SI_DEBUG
#define echoResult(r,p) { AssertWarn(0, _echoResult(r,p)); }
char* _echoResult(HRESULT r, const char *pre);
#else
#define echoResult(r,p) {}
#endif

#endif //_SimInput_H_

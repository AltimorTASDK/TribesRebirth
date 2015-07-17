//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <console.h>
#include <SimInputDevice.h>
#include <SimInput.h>
#include <simAction.h>


IMPLEMENT_PERSISTENT_TAG(SimInputEvent, FOURCC('S','I','E','V'));


//---------------------------------------------------------------------------
Persistent::Base::Error SimInputEvent::read(StreamIO &sio, int version, int user)
{
   if (SimEvent::read(sio, version, user) != Ok) return(ReadError);
   if (!sio.read(&deviceType))   return(ReadError);
   if (!sio.read(&deviceInst))   return(ReadError);
   if (!sio.read(&objType))      return(ReadError);
   if (!sio.read(&objInst))      return(ReadError);
   if (!sio.read(&action))       return(ReadError);
   if (!sio.read(&ascii))        return(ReadError);
   if (!sio.read(&modifier))     return(ReadError);
   if (!sio.read(&fValue))       return(ReadError);

   return(Ok);
}

//---------------------------------------------------------------------------
Persistent::Base::Error SimInputEvent::write(StreamIO &sio, int version, int user)
{
   if (SimEvent::write(sio, version, user) != Ok) return(WriteError);
   if (!sio.write(deviceType))    return(WriteError);
   if (!sio.write(deviceInst))    return(WriteError);
   if (!sio.write(objType))       return(WriteError);
   if (!sio.write(objInst))       return(WriteError);
   if (!sio.write(action))        return(WriteError);
   if (!sio.write(ascii))         return(WriteError);
   if (!sio.write(modifier))      return(WriteError);
   if (!sio.write(fValue))        return(WriteError);

   return Ok;
}


//------------------------------------------------------------------------------
SimInputConsumerSet::SimInputConsumerSet():
   SimSet( false )
{
   id = SimInputConsumerSetId;
   attached = false;
   sorted   = false;
   setStable(true);
}   


//------------------------------------------------------------------------------
SimObject* SimInputConsumerSet::addObject(SimObject* obj)
{
   Parent::addObject(obj);
   sorted = false;
   return ( obj );
}


//------------------------------------------------------------------------------
bool SimInputConsumerSet::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimInputEvent);
      onEvent(Sim3DMouseEvent);
   }
   return (Parent::processEvent(event));
}   

//------------------------------------------------------------------------------
bool SimInputConsumerSet::onSim3DMouseEvent(const Sim3DMouseEvent* event)
{
   for (iterator ptr = begin(); ptr != end(); ptr++)
      // first consumer to process it (ie return true) we stop.
      if ( (*ptr)->processEvent( event ) )   
         break;
   return (true);
}   


//------------------------------------------------------------------------------
bool SimInputConsumerSet::onSimInputEvent(const SimInputEvent *event)
{
   sort();   
   for (iterator ptr = begin(); ptr != end(); ptr++)
      // first consumer to process it (ie return true) we stop.
      if ( (*ptr)->processEvent( event ) )   
         break;
   return (true);
}   


//------------------------------------------------------------------------------
int SimInputConsumerSet::getPriority(SimObject *object)
{
   SimInputPriorityQuery query;
   object->processQuery(&query);
   return (query.priority);
}   


//------------------------------------------------------------------------------
void SimInputConsumerSet::sort()
{
   if (sorted) return;

   // I don't like that I have to query inside the sort but this does not 
   // happen very frequently

   // simple (STABLE) insertion sort by decending order
   int i,j;
   iterator obj = begin();
   for (j=0, i=1; i < size(); j=i, i++)
   {
      for(; j >= 0 && getPriority(obj[j]) < getPriority(obj[j+1]); j-- )
      {
         // swap
         SimObject *tmp = obj[j];
         obj[j]   = obj[j+1];
         obj[j+1] = tmp;
      }
   }
   sorted = true;
}   




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------ 
SimInputManager::SimInputManager()
{
   id = SimInputManagerId;
   fnDirectInputCreate = NULL;
   pDirectInput = NULL;
   hDll         = NULL;
   consumer     = NULL;
   simActionHandler = NULL;
   modifierKeys = 0;
   setStable(true);
}  


//------------------------------------------------------------------------------ 
SimInputManager::~SimInputManager()
{
   close();
}   


//------------------------------------------------------------------------------
bool SimInputManager::open()
{
   close();
   hDll = LoadLibrary("dInput.dll");
   if ( hDll )
   {
      fnDirectInputCreate = (FN_DirectInputCreate)GetProcAddress( hDll, "DirectInputCreateA");
      if ( fnDirectInputCreate )
      {
         if (fnDirectInputCreate( GetModuleHandle(NULL), 0x500, &pDirectInput, NULL ) == DI_OK)
         {
            SimInputDevice::isDX5 = true;
            enumerate();
            return (true);
         }
         if (fnDirectInputCreate( GetModuleHandle(NULL), 0x300, &pDirectInput, NULL ) == DI_OK)
         {
            SimInputDevice::isDX5 = false;
            enumerate();
            return (true);
         }
      }
   }
   close();
   return ( false );
   
}   


//------------------------------------------------------------------------------
void SimInputManager::close()
{
   // loop through all Devices now and delete then
   release();

   iterator ptr = begin();
   while( ptr != end() )
   {
      SimInputDevice *dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr) 
      {
         removeObject( dptr );                           // remove from this set
         if ( dptr->getManager() )                     // remove from manager
            dptr->getManager()->unregisterObject( dptr );
         delete dptr;
         ptr = begin();
      }
      else
         ptr++;
   }

   if ( pDirectInput ) 
   {
      pDirectInput->Release();
      pDirectInput = NULL;
   }

   if ( hDll ) 
   {
      fnDirectInputCreate = NULL;
      FreeLibrary( hDll );
      hDll = NULL;
   }
}   



//------------------------------------------------------------------------------
BOOL CALLBACK SimInputManager::EnumDevicesProc( const DIDEVICEINSTANCE *pddi, LPVOID pvRef )
{
   SimInputManager *parent = (SimInputManager*)pvRef;
   SimInputDevice *device  = new SimInputDevice(pddi);
   parent->addObject( device );
   if (!device->open())
   {
      parent->removeObject(device);
      delete device;
   }
   return (DIENUM_CONTINUE);
}   



//------------------------------------------------------------------------------
void SimInputManager::enumerate()
{
   if (pDirectInput)  
   {
      SimInputDevice::init();
      pDirectInput->EnumDevices(DIDEVTYPE_MOUSE, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY);
      pDirectInput->EnumDevices(DIDEVTYPE_KEYBOARD, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY);
      pDirectInput->EnumDevices(DIDEVTYPE_JOYSTICK, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY);
   }
}   


//------------------------------------------------------------------------------
void SimInputManager::capture()
{
   SimInputDevice *dptr;
   for (iterator ptr = begin(); ptr != end(); ptr++)
   {
      dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr) dptr->capture();
   }
}   


//------------------------------------------------------------------------------
void SimInputManager::release()
{
   SimInputDevice *dptr;
   for (iterator ptr = begin(); ptr != end(); ptr++)
   {
      dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr) dptr->release();
   }
}   


//------------------------------------------------------------------------------
void SimInputManager::activate(BYTE deviceType, BYTE deviceInst)
{
   SimInputDevice *dptr;
   for (iterator ptr = begin(); ptr != end(); ptr++)
   {
      dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr && 
         ((deviceType == SI_ANY) || (dptr->getDeviceType() == deviceType)) && 
         ((deviceInst == SI_ANY) || (dptr->getDeviceInst() == deviceInst)) )
      {
         dptr->activate();
      }
   }
}   


//------------------------------------------------------------------------------
void SimInputManager::deactivate(BYTE deviceType, BYTE deviceInst)
{
   SimInputDevice *dptr;
   for (iterator ptr = begin(); ptr != end(); ptr++)
   {
      dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr && 
         ((deviceType == SI_ANY) || (dptr->getDeviceType() == deviceType)) && 
         ((deviceInst == SI_ANY) || (dptr->getDeviceInst() == deviceInst)) )
      {
         dptr->deactivate();
      }
   }
}   


//------------------------------------------------------------------------------

bool SimInputManager::onAdd()
{
   if(!Parent::onAdd())
      return false;
   addToSet(SimFrameEndNotifySetId);
   capture();

   return true;
}

bool SimInputManager::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimInputEvent);
      onEvent(SimInputActivateEvent);
   
      //--------------------------------------
      case SimFrameEndNotifyEventType:
         process();
         return (true);

      //--------------------------------------
      // the keyboard focus was lost... reset the modifier keys
      case SimMessageEventType:
         modifierKeys = 0;
         return (true);
      default:
         return Parent::processEvent(event);
   }
}   


//------------------------------------------------------------------------------
void SimInputManager::onDeleteNotify(SimObject *object)
{
   if ( object == consumer)
      consumer = NULL;
   Parent::onDeleteNotify(object);
}   

//------------------------------------------------------------------------------
void SimInputManager::onRemove()
{
   release();
   Parent::onRemove();
}   


void SimInputManager::addMakeEvent(SimInputEvent *ev)
{
   makeList.push_back(*ev);
}

void SimInputManager::removeMakeEvent(int i)
{
   makeList.erase(i);
}

//------------------------------------------------------------------------------
bool SimInputManager::onSimInputEvent(const SimInputEvent *ev)
{
   // we need to be able to modify this event
   SimInputEvent *event = const_cast<SimInputEvent*>(ev);
   bool modKey = false;
   
   // if key event produced by the default keyboard record the modifier key states
   if (event->deviceType == SI_KEYBOARD && event->deviceInst == 0)   
   {
      if (event->action == SI_MAKE)
         switch (event->objInst)
         {
            case DIK_LSHIFT:     modifierKeys |= SI_LSHIFT; modKey = true; break; 
            case DIK_RSHIFT:     modifierKeys |= SI_RSHIFT; modKey = true; break;
            case DIK_LCONTROL:   modifierKeys |= SI_LCTRL; modKey = true; break;
            case DIK_RCONTROL:   modifierKeys |= SI_RCTRL; modKey = true; break;
            case DIK_LMENU:      modifierKeys |= SI_LALT; modKey = true; break;
            case DIK_RMENU:      modifierKeys |= SI_RALT; modKey = true; break;
         }
      else   
         switch (event->objInst)
         {
            case DIK_LSHIFT:     modifierKeys &= ~SI_LSHIFT; modKey = true; break; 
            case DIK_RSHIFT:     modifierKeys &= ~SI_RSHIFT; modKey = true; break;
            case DIK_LCONTROL:   modifierKeys &= ~SI_LCTRL; modKey = true; break;
            case DIK_RCONTROL:   modifierKeys &= ~SI_RCTRL; modKey = true; break;
            case DIK_LMENU:      modifierKeys &= ~SI_LALT; modKey = true; break;
            case DIK_RMENU:      modifierKeys &= ~SI_RALT; modKey = true; break;
         }
   }

   //if (event->deviceType == SI_KEYBOARD && event->objType == SI_KEY && !modKey )
   if (!modKey )
      event->modifier = modifierKeys;
   else
      event->modifier = 0;
   
   // if a keystroke generate ascii character if one exists
   if (event->objType == SI_KEY)
      event->ascii = getAscii(event->objInst, modifierKeys);

   if (!consumer)
   {
      consumer = manager->findObject(SimInputConsumerSetId);
      if (consumer)
         consumer->deleteNotify( this );
   }
   if(!simActionHandler)
   {
      if(consumer)
      {
         SimSet *s = dynamic_cast<SimSet *>(consumer);
         if(s)
         {
            SimSet::iterator i;
            for(i = s->begin(); i != s->end(); i++)
            {
               if(dynamic_cast<SimActionHandler *>(*i) != NULL)
                  simActionHandler = (SimActionHandler *) *i;
            }
         }
      }
   }

   if (event->action == SI_BREAK && simActionHandler)
   {
      // check if it's in the break list
      Vector<SimInputEvent>::iterator i = makeList.begin();
      for (; i != makeList.end(); i++)
         if (SimActionMap::isMakeBreakPair(i, event))
         {
            simActionHandler->onSimInputEvent(event);
            return true;
         }
   }

   if (consumer)
      consumer->processEvent( event );

   return (true);
}   


//------------------------------------------------------------------------------
bool SimInputManager::onSimInputActivateEvent(const SimInputActivateEvent *event)
{
   if (event->activate)
      activate(event->deviceType, event->deviceInst);
   else
      deactivate(event->deviceType, event->deviceInst);
   return (true);
}   


//------------------------------------------------------------------------------
void SimInputManager::process()
{
   SimInputDevice *dptr;
   for (iterator ptr = begin(); ptr != end(); ptr++)
   {
      dptr = dynamic_cast<SimInputDevice*>(*ptr);
      if (dptr) dptr->process();
   }
}   



//------------------------------------------------------------------------------
DIKCode DIKCodeList[DIK_LAST+1] = {
     0,      0, 0,                  //                     0x00
'\x1b', '\x1b', "escape",           // DIK_ESCAPE          0x01
   '1',    '!', 0,                  // DIK_1               0x02
   '2',    '@', 0,                  // DIK_2               0x03
   '3',    '#', 0,                  // DIK_3               0x04
   '4',    '$', 0,                  // DIK_4               0x05
   '5',    '%', 0,                  // DIK_5               0x06
   '6',    '^', 0,                  // DIK_6               0x07
   '7',    '&', 0,                  // DIK_7               0x08
   '8',    '*', 0,                  // DIK_8               0x09
   '9',    '(', 0,                  // DIK_9               0x0A
   '0',    ')', 0,                  // DIK_0               0x0B
   '-',    '_', 0,                  // DIK_MINUS           0x0C
   '=',    '+', 0,                  // DIK_EQUALS          0x0D
  '\b',   '\b', "backspace",        // DIK_BACK            0x0E
  '\t',   '\t', "tab",              // DIK_TAB             0x0F
   'q',    'Q', 0,                  // DIK_Q               0x10
   'w',    'W', 0,                  // DIK_W               0x11
   'e',    'E', 0,                  // DIK_E               0x12
   'r',    'R', 0,                  // DIK_R               0x13
   't',    'T', 0,                  // DIK_T               0x14
   'y',    'Y', 0,                  // DIK_Y               0x15
   'u',    'U', 0,                  // DIK_U               0x16
   'i',    'I', 0,                  // DIK_I               0x17
   'o',    'O', 0,                  // DIK_O               0x18
   'p',    'P', 0,                  // DIK_P               0x19
   '[',    '{', 0,                  // DIK_LBRACKET        0x1A
   ']',    '}', 0,                  // DIK_RBRACKET        0x1B
  '\r',   '\r', "enter",            // DIK_RETURN          0x1C
     0,      0, "lcontrol",         // DIK_LCONTROL        0x1D
   'a',    'A', 0,                  // DIK_A               0x1E
   's',    'S', 0,                  // DIK_S               0x1F
   'd',    'D', 0,                  // DIK_D               0x20
   'f',    'F', 0,                  // DIK_F               0x21
   'g',    'G', 0,                  // DIK_G               0x22
   'h',    'H', 0,                  // DIK_H               0x23
   'j',    'J', 0,                  // DIK_J               0x24
   'k',    'K', 0,                  // DIK_K               0x25
   'l',    'L', 0,                  // DIK_L               0x26
   ';',    ':', 0,                  // DIK_SEMICOLON       0x27
  '\'',    '"', "quote",            // DIK_APOSTROPHE      0x28
   '`',    '~', 0,                  // DIK_GRAVE           0x29
     0,      0, "lshift",           // DIK_LSHIFT          0x2A
  '\\',    '|', 0,                  // DIK_BACKSLASH       0x2B
   'z',    'Z', 0,                  // DIK_Z               0x2C
   'x',    'X', 0,                  // DIK_X               0x2D
   'c',    'C', 0,                  // DIK_C               0x2E
   'v',    'V', 0,                  // DIK_V               0x2F
   'b',    'B', 0,                  // DIK_B               0x30
   'n',    'N', 0,                  // DIK_N               0x31
   'm',    'M', 0,                  // DIK_M               0x32
   ',',    '<', "comma",            // DIK_COMMA           0x33
   '.',    '>', "period",           // DIK_PERIOD          0x34
   '/',    '?', 0,                  // DIK_SLASH           0x35
     0,      0, "rshift",           // DIK_RSHIFT          0x36
   '*',    '*', "numpad*",          // DIK_MULTIPLY        0x37
     0,      0, "lalt",             // DIK_LMENU           0x38
   ' ',    ' ', "space",            // DIK_SPACE           0x39
     0,      0, "capslock",         // DIK_CAPITAL         0x3A
     0,      0, "f1",               // DIK_F1              0x3B
     0,      0, "f2",               // DIK_F2              0x3C
     0,      0, "f3",               // DIK_F3              0x3D
     0,      0, "f4",               // DIK_F4              0x3E
     0,      0, "f5",               // DIK_F5              0x3F
     0,      0, "f6",               // DIK_F6              0x40
     0,      0, "f7",               // DIK_F7              0x41
     0,      0, "f8",               // DIK_F8              0x42
     0,      0, "f9",               // DIK_F9              0x43
     0,      0, "f10",              // DIK_F10             0x44
     0,      0, "numlock",          // DIK_NUMLOCK         0x45
     0,      0, "scroll",           // DIK_SCROLL          0x46
   '7',    '7', "numpad7",          // DIK_NUMPAD7         0x47
   '8',    '8', "numpad8",          // DIK_NUMPAD8         0x48
   '9',    '9', "numpad9",          // DIK_NUMPAD9         0x49
   '-',    '-', "numpad-",          // DIK_SUBTRACT        0x4A
   '4',    '4', "numpad4",          // DIK_NUMPAD4         0x4B
   '5',    '5', "numpad5",          // DIK_NUMPAD5         0x4C
   '6',    '6', "numpad6",          // DIK_NUMPAD6         0x4D
   '+',    '+', "numpad+",          // DIK_ADD             0x4E
   '1',    '1', "numpad1",          // DIK_NUMPAD1         0x4F
   '2',    '2', "numpad2",          // DIK_NUMPAD2         0x50
   '3',    '3', "numpad3",          // DIK_NUMPAD3         0x51
   '0',    '0', "numpad0",          // DIK_NUMPAD0         0x52
   '.',    '.', "decimal",          // DIK_DECIMAL         0x53
     0,      0, 0,                  //                     0x54
     0,      0, 0,                  //                     0x55
     0,      0, 0,                  //                     0x56
     0,      0, "f11",              // DIK_F11             0x57
     0,      0, "f12",              // DIK_F12             0x58
     0,      0, 0,                  //                     0x59
     0,      0, 0,                  //                     0x5A
     0,      0, 0,                  //                     0x5B
     0,      0, 0,                  //                     0x5C
     0,      0, 0,                  //                     0x5D
     0,      0, 0,                  //                     0x5E
     0,      0, 0,                  //                     0x5F
     0,      0, 0,                  //                     0x60
     0,      0, 0,                  //                     0x61
     0,      0, 0,                  //                     0x62
     0,      0, 0,                  //                     0x63
     0,      0, "f13",              // DIK_F13             0x64
     0,      0, "f14",              // DIK_F14             0x65
     0,      0, "f15",              // DIK_F15             0x66
     0,      0, 0,                  //                     0x67
     0,      0, 0,                  //                     0x68
     0,      0, 0,                  //                     0x69
     0,      0, 0,                  //                     0x6A
     0,      0, 0,                  //                     0x6B
     0,      0, 0,                  //                     0x6C
     0,      0, 0,                  //                     0x6D
     0,      0, 0,                  //                     0x6E
     0,      0, 0,                  //                     0x6F
     0,      0, 0,                  // DIK_KANA            0x70
     0,      0, 0,                  //                     0x71
     0,      0, 0,                  //                     0x72
     0,      0, 0,                  //                     0x73
     0,      0, 0,                  //                     0x74
     0,      0, 0,                  //                     0x75
     0,      0, 0,                  //                     0x76
     0,      0, 0,                  //                     0x77
     0,      0, 0,                  //                     0x78
     0,      0, 0,                  // DIK_CONVERT         0x79
     0,      0, 0,                  //                     0x7A
     0,      0, 0,                  // DIK_NOCONVERT       0x7B
     0,      0, 0,                  //                     0x7C
     0,      0, 0,                  // DIK_YEN             0x7D
     0,      0, 0,                  //                     0x7E
     0,      0, 0,                  //                     0x7F
     0,      0, 0,                  //                     0x80
     0,      0, 0,                  //                     0x81
     0,      0, 0,                  //                     0x82
     0,      0, 0,                  //                     0x83
     0,      0, 0,                  //                     0x84
     0,      0, 0,                  //                     0x85
     0,      0, 0,                  //                     0x86
     0,      0, 0,                  //                     0x87
     0,      0, 0,                  //                     0x88
     0,      0, 0,                  //                     0x89
     0,      0, 0,                  //                     0x8A
     0,      0, 0,                  //                     0x8B
     0,      0, 0,                  //                     0x8C
   '=',    '=', "numpadequals",     // DIK_NUMPADEQUALS    0x8D
     0,      0, 0,                  //                     0x8E
     0,      0, 0,                  //                     0x8F
     0,      0, 0,                  // DIK_CIRCUMFLEX      0x90
   '@',    '@', 0,                  // DIK_AT              0x91
   ':',    ':', 0,                  // DIK_COLON           0x92
   '_',    '_', 0,                  // DIK_UNDERLINE       0x93
     0,      0, 0,                  // DIK_KANJI           0x94
     0,      0, "stop",             // DIK_STOP            0x95
     0,      0, 0,                  // DIK_AX              0x96
     0,      0, 0,                  // DIK_UNLABELED       0x97
     0,      0, 0,                  //                     0x98
     0,      0, 0,                  //                     0x99
     0,      0, 0,                  //                     0x9A
     0,      0, 0,                  //                     0x9B
  '\r',   '\r', "numpadenter",      // DIK_NUMPADENTER     0x9C
     0,      0, "rcontrol",         // DIK_RCONTROL        0x9D
     0,      0, 0,                  //                     0x9E
     0,      0, 0,                  //                     0x9F
     0,      0, 0,                  //                     0xA0
     0,      0, 0,                  //                     0xA1
     0,      0, 0,                  //                     0xA2
     0,      0, 0,                  //                     0xA3
     0,      0, 0,                  //                     0xA4
     0,      0, 0,                  //                     0xA5
     0,      0, 0,                  //                     0xA6
     0,      0, 0,                  //                     0xA7
     0,      0, 0,                  //                     0xA8
     0,      0, 0,                  //                     0xA9
     0,      0, 0,                  //                     0xAA
     0,      0, 0,                  //                     0xAB
     0,      0, 0,                  //                     0xAC
     0,      0, 0,                  //                     0xAD
     0,      0, 0,                  //                     0xAE
     0,      0, 0,                  //                     0xAF
     0,      0, 0,                  //                     0xB0
     0,      0, 0,                  //                     0xB1
     0,      0, 0,                  //                     0xB2
   ',',    ',', "numpadcomma",      // DIK_NUMPADCOMMA     0xB3
     0,      0, 0,                  //                     0xB4
   '/',    '/', "numpad/",          // DIK_DIVIDE          0xB5
     0,      0, 0,                  //                     0xB6
     0,      0, "sysreq",           // DIK_SYSRQ           0xB7
     0,      0, "ralt",             // DIK_RMENU           0xB8
     0,      0, 0,                  //                     0xB9
     0,      0, 0,                  //                     0xBA
     0,      0, 0,                  //                     0xBB
     0,      0, 0,                  //                     0xBC
     0,      0, 0,                  //                     0xBD
     0,      0, 0,                  //                     0xBE
     0,      0, 0,                  //                     0xBF
     0,      0, 0,                  //                     0xC0
     0,      0, 0,                  //                     0xC1
     0,      0, 0,                  //                     0xC2
     0,      0, 0,                  //                     0xC3
     0,      0, 0,                  //                     0xC4
     0,      0, 0,                  //                     0xC5
     0,      0, 0,                  //                     0xC6
     0,      0, "home",             // DIK_HOME            0xC7
     0,      0, "up",               // DIK_UP              0xC8
     0,      0, "prior",            // DIK_PRIOR           0xC9
     0,      0, 0,                  //                     0xCA
     0,      0, "left",             // DIK_LEFT            0xCB
     0,      0, 0,                  //                     0xCC
     0,      0, "right",            // DIK_RIGHT           0xCD
     0,      0, 0,                  //                     0xCE
     0,      0, "end",              // DIK_END             0xCF
     0,      0, "down",             // DIK_DOWN            0xD0
     0,      0, "next",             // DIK_NEXT            0xD1
     0,      0, "insert",           // DIK_INSERT          0xD2
     0,      0, "delete",           // DIK_DELETE          0xD3
     0,      0, 0,                  //                     0xD4
     0,      0, 0,                  //                     0xD5
     0,      0, 0,                  //                     0xD6
     0,      0, 0,                  //                     0xD7
     0,      0, 0,                  //                     0xD8
     0,      0, 0,                  //                     0xD9
     0,      0, 0,                  //                     0xDA
     0,      0, "win",              // DIK_LWIN            0xDB
     0,      0, "win",              // DIK_RWIN            0xDC
     0,      0, "app",              // DIK_APPS            0xDD
};


//------------------------------------------------------------------------------
BYTE SimInputManager::getAscii(BYTE dik, BYTE mod)
{
   if (dik > DIK_LAST) return (0);
   if (mod & SI_SHIFT)
      return DIKCodeList[dik].upper;
   return DIKCodeList[dik].lower;
}   

//------------------------------------------------------------------------------
BYTE SimInputManager::getDIK_CODE(const char *str)
{
   for (int i=0; i<DIK_LAST; i++)
   {
      if (DIKCodeList[i].name && 
          stricmp(DIKCodeList[i].name, str) == 0)
         return (i);
   }
   return (0);
}   

//------------------------------------------------------------------------------
BYTE SimInputManager::getDIK_CODE(const char c)
{
   for (int i=0; i<DIK_LAST; i++)
   {
      if (DIKCodeList[i].lower == c || DIKCodeList[i].upper == c)
         return (i);
   }
   return (0);
}   

//------------------------------------------------------------------------------
const DIKCode* SimInputManager::getDIK_STRUCT(BYTE dik)
{
   if (dik > DIK_LAST) return (0);
   return &DIKCodeList[dik];
}   


#if SI_DEBUG
//------------------------------------------------------------------------------
char* _echoResult(HRESULT r, const char *pre)
{
   char *s;
   switch (r)
   {
      case DI_OK: s = NULL; break;
      case DI_POLLEDDEVICE:      s= "DI_POLLEDDEVICE: The device is a polled device."; break;
      case DI_DOWNLOADSKIPPED:   s = "DI_DOWNLOADSKIPPED: effect down load skipped"; break;
      case DI_EFFECTRESTARTED:   s = "DI_EFFECTRESTARTED: effect needs to be restarted"; break;
      case DIERR_OLDDIRECTINPUTVERSION:  s = "DIERR_OLDDIRECTINPUTVERSION: The application requires a newer version of DirectInput."; break;     
      case DIERR_BETADIRECTINPUTVERSION: s = "DIERR_BETADIRECTINPUTVERSION: The application was written for an unsupported prerelease version if dinput."; break;
      case DIERR_BADDRIVERVER:   s = "DIERR_BADDRIVERVER: incompatible driver version or incomplete driver components"; break;              
      case DIERR_DEVICENOTREG:   s = "DIERR_DEVICENOTREG: not registered"; break;
      //case DIERR_NOTFOUND:     s = "DIERR_NOTFOUND: The requested object does not exist."; break;                  
      case DIERR_OBJECTNOTFOUND: s = "DIERR_OBJECTNOTFOUND: The requested object does not exist."; break;            
      case DIERR_INVALIDPARAM:   s = "DIERR_INVALIDPARAM: An invalid parameter was passed"; break;
      case DIERR_NOINTERFACE:    s = "DIERR_NOINTERFACE: interface is not supported by the object"; break;
      case DIERR_GENERIC:        s = "DIERR_GENERIC: An undetermined error occured"; break;
      case DIERR_OUTOFMEMORY:    s = "DIERR_OUTOFMEMORY: couldn't allocate sufficient memory"; break;
      case DIERR_UNSUPPORTED:    s = "DIERR_UNSUPPORTED: function called is not supported at this time"; break;
      case DIERR_NOTINITIALIZED: s = "DIERR_NOTINITIALIZED: This object has not been initialized"; break;
      case DIERR_ALREADYINITIALIZED:   s = "DIERR_ALREADYINITIALIZED: This object is already initialized"; break;
      case DIERR_NOAGGREGATION:  s = "DIERR_NOAGGREGATION: This object does not support aggregation"; break;
      case DIERR_INPUTLOST:      s = "DIERR_INPUTLOST: Access to the device has been lost"; break;
      case DIERR_ACQUIRED:       s = "DIERR_ACQUIRED: operation cannot be performed while the device is acquired."; break;
      case DIERR_NOTACQUIRED:    s = "DIERR_NOTACQUIRED: operation cannot be performed unless the device is acquired"; break;
      case DIERR_HANDLEEXISTS:   s = "DIERR_HANDLEEXISTS: specified property cannot be changed"; break;
      case E_PENDING:            s = "E_PENDING: Data is not yet available."; break;
      case DIERR_INSUFFICIENTPRIVS:    s = "DIERR_INSUFFICIENTPRIVS: user does not have sufficient privileges"; break;
      case DIERR_DEVICEFULL:     s = "DIERR_DEVICEFULL: The device is full."; break;
      case DIERR_MOREDATA:       s = "DIERR_MOREDATA: Not all the requested information fit into the buffer"; break;
      case DIERR_NOTDOWNLOADED:  s = "DIERR_NOTDOWNLOADED: The effect is not downloaded."; break;
      case DIERR_HASEFFECTS:     s = "DIERR_HASEFFECTS: cannot be reinitialized because there are still effects attached"; break;
      case DIERR_NOTEXCLUSIVEACQUIRED: s = "DIERR_NOTEXCLUSIVEACQUIRED: device is not acquired in DISCL_EXCLUSIVE mode."; break;
      case DIERR_INCOMPLETEEFFECT:     s = "DIERR_INCOMPLETEEFFECT: effect not downloaded, essential information missing"; break;
      case DIERR_NOTBUFFERED:    s = "DIERR_NOTBUFFERED: Attempted to read buffered device data from a device that is not buffered"; break;
      case DIERR_EFFECTPLAYING:  s = "DIERR_EFFECTPLAYING: connot modify params of playing effect"; break;
      default: s = "DIERR_unknown ???";
   }
   static char buff[200];
   sprintf(buff, "%s: %s", pre, s);
   return (buff);
}   
#endif


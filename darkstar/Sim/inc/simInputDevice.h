//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMINPUTDEVICE_H_
#define _SIMINPUTDEVICE_H_

#include <dInput.h>
#include <simInputDevice.h>
#include <simInput.h>


// device constants (from DirectInput)    (n <= 31) !!!
#define SI_DEVICE    DI8DEVTYPE_DEVICE    
#define SI_MOUSE     DI8DEVTYPE_MOUSE     
#define SI_KEYBOARD  DI8DEVTYPE_KEYBOARD  
#define SI_JOYSTICK  DI8DEVTYPE_JOYSTICK  
#define SI_POINTER   (16)

// Device Event Action Types
#define SI_MAKE      0x01
#define SI_BREAK     0x02
#define SI_MOVE      0x03

//Device Event Types
#define SI_UNKNOWN   0x01
#define SI_BUTTON    0x02
#define SI_POV       0x03
#define SI_XPOV      0x04
#define SI_YPOV      0x05
#define SI_UPOV      0x06
#define SI_DPOV      0x07
#define SI_LPOV      0x08
#define SI_RPOV      0x09
#define SI_KEY       0x0A
#define SI_XAXIS     0x0B
#define SI_YAXIS     0x0C
#define SI_ZAXIS     0x0D
#define SI_RXAXIS    0x0E
#define SI_RYAXIS    0x0F
#define SI_RZAXIS    0x10  
#define SI_SLIDER    0x11  

// Event SubTypes
#define SI_ANY       0xff

// Modifier Keys
#define SI_LSHIFT    (1<<0)   
#define SI_RSHIFT    (1<<1)
#define SI_SHIFT     (SI_LSHIFT|SI_RSHIFT)
#define SI_LCTRL     (1<<2)   
#define SI_RCTRL     (1<<3)
#define SI_CTRL      (SI_LCTRL|SI_RCTRL)
#define SI_LALT      (1<<4)   
#define SI_RALT      (1<<5)
#define SI_ALT       (SI_LALT|SI_RALT)



class SimInputDevice: public SimObject
{
public:
   static bool isDX5;
   struct ObjInfo
   {
      BYTE objType;   
      BYTE objInst;   
      int min, max;
   };

protected:
   enum constants {
      QUEUED_BUFFER_SIZE = 128,     
   };

   IDirectInputDevice *did;
   IDirectInputDevice2 *did2;
   DIDEVICEINSTANCE diInstance;
   DIDEVCAPS diCaps;
   BYTE   deviceType;
   BYTE   deviceInst;
   char   deviceName[30];

   static BYTE deviceInstCounter;
   static BYTE mouseInstCounter;
   static BYTE keyboardInstCounter;
   static BYTE joystickInstCounter;
   static HWND hWnd;

   bool     active;                 // available to be used
   bool     captured;               // in use
   SimMessageEvent *captureEvent;   // key used when recapturing a device

   //--------------------------------------
   DIDEVICEOBJECTINSTANCE *objInstance;
   DIOBJECTDATAFORMAT     *objFormat;
   ObjInfo                *objInfo;
   char                   *objBuffer1;    // polled device input buffers
   char                   *objBuffer2;
   char                   *prevObjBuffer; // points to buffer 1 or 2

   DWORD objBufferSize;                   // size of objBuffer*
   DWORD objCount;                        // number of objects on this device
   DWORD objEnumCount;                    // used during enumeration ONLY   
   int   objBufferOfs;                    // used during enumeration ONLY

   enum Constants
   {
      SIZEOF_BUTTON = 1,                  // size of an objects data in bytes
      SIZEOF_KEY    = 1,
      SIZEOF_AXIS   = 4,
      SIZEOF_POV    = 4,
   };
   static BOOL CALLBACK EnumDeviceObjectsProc( const DIDEVICEOBJECTINSTANCE *doi, LPVOID pvRef);
   bool enumerate();
   void processAsync();
   void processPolled();
   bool setFormat();

   DWORD findObjInstance(DWORD offset);
   bool  buildEvent(DWORD offset, int data, int old);
   IDirectInput* getDInput();
   HWND getWindowHandle();

public:
   static void init();

   SimInputDevice(const DIDEVICEINSTANCE *dii);
   ~SimInputDevice();

   BYTE  getDeviceInst();
   BYTE  getDeviceType();
   void  setDeviceInst(BYTE id);

   bool open();
   void close();

   bool capture();
   void release();

   void activate();
   void deactivate();

   bool isActive();
   bool isCaptured();
   bool isPolled();
   const char* getName();
   const char* getDeviceName();
   const char* getProductName();
   void setMaxDelta(float delta);
   const ObjInfo* getObject(DWORD n);

   void process();

   //--------------------------------------
   bool processEvent(const SimEvent *event);
   bool processQuery(SimQuery *query)  { return (false); };
};


//------------------------------------------------------------------------------
inline void SimInputDevice::activate()
{
   active = true;
}   

inline void SimInputDevice::deactivate()
{
   active = false;
   release();
}   


inline BYTE SimInputDevice::getDeviceInst()
{
   return (deviceInst);   
}   


inline void SimInputDevice::setDeviceInst(BYTE id)
{
   deviceInst = id;   
}   

inline BYTE SimInputDevice::getDeviceType()
{
   return (deviceType);   
}   


inline bool SimInputDevice::isPolled()
{
   return (diCaps.dwFlags & DIDC_POLLEDDEVICE);
}   


#endif //_SIMINPUTDEVICE_H_

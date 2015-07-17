//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simInputDevice.h>
#include <simCanvas.h>
#include "console.h"

BYTE SimInputDevice::deviceInstCounter = 0;
BYTE SimInputDevice::mouseInstCounter  = 0;
BYTE SimInputDevice::keyboardInstCounter = 0;
BYTE SimInputDevice::joystickInstCounter = 0;
bool SimInputDevice::isDX5 = false;
HWND SimInputDevice::hWnd  = NULL;



//------------------------------------------------------------------------------
SimInputDevice::SimInputDevice(const DIDEVICEINSTANCE *dii)
{
   diInstance= *dii;
   did   = NULL;   
   did2  = NULL;   
   captureEvent  = NULL;
   objInstance   = NULL;
   objFormat     = NULL;  
   objBuffer1    = NULL;  
   objBuffer2    = NULL;  
   prevObjBuffer = NULL;  
   objInfo       = NULL;   
   deviceName[0] = 0;

   active    = false;
   captured  = false;
   deviceType= GET_DIDEVICE_TYPE(diInstance.dwDevType);
   switch (deviceType)
   {
      case SI_DEVICE:   deviceInst = deviceInstCounter++;   break;
      case SI_MOUSE:    deviceInst = mouseInstCounter++;    break;
      case SI_KEYBOARD: deviceInst = keyboardInstCounter++; break;
      case SI_JOYSTICK: deviceInst = joystickInstCounter++; break;
   }
}   


//------------------------------------------------------------------------------
SimInputDevice::~SimInputDevice()
{
   close();   
}   


//------------------------------------------------------------------------------
void SimInputDevice::init()
{
   // reset device instance counters
   deviceInstCounter = 0;
   mouseInstCounter  = 0;
   keyboardInstCounter = 0;
   joystickInstCounter = 0;
}   


//------------------------------------------------------------------------------
bool SimInputDevice::open()
{
   if (getDInput()->CreateDevice(diInstance.guidInstance, &did, NULL) == DI_OK)
   {
      did->QueryInterface(IID_IDirectInputDevice2, (void**)&did2);   
      return (enumerate());
   }
   return (false);
}   


//------------------------------------------------------------------------------
void SimInputDevice::close()
{
   if (did)
   {
      release();
      if (did2)
      {
         did2->Release();
         did2 = NULL;
      }
      did->Release();
      did = NULL;

      delete [] objInstance;
      delete [] objFormat;  
      delete [] objBuffer1;  
      delete [] objBuffer2;  
      delete [] objInfo;    

      objInstance   = NULL;
      objFormat     = NULL;  
      objBuffer1    = NULL;  
      objBuffer2    = NULL;  
      prevObjBuffer = NULL;  
      objInfo       = NULL;   
      deviceName[0] = 0;
   }
}   


//------------------------------------------------------------------------------
HWND SimInputDevice::getWindowHandle()
{
   if (hWnd && IsWindow(hWnd))
      return (hWnd);
#if 0
   //--------------------------------------
   // Get window handle for application owning this thread
   DWORD proc, curproc;
   hWnd = GetTopWindow(0);
   curproc = GetCurrentProcessId();
   while (hWnd)
   {
      GetWindowThreadProcessId(hWnd, &proc);
      if (proc == curproc)
         break;

      hWnd = GetWindow(hWnd, GW_HWNDNEXT);
   }

   // If no window found, use active window (no guarantee it's the right window though!)
   if (hWnd == NULL)      
      hWnd = GetActiveWindow();
   return (hWnd);

#else
   // while this routine works for our current situation there is still no
   // guarantee the first window we find in the Canvas set is the app main_window  :(
   SimSet *set = static_cast<SimSet*>(manager->findObject(SimCanvasSetId));
   if (set && set->size())
   {
      SimCanvas *canvas = dynamic_cast<SimCanvas*>(*(set->begin()));
      if (canvas)
         hWnd = canvas->getAppHandle();
      else
         return (NULL);
   }
   return (NULL);
#endif
}   


//------------------------------------------------------------------------------
BOOL CALLBACK SimInputDevice::EnumDeviceObjectsProc( const DIDEVICEOBJECTINSTANCE *doi, LPVOID pvRef)
{
   DIDEVICEOBJECTINSTANCE *objInstance;
   DIOBJECTDATAFORMAT     *objFormat;
   
   if (doi->guidType == GUID_Unknown)     // we don't want to enumerate Unknown items
      return (DIENUM_CONTINUE);

   // reduce a couple pointers
   SimInputDevice *device = (SimInputDevice*)pvRef;
   objInstance = &device->objInstance[device->objEnumCount];
   objFormat   = &device->objFormat[device->objEnumCount];

   // fill in object instance structure
   *objInstance = *doi;

   // DWORD objects must be DWORD aligned
   if (!(objInstance->dwType & DIDFT_BUTTON ))
      device->objBufferOfs = (device->objBufferOfs+3)&~3;

   objInstance->dwOfs = device->objBufferOfs;

   // fill in object data format structure
   objFormat->pguid  = &objInstance->guidType;
   objFormat->dwType = objInstance->dwType;
   objFormat->dwFlags= 0;
   objFormat->dwOfs  = device->objBufferOfs;

   // advance enumeration counters
   if (objFormat->dwType & DIDFT_BUTTON )
      device->objBufferOfs += SIZEOF_BUTTON;
   else
      device->objBufferOfs += SIZEOF_AXIS;
   device->objEnumCount++;

   return (DIENUM_CONTINUE);
}   

    
//------------------------------------------------------------------------------
bool SimInputDevice::enumerate()
{
   if (!did) return (false);

   diCaps.dwSize = isDX5 ? sizeof(DIDEVCAPS) : sizeof(DIDEVCAPS_DX3);
   if (did->GetCapabilities( &diCaps ) != DI_OK) return (false);
   
   // establish needed buffer sizes and allocate them
   // worst case buffer size is 1 dword per object
   objBufferSize = (diCaps.dwAxes + diCaps.dwButtons + diCaps.dwPOVs) * sizeof(DWORD);
   objCount      = diCaps.dwAxes + diCaps.dwButtons + diCaps.dwPOVs;

   objInstance = new DIDEVICEOBJECTINSTANCE[objCount];
   objFormat   = new DIOBJECTDATAFORMAT[objCount];
   objInfo     = new ObjInfo[objCount];
   if (isPolled())
   {
      objBuffer1  = new char[objBufferSize];
      objBuffer2  = new char[objBufferSize];
   }
   objEnumCount= 0;
   objBufferOfs= 0;

   if (did->EnumObjects(EnumDeviceObjectsProc, this, DIDFT_ALL) != DI_OK) return (false);
   objBufferSize = (objBufferOfs+3)&~3;  // fill in actual size to nearest DWORD

   BYTE buttonInst = 0;
   BYTE povInst    = 0;
   BYTE xAxisInst  = 0;
   BYTE yAxisInst  = 0;
   BYTE zAxisInst  = 0;
   BYTE rAxisInst  = 0;
   BYTE uAxisInst  = 0;
   BYTE vAxisInst  = 0;
   BYTE sliderInst = 0;
   BYTE unknownInst= 0;

   // this is ugly but microsoft forces me to do it...
   // do they actually use this stuff?
   for (DWORD i = 0; i<objCount; i++)
   {
      if (objInstance[i].guidType == GUID_XAxis) 
      {
         objInfo[i].objType = SI_XAXIS;
         objInfo[i].objInst = xAxisInst++;
      }
      else if (objInstance[i].guidType == GUID_YAxis) 
      {
         objInfo[i].objType = SI_YAXIS;
         objInfo[i].objInst = yAxisInst++;
      }
      else if (objInstance[i].guidType == GUID_ZAxis) 
      {
         objInfo[i].objType = SI_ZAXIS;
         objInfo[i].objInst = zAxisInst++;;
      }
      else if (objInstance[i].guidType == GUID_RxAxis) 
      {
         objInfo[i].objType = SI_RXAXIS;
         objInfo[i].objInst = rAxisInst++;;
      }
      else if (objInstance[i].guidType == GUID_RyAxis) 
      {
         objInfo[i].objType = SI_RYAXIS;
         objInfo[i].objInst = uAxisInst++;
      }
      else if (objInstance[i].guidType == GUID_RzAxis) 
      {
         objInfo[i].objType = SI_RZAXIS;
         objInfo[i].objInst = vAxisInst++;
      }
      else if (objInstance[i].guidType == GUID_Slider) 
      {
         objInfo[i].objType = SI_SLIDER;
         objInfo[i].objInst = sliderInst++;
      }
      else if (objInstance[i].guidType == GUID_Button) 
      {
         objInfo[i].objType = SI_BUTTON;
         objInfo[i].objInst = buttonInst++;
      }
      else if (objInstance[i].guidType == GUID_Key) 
      {
         objInfo[i].objType = SI_KEY;
         objInfo[i].objInst = DIDFT_GETINSTANCE(objFormat[i].dwType);  // DIK_CODE
      }
      else if (objInstance[i].guidType == GUID_POV) 
      {
         objInfo[i].objType = SI_POV;
         objInfo[i].objInst = povInst++;
      }
      else 
      {
         objInfo[i].objType = SI_UNKNOWN;
         objInfo[i].objInst = unknownInst;
      }

      if (objInfo[i].objType != SI_BUTTON &&
          objInfo[i].objType != SI_KEY &&
          objInfo[i].objType != SI_POV )
      {
         // get range properties
         DIPROPRANGE prop;
         prop.diph.dwSize        = sizeof(DIPROPRANGE);
         prop.diph.dwHeaderSize  = sizeof(DIPROPHEADER);
         prop.diph.dwHow         = DIPH_BYID;
         prop.diph.dwObj         = objFormat[i].dwType;

         if (SUCCEEDED(did->GetProperty(DIPROP_RANGE, &prop.diph)))
         {
            objInfo[i].min = prop.lMin;   
            objInfo[i].max = prop.lMax;   
         }
         else
         {
            objInfo[i].min = DIPROPRANGE_NOMIN;
            objInfo[i].max = DIPROPRANGE_NOMAX;   
         }
      }
      else
      {
         objInfo[i].min = DIPROPRANGE_NOMIN;
         objInfo[i].max = DIPROPRANGE_NOMAX;   
      }
   }

   switch (deviceType)
   {
      case SI_MOUSE:    sprintf(deviceName, "mouse%d", deviceInst); break;
      case SI_KEYBOARD: sprintf(deviceName, "keyboard%d", deviceInst); break;
      case SI_JOYSTICK: sprintf(deviceName, "joystick%d", deviceInst); break;
      default:
      case SI_DEVICE:   sprintf(deviceName, "unknown%d", deviceInst); break;
   }

   return (true);
}   




//------------------------------------------------------------------------------
bool SimInputDevice::setFormat()
{
   DIDATAFORMAT dataFormat;
   dataFormat.dwSize    = sizeof(DIDATAFORMAT);
   dataFormat.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
   dataFormat.dwFlags   = (deviceType == SI_MOUSE) ?  DIDF_RELAXIS : DIDF_ABSAXIS;
   dataFormat.dwDataSize= objBufferSize;
   dataFormat.dwNumObjs = objCount;
   dataFormat.rgodf     = objFormat;
      
   HRESULT r = did->SetDataFormat( &dataFormat );
   echoResult(r, avar("%d", deviceType) );
   return (SUCCEEDED(r));
}   


//------------------------------------------------------------------------------
bool SimInputDevice::capture()
{
   if (active && !captured && did)      
   {
      DIPROPDWORD prop;
      prop.diph.dwSize       = sizeof(DIPROPDWORD);
      prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
      prop.diph.dwObj = 0;
      prop.diph.dwHow = DIPH_DEVICE;
      if (isPolled())
         prop.dwData = objBufferSize;
      else
         prop.dwData = QUEUED_BUFFER_SIZE;

      getWindowHandle();
      HRESULT r;
      r = did->SetCooperativeLevel(hWnd, DISCL_FOREGROUND|DISCL_EXCLUSIVE);
      if (!SUCCEEDED(r)) 
      {
         r = did->SetCooperativeLevel(hWnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);         
         if (!SUCCEEDED(r))
         {
            echoResult(r, getDeviceName());
            return (false);
         }
      }

      if (!setFormat()) 
         return (false);
      r = did->SetProperty(DIPROP_BUFFERSIZE, &prop.diph);  
      if (!SUCCEEDED(r))  
      {
         echoResult(r, getDeviceName());
         return (false);
      }
      
      if (deviceType == SI_MOUSE)
      {
         prop.diph.dwObj   = 0;
         prop.diph.dwHow   = DIPH_DEVICE;
         prop.dwData       = DIPROPAXISMODE_REL;
         r = did->SetProperty(DIPROP_AXISMODE, &prop.diph);  
         if (!SUCCEEDED(r))  
         {
            echoResult(r, getDeviceName());
            return (false);
         }
      }

      r = did->Acquire(); 
      if (!SUCCEEDED(r))  
      {
         echoResult(r, getDeviceName());
         return (false);
      }
      captured = true;
      return (true);
   }
   return (false);
}   


//------------------------------------------------------------------------------
void SimInputDevice::release()
{
   if ((deviceType == SI_KEYBOARD) && group)
   {
      SimMessageEvent event;
      group->processEvent(&event);
   }
   if (did)
      did->Unacquire();
   captured = false;
}   


//------------------------------------------------------------------------------
void SimInputDevice::process()
{
   if (!active) return;
   if (captured)
   {
       if (isPolled())
         processPolled();
      else
         processAsync();
   }
   if (!captureEvent)
   {
      captureEvent = new SimMessageEvent();
      captureEvent->time = manager->getCurrentTime() + (deviceType == SI_JOYSTICK ? 1.0f : 0.10f);
      postEvent( captureEvent );
   }
}   


//------------------------------------------------------------------------------
DWORD SimInputDevice::findObjInstance(DWORD offset)
{
   DIDEVICEOBJECTINSTANCE *inst = objInstance;
   for (DWORD i=0; i<objCount; i++, inst++)
   {
      if (inst->dwOfs == offset)
         return (i);
   }  
   AssertFatal(0, "SimInputDevice::findObjInstance: Unable to locate object instance");
   return (0);
}   


//------------------------------------------------------------------------------
bool SimInputDevice::buildEvent(DWORD offset, int data, int old)
{
   DIDEVICEOBJECTINSTANCE &inst = objInstance[offset];   
   ObjInfo &info = objInfo[offset];
   
   if (info.objType == SI_UNKNOWN) return (false);
   SimInputEvent event; 
   
   event.deviceType = deviceType;
   event.deviceInst = deviceInst;
   event.objType    = info.objType;
   event.objInst    = info.objInst;
  
   switch (event.objType)
   {
      case SI_XAXIS:
      case SI_YAXIS:
      case SI_ZAXIS:
      case SI_RXAXIS:
      case SI_RYAXIS:
      case SI_RZAXIS:
      case SI_SLIDER:
         event.action = SI_MOVE;
         if (event.deviceType == SI_MOUSE)
            event.fValue = float(data);
         else
         {
            // if not an open range scale to range -1.0 to +1.0
            if (info.min != DIPROPRANGE_NOMIN && info.max != DIPROPRANGE_NOMAX)
            {
               float range  = float(info.max - info.min);
               event.fValue = float(data-info.min) / range;
            }
            else
            {
               event.fValue = data;
            }
         }
         //if (event.deviceType == SI_MOUSE && event.objType == SI_ZAXIS)
         //   Console->printf("Mouse z f%5.2f d%d", event.fValue, data);
         //if (event.deviceType == SI_MOUSE)
         //   Console->printf("Mouse %d f%5.2f d%d", event.objType, event.fValue, data);
         group->processEvent( &event );
         break;

      case SI_BUTTON:
         event.action = (data & 0x80) ? SI_MAKE : SI_BREAK;
         event.fValue = (event.action == SI_MAKE) ? 1.0f : 0.0f;
         group->processEvent( &event );
         break;

      case SI_KEY:
         event.action = (data & 0x80) ? SI_MAKE : SI_BREAK;
         event.ascii  = (inst.tszName[1] == '\0') ? inst.tszName[0] : 0;
         event.fValue = (event.action == SI_MAKE) ? 1.0f : 0.0f;
         group->processEvent( &event );
         break;

      case SI_POV:
         // split the single POV angle event into X/Y events
         event.action = SI_MOVE;  
         if (LOWORD(data) == 0xffff)         // is it centered?
         {
            event.objType  = SI_XPOV;
            event.fValue   = 0.5f;
            group->processEvent( &event );   // send XPOV event

            event.objType  = SI_YPOV;        // setup YPOV event
            group->processEvent( &event );
         }
         else
         {
            // map 0-35999 data into 0.0-M_2PI fdata
            float fdata = float(data) * (1.0/36000.0f) * M_2PI;   
            float x, y;       
            m_sincos( fdata, &x, &y);
            x = (x+1.0f)*0.5f;
            y = (-y+1.0f)*0.5f;
            event.objType  = SI_XPOV;
            event.fValue   = x;
            group->processEvent( &event );   // send XPOV event

            event.objType  = SI_YPOV;        // setup YPOV event
            event.fValue   = y;
            group->processEvent( &event );
         }

         // handle artificial POV up/down/left/right buttons
         data = LOWORD(data);
         old  = LOWORD(old);
         data = (data == 0xffff) ? 5 : ((data+31500)/9000)-3  & 0x03;
         old  = (old  == 0xffff) ? 5 : (((old+31500)/9000)-3) & 0x03;
         // 5 == centered position
         if (data != old) 
         {
            if (old != 5)
            {
               event.action = SI_BREAK;  
               event.fValue = 0.0f;
               switch (old)
               {
                  case 0:  event.objType = SI_UPOV; break;
                  case 1:  event.objType = SI_RPOV; break;
                  case 2:  event.objType = SI_DPOV; break;
                  case 3:  event.objType = SI_LPOV; break;
               }   
               group->processEvent( &event );
            }
            if (data != 5)
            {
               event.action = SI_MAKE;  
               event.fValue = 1.0f;
               switch (data)
               {
                  case 0:  event.objType = SI_UPOV; break;
                  case 1:  event.objType = SI_RPOV; break;
                  case 2:  event.objType = SI_DPOV; break;
                  case 3:  event.objType = SI_LPOV; break;
               }   
               group->processEvent( &event );
            }
         }
         break;
   }
   return ( true );
}   


//------------------------------------------------------------------------------
void SimInputDevice::processAsync()
{
   DIDEVICEOBJECTDATA dod[QUEUED_BUFFER_SIZE];
   DWORD   dwItems = QUEUED_BUFFER_SIZE;
   HRESULT result;
   if (did)   
   {
      do    // async fills the supplies buffer with events that have occured on
      {     // on device objects, processthem until none are left
         result = did->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), dod, &dwItems, 0);   
         if (!SUCCEEDED(result))
         {
            //AssertWarn(0,"SimInputDevice::processAsync: GetDeviceData failed.");
            echoResult(result, getDeviceName());
            if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
            {
               captured = false;
               release();
            }
            return;
         }
      
         for (DWORD i=0; i<dwItems; i++)
            buildEvent(findObjInstance(dod[i].dwOfs), dod[i].dwData, dod[i].dwData);

      }while (dwItems);
   }
}   


//------------------------------------------------------------------------------
void SimInputDevice::processPolled()
{
   // gotta handle polled devies just a little different....
   if (did)
   {
      if (did2)
         did2->Poll();

      int old;
      char *buffer = (prevObjBuffer == objBuffer1) ? objBuffer2 : objBuffer1;
      HRESULT result = did->GetDeviceState(objBufferSize, buffer);
      if (!SUCCEEDED(result))
      {
         //AssertWarn(0,"SimInputDevice::processPolledc: GetDeviceState failed.");
         echoResult(result, getDeviceName());
         if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
         {
            captured = false;
            release();
         }
         return;
      }
      // polled devices get data for all device objects reguardless of change
      // loop through all the objects and produce appropiate events
      for (DWORD i = 0; i<objCount; i++)
      {
         int data;
         if (objFormat[i].dwType & DIDFT_BUTTON) 
         {
            if (prevObjBuffer)
            {
               data = *((char*)(buffer+objFormat[i].dwOfs));
               old = *((char*)(prevObjBuffer+objFormat[i].dwOfs));
               if (data == old)     // if button state has not changed do not 
                  continue;         // produce any events
            }
            else                    // need to get initial state first
               continue;    
         }
         else if (objFormat[i].dwType & DIDFT_POV) 
         {
            if (prevObjBuffer)
            {
               data = *((int*)(buffer+objFormat[i].dwOfs));
               old = *((int*)(prevObjBuffer+objFormat[i].dwOfs));
               if (LOWORD(data) == LOWORD(old))  // if POV state has not changed do not 
                  continue;         // produce any events
            }
            else                    // need to get initial state first
               continue;    
         }
         else
            data = *((int*)(buffer+objFormat[i].dwOfs));

         buildEvent(i, data, old);
      }
      prevObjBuffer = buffer;  // swap previous obj state buffer
   }
}   


//------------------------------------------------------------------------------
bool SimInputDevice::isActive()
{
   return (active);
}   

bool SimInputDevice::isCaptured()
{
   return (captured);
}   


//------------------------------------------------------------------------------
bool SimInputDevice::processEvent(const SimEvent *event)
{
   if (event->type == SimMessageEventType)
   {
      if (static_cast<const SimMessageEvent*>(event) == captureEvent)
      {
         captureEvent = NULL;
         capture();
      }
      return (true);
   }
   return (false);
} 


//------------------------------------------------------------------------------
IDirectInput* SimInputDevice::getDInput()
{
   return ((SimInputManager*)group)->getDInput();
}   


//------------------------------------------------------------------------------
const SimInputDevice::ObjInfo* SimInputDevice::getObject(DWORD n)
{
   if (n >= objCount || objInfo == NULL) return (NULL);
   return (objInfo+n);
}   

  
//------------------------------------------------------------------------------
const char* SimInputDevice::getDeviceName()
{
   return (deviceName);
}


//------------------------------------------------------------------------------
const char* SimInputDevice::getName()
{
   return (diInstance.tszInstanceName);
}


//------------------------------------------------------------------------------
const char* SimInputDevice::getProductName()
{
   return (diInstance.tszProductName);
}







//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

#ifndef _SIMEV_H_
#define _SIMEV_H_

#include <ml.h>
#include <simBase.h>
#include "simEvDcl.h"

//--------------------------------------------------------------------------- 

class SimEditEvent: public SimEvent
{
public:
	SimEditEvent() { type = SimEditEventType; }
   SimEditEvent(const SimEditEvent &event) { *this = event; }
	UINT editorId;
	bool state;

	static void post(SimObject* notify, int editorId, bool state) {
		SimEditEvent* event = new SimEditEvent;
		event->editorId = editorId;
		event->state = state;
		notify->postCurrentEvent(event);
	}
};


//--------------------------------------------------------------------------- 

class SimTimeEvent: public SimEvent
{
public:
	SimTimeEvent() { type = SimTimeEventType; }
	SimTimeEvent(const SimTimeEvent &event) { *this = event; }

	static void post(SimObject* notify) {
		notify->postCurrentEvent(new SimTimeEvent);
	}
	static void post(SimObject* notify,SimTime time) {
		SimTimeEvent* event = new SimTimeEvent;
		event->time = time;
		notify->postEvent(event);
	}
};


//--------------------------------------------------------------------------- 

class SimMessageEvent: public SimEvent
{
public:
	SimMessageEvent() { type = SimMessageEventType; }
   SimMessageEvent(const SimMessageEvent &event) { *this = event; }
	int message;

   DECLARE_PERSISTENT( SimMessageEvent );
   void pack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);
   void unpack(SimManager *manager, Net::PacketStream *ps, BitStream *bstream);

	static void post(SimObject* notify,int msg) {
		SimMessageEvent* event = new SimMessageEvent;
		event->message = msg;
		notify->postCurrentEvent(event);
	}
	static void post(SimObject* notify,SimTime time,int msg) {
		SimMessageEvent* event = new SimMessageEvent;
		event->time = time;
		event->message = msg;
		notify->postEvent(event);
	}
};

//------------------------------------------------------------------------------

class SimMCINotifyEvent: public SimEvent
{
public:
	SimMCINotifyEvent() { type = SimMCINotifyEventType; }
   SimMCINotifyEvent(const SimMCINotifyEvent &event) { *this = event; }
	LPARAM devId;
	WPARAM flags;
};

//------------------------------------------------------------------------------

class SimMIXNotifyEvent: public SimEvent
{
public:
	SimMIXNotifyEvent() { type = SimMIXNotifyEventType; }
   SimMIXNotifyEvent(const SimMIXNotifyEvent &event) { *this = event; }
	LPARAM dwId;
	WPARAM hMixer;
   bool control; // control or line
};

//--------------------------------------------------------------------------- 

class SimFrameEndNotifyEvent: public SimEvent
{
public:
	SimFrameEndNotifyEvent() { type = SimFrameEndNotifyEventType; }
};

//--------------------------------------------------------------------------- 

class SimGainFocusEvent: public SimEvent
{
public:
	SimGainFocusEvent() { type = SimGainFocusEventType; }
   SimGainFocusEvent(const SimGainFocusEvent &event) { *this = event; }

	static void post(SimObject* notify) {
		SimGainFocusEvent* event = new SimGainFocusEvent;
		notify->postCurrentEvent(event);
	}
	static void post(SimObject* notify,SimTime time) {
		SimGainFocusEvent* event = new SimGainFocusEvent;
		event->time = time;
		notify->postEvent(event);
	}
};

//--------------------------------------------------------------------------- 

class SimLoseFocusEvent: public SimEvent
{
public:
	SimLoseFocusEvent() { type = SimLoseFocusEventType; }
   SimLoseFocusEvent(const SimLoseFocusEvent &event) { *this = event; }

	static void post(SimObject* notify) {
		SimLoseFocusEvent* event = new SimLoseFocusEvent;
		notify->postCurrentEvent(event);
	}
	static void post(SimObject* notify,SimTime time) {
		SimLoseFocusEvent* event = new SimLoseFocusEvent;
		event->time = time;
		notify->postEvent(event);
	}
};


//--------------------------------------------------------------------------- 

class SimGroupObjectIdEvent: public SimEvent
{
public:
   SimObjectId id;
	SimGroupObjectIdEvent(SimObjectId _id) { type = SimGroupObjectIdEventType; id=_id; }
   SimGroupObjectIdEvent(const SimGroupObjectIdEvent &event) { *this = event; }
};


//--------------------------------------------------------------------------- 

class SimObjectTransformEvent: public SimEvent
{
public:
   TMat3F tmat;
	SimObjectTransformEvent() { type = SimObjectTransformEventType; }
	SimObjectTransformEvent(const TMat3F &t) { type = SimObjectTransformEventType;  tmat = t; }
   SimObjectTransformEvent(const SimObjectTransformEvent &event) { *this = event; }

	static void post(SimObject* notify, const TMat3F &t) {
		SimObjectTransformEvent* event = new SimObjectTransformEvent(t);
		notify->postCurrentEvent(event);
	}
	static void post(SimObject* notify,SimTime time, const TMat3F &t) {
		SimObjectTransformEvent* event = new SimObjectTransformEvent(t);
		event->time = time;
		notify->postEvent(event);
	}
};


//--------------------------------------------------------------------------- 

class SimObjectTransformQuery: public SimQuery
{
public:
   TMat3F  tmat;
   Point3F centerOffset;
   SimObjectTransformQuery() { type = SimObjectTransformQueryType; }
};

class SimTimerEvent: public SimEvent
{
public:
   SimObjectId timerId;
	SimTime timerInterval;

	SimTimerEvent() { type = SimTimerEventType; }

	static void post(SimObject* notify) {
		notify->postCurrentEvent(new SimTimerEvent);
	}
	static void post(SimObject* notify,SimTime time) {
		SimTimerEvent* event = new SimTimerEvent;
		event->time = time;
		notify->postEvent(event);
	}
};


//--------------------------------------------------------------------------- 
class SimTriggerEvent: public SimEvent
{
public:
	enum Action
	{
      // Common
	   Activate,   
	   Deactivate, 
      Toggle,
	     
      // Starsiege
	   Heal,   
	   Reload,   

      // Tribes
	};
	
   Action action;
	float	 value;

	SimTriggerEvent() { type = SimTriggerEventType; }
	SimTriggerEvent(Action _action, float _value) { 
	   type = SimTriggerEventType; 
      action= _action; 
      value = _value;
	}
   SimTriggerEvent(const SimTriggerEvent &event) { *this = event; }

	static void post(SimObject *notify, Action action, float value) {
		SimTriggerEvent *event = new SimTriggerEvent(action, value);
		notify->postCurrentEvent(event);
	}
};  


class GFXSurface;

class SimRegisterTextureEvent : public SimEvent
{
  public:
   GFXSurface* pSurface;

	SimRegisterTextureEvent() {
	   type = SimRegisterTextureEventType;
	   pSurface = NULL;
	}
	SimRegisterTextureEvent(GFXSurface* io_pSurface) {
	   type     = SimRegisterTextureEventType;
	   pSurface = io_pSurface;
	}
};

typedef enum 
{ 
   SimOrthographicProjection, 
   SimPerspectiveProjection 
} SimCameraProjection;

//--------------------------------------------------------------------------- 
// Info need to intialize a TSPerspCamera

struct SimCameraInfo
{
   SimCameraProjection projectionType;
	float nearPlane;		// Distance to focal plane
	float farPlane;		// Distance to far plane
	float fov;				// Field of view angle for perspective cam.
   RectF worldBounds;	// Used for ortho cam.
   TMat3F tmat;
   ColorF alphaColor;
   float alphaBlend;
};


//--------------------------------------------------------------------------- 

class SimCameraQuery: public SimQuery
{
public:
	SimCameraQuery() { type = SimCameraQueryType; }
	SimCameraInfo cameraInfo;
};

#endif

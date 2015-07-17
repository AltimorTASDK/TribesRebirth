//--------------------------------------------------------------------------- 
//
// simCamera.h
//
//--------------------------------------------------------------------------- 

#ifndef _SIMCAMERA_H_
#define _SIMCAMERA_H_

#include <sim.h>
#include <simNetObject.h>

//--------------------------------------------------------------------------- 

struct SimCameraMountEvent : public SimEvent
{
   SimObject *prevCamera;  // previous camera
   SimObject *data;        // object of interest, path object, etc...
   SimCameraMountEvent() { 
      type = SimCameraMountEventType; 
      prevCamera = NULL;
   }
};

struct SimCameraUnmountEvent : public SimEvent
{
   SimCameraUnmountEvent() { type = SimCameraUnmountEventType; }
};


struct SimCameraRemountEvent : public SimEvent
{
   SimObject *data;        // object of interest, path object, etc...
   SimCameraRemountEvent() { 
      type = SimCameraRemountEventType; 
      data = NULL;
   }
};


//--------------------------------------------------------------------------- 

struct SimCameraNetEvent : public SimEvent
{
   SimObjectId cameraMountId;
   SimCameraNetEvent() { type = SimCameraNetEventType; }

   DECLARE_PERSISTENT(SimCameraNetEvent);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

//--------------------------------------------------------------------------- 

class SimCamera : public SimNetObject
{
   typedef SimNetObject Parent;

   protected:
      bool playback;
      bool mounted;
      SimObject *prevCameraMount;
      SimObject *cameraMount;
            
      bool onSimCameraNetEvent(const SimCameraNetEvent *event);
      bool onAdd();
      void onDeleteNotify(SimObject *object);

      bool onSimCameraQuery(SimCameraQuery *query);
      bool pickObject();

   public:
      SimCamera();
      bool processEvent(const SimEvent *);
      bool processQuery(SimQuery *);

      SimObject* getObject() { return cameraMount; }
      void setObject(SimObject *obj, SimObject *data=NULL);

	   DECLARE_PERSISTENT(SimCamera);
   	virtual Error read(StreamIO &, int version, int);
	   virtual Error write(StreamIO &, int version, int);
};

//--------------------------------------------------------------------------- 

#endif

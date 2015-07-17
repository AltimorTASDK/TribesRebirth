//--------------------------------------------------------------------------- 
//            
// simCamera.cpp
//
//--------------------------------------------------------------------------- 

#ifdef __BORLANDC__
#pragma warn -inl
#pragma warn -aus
#endif

#include <simCamera.h>

//--------------------------------------------------------------------------- 

SimCamera::SimCamera()
{
   playback = false;
   mounted = false;
   cameraMount = NULL;
   prevCameraMount = NULL;
}

//--------------------------------------------------------------------------- 

bool SimCamera::pickObject()
{
   if (getObject())
      return true;

   SimCameraMountSet *set = dynamic_cast<SimCameraMountSet *>
      (manager->findObject(SimCameraMountSetId));
   if (set) {
      SimCameraMountSet::iterator iter = set->begin();
      for (; iter != set->end(); iter++)  
      	if (!(*iter)->isRemoved()) {
            setObject(*iter);
            return true;
         }
   }
   setObject(NULL);
   return false;
}   

//--------------------------------------------------------------------------- 

bool SimCamera::onSimCameraQuery(SimCameraQuery *query)
{
   if (pickObject())
      return getObject()->processQuery(query);
   return false;
}

//--------------------------------------------------------------------------- 

bool SimCamera::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimCameraQuery);
      case SimObjectTransformQueryType: 
         if (cameraMount)
            return cameraMount->processQuery(query);
         else
            return false;
      default:
         return false;
   }
}

//--------------------------------------------------------------------------- 

void SimCamera::onDeleteNotify(SimObject *object)
{
   if(object == (SimObject *)cameraMount)
   {
      cameraMount = NULL;
      pickObject();
   }
   Parent::onDeleteNotify(object);
}

bool SimCamera::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimCameraNetEvent);
      case SimCameraMountEventType:
         mounted = true;
         if (cameraMount)
            cameraMount->processEvent(event);
         break;
         
      case SimCameraUnmountEventType:
         mounted = false;
         if (cameraMount)
            cameraMount->processEvent(event);
         break;            

      default:
         return false;
   }
   
   return true;
}

//--------------------------------------------------------------------------- 

bool SimCamera::onAdd()
{
   if (!id)
      manager->assignId(this);
   addToSet(SimCameraSetId);
   
   return Parent::onAdd();
}      

//--------------------------------------------------------------------------- 

void SimCamera::setObject(SimObject *newObject, SimObject *data)
{
   if (cameraMount == newObject)
      return;
      
   prevCameraMount = cameraMount;
   SimCameraMountEvent me;
   if (prevCameraMount) {
      clearNotify(prevCameraMount);
      if (mounted) {
         SimCameraUnmountEvent ume;
         prevCameraMount->processEvent(&ume);
         me.prevCamera = prevCameraMount;
      }         
   }
   me.data = data;
   if (newObject) {
      deleteNotify(newObject);
      if (mounted)
         newObject->processEvent(&me);
   }
   cameraMount = newObject;
}
   
//--------------------------------------------------------------------------- 

bool SimCamera::onSimCameraNetEvent(const SimCameraNetEvent *event)
{
   SimObject *cm = manager->findObject(event->cameraMountId);
   if (cm) 
      setObject(cm);

   return true;
}

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG(SimCamera, FOURCC('S','C','A','M'));

Persistent::Base::Error SimCamera::read(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = SimNetObject::read(sio, a, b);
   if (error == Persistent::Base::Ok) {
      UInt8 _playback = playback;
      sio.read(&_playback);
      playback = _playback;
   	return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
   }
   return error;
}

Persistent::Base::Error SimCamera::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = SimNetObject::write(sio, a, b);
   if (error == Persistent::Base::Ok) {
      sio.write((UInt8)playback);
   	return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
   }
   return error;
}

//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG(SimCameraNetEvent, FOURCC('S', 'C', 'N','E') );

Persistent::Base::Error SimCameraNetEvent::read(StreamIO &sio, int a, int b )
{
   SimEvent::read(sio, a, b);
   sio.read(&cameraMountId);
	return (sio.getStatus() == STRM_OK) ? Ok: ReadError;
}

Persistent::Base::Error SimCameraNetEvent::write( StreamIO &sio, int a, int b )
{
   SimEvent::write(sio, a, b);
   sio.write(cameraMountId);
	return (sio.getStatus() == STRM_OK) ? Ok: WriteError;
}

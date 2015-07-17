//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simMarker.h"
#include "esfObjectTypes.h"
#include <darkstar.strings.h>
#include <simPreLoadManager.h>
#include "simGame.h"

//-------------------------------------------------------------------------- 

SimMarker::SimMarker()
{
   inst = NULL;
   //netFlags.set(SimNetObject::ScopeAlways);

   // the following is temporary... only way to get ghosts at all
   //netFlags.set(SimNetObject::Ghostable);
}

//-------------------------------------------------------------------------- 

void SimMarker::ghostsOn()
{
   // intended to set the Ghostable bit
   
   // the next line is commented out because there's no way to unghost an object
   netFlags.set(SimNetObject::Ghostable);
   collisionImage.trigger = false;
}

void SimMarker::ghostsOff()
{
   // intended to clear the Ghostable bit
   
   // the next line is commented out because there's no way to unghost an object
   netFlags.clear(SimNetObject::Ghostable);
   collisionImage.trigger = true;
}
   
//-------------------------------------------------------------------------- 

void SimMarker::shapeOn()
{
   shapeOff();
   ResourceManager *rm = SimResource::get(manager);
   AssertFatal(rm, "SimMarker::shapeOn: No resource manager available");

   hShape = rm->load(SimTagDictionary::getString(manager, getDefaultShape()), true);
   
   AssertFatal((bool)hShape, "SimMarker::shapeOn: Shape could not be loaded");
   
   inst = new TS::ShapeInstance(hShape, *rm);
   image.shape = inst;   
   image.itype = SimRenderImage::Normal;

   // set the bounding box for collisions (but mark it as a trigger object
   // so that other objects won't bounce off it)
   
   RealF shapeRadius = inst->getShape().fRadius;
   collisionImage.radius = shapeRadius;
   collisionImage.center = inst->getShape().fCenter;
   boundingBox.fMin.set(-shapeRadius, -shapeRadius, -shapeRadius);
   boundingBox.fMax.set(+shapeRadius, +shapeRadius, +shapeRadius);
   setPosition(getTransform(),NULL,true);
}

void SimMarker::shapeOff()
{
   image.shape = NULL;
   if(inst)
      delete inst;
   inst = NULL;
   
   // no collisions, so just empty all this stuff out
   collisionImage.radius = 0;
   collisionImage.center.set();
   boundingBox.fMin.set();
   boundingBox.fMax.set();   
}
   
//------------------------------------------------------------------------------

bool SimMarker::processEvent(const SimEvent *event)
{
   switch (event->type) 
   {
      onEvent(SimEditEvent);
      onEvent(SimObjectTransformEvent);
   }   
   return false;
}  


//------------------------------------------------------------------------------
bool SimMarker::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimObjectTransformQuery);
      onQuery(SimCollisionImageQuery);
      onQuery(SimRenderQueryImage);
      onQuery(SimImageTransformQuery);
   }
   return false;
}   


//------------------------------------------------------------------------------

bool SimMarker::onAdd()
{
   setMaskBits(Moved);
   setPosition(getTransform(), NULL, true);
   if (!Parent::onAdd())
      return false;
   type = SimMarkerObjectType | SimMissionObjectType;

   // add to root container
   SimContainer *root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root,"SimMarker::addNotify: no root container");
   root->addObject(this);
      
   if (manager == SimGame::get()->getManager(SimGame::SERVER))
   {
      ghostsOff();   // server object only by default
      shapeOff();    // server object doesn't need a shape unless mission editor running
   }
   else
   {
      shapeOn();  // client object only exists for mission editor, so shape on
   }

   return true;
}

void SimMarker::onRemove()
{   
   shapeOff();
   Parent::onRemove();
}

void SimMarker::onPreLoad(SimPreLoadManager *splm)
{
   Parent::onPreLoad(splm);

   const char *shapeName = SimTagDictionary::getString(getDefaultShape());
   if (shapeName)
      splm->preLoadTSShape(shapeName);
}

//------------------------------------------------------------------------------

bool SimMarker::onSimEditEvent(const SimEditEvent *event)
{
   // this event is only sent to the server

   if( event->editorId == SimMissionEditorId )
   {
      if( event->state )
      {
         ghostsOn();
         shapeOn();     // mission editor is on, so shape is on
      }
      else
      {
         ghostsOff();
         shapeOff();    // mission editor is off, so shape is off
      }

      return true;
   }
         
   return false;
}

//------------------------------------------------------------------------------
DWORD SimMarker::packUpdate(Net::GhostManager *, DWORD mask, BitStream *stream)
{
   if (mask & Moved) {
      stream->writeFlag(true);
      stream->write(sizeof(getTransform()), &getTransform());
   }
   else
      stream->writeFlag(false);
   return 0;
}

//------------------------------------------------------------------------------
void SimMarker::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   if (stream->readFlag()) {
      TMat3F newTransform;
      stream->read(sizeof(newTransform), &newTransform);
      processEvent(&SimObjectTransformEvent(newTransform));
   }      
}
 
//------------------------------------------------------------------------------
void SimMarker::inspectRead(Inspect* insp)
{
//   Parent::inspectRead(insp);

   Point3F   pos;
   Point3F  rot;
   insp->read(IDITG_POSITION, pos);
   insp->read(IDITG_ROTATION, rot);
   
   SimObjectTransformQuery query;
   processQuery( &query );
   SimObjectTransformEvent newEvent(query.tmat);
   
   newEvent.tmat.set(EulerF (rot.x, rot.y, rot.z), pos);
   processEvent(&newEvent);
}   

//------------------------------------------------------------------------------

void SimMarker::inspectWrite(Inspect* insp)
{
//   Parent::inspectWrite(insp);
   TMat3F mat = getTransform();
   EulerF rot;
   
   mat.angles(&rot);
   insp->write(IDITG_POSITION, mat.p );
   insp->write(IDITG_ROTATION, Point3F (rot.x, rot.y, rot.z) );
}

//------------------------------------------------------------------------------

bool SimMarker::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
   // set the bits on all objects even though it only is read for the server
   setMaskBits(Moved);
   
   setPosition(event->tmat, NULL, true);
   return true;   
}

//------------------------------------------------------------------------------

bool SimMarker::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->tmat = getTransform();
   if(inst)
      query->centerOffset = inst->getShape().fCenter;
   
   return true;   
}

//------------------------------------------------------------------------------

bool SimMarker::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
   query->count = 1;            
   query->image[0] = &collisionImage;
   collisionImage.transform = getTransform();
   return true;
}

//------------------------------------------------------------------------------

bool SimMarker::onSimRenderQueryImage(SimRenderQueryImage* query)
{
        if (!image.shape)
           return false;
   query->count = 1;
   query->image[0] = &image;
   image.transform = getTransform();
   return true;
}

//------------------------------------------------------------------------------

bool SimMarker::onSimImageTransformQuery(SimImageTransformQuery* query)
{
   query->transform = getTransform();
   return true;
}

//------------------------------------------------------------------------------

void SimMarker::initPersistFields()
{
   addField("transform", TypeTMat3F, Offset(transform, SimMarker));
}

Persistent::Base::Error SimMarker::read(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::read(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.read(sizeof(transform), (void *) &transform);

   // set up bounding box, etc
   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error SimMarker::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = Parent::write(sio, a, b);
   if (error == Persistent::Base::Ok) {
      sio.write(sizeof(getTransform()), (void *)&getTransform());
      return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
   }
   return error;
}

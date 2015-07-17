//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

#include <sim.h>
#include <simResource.h>
#include <console.h>

#include <simDebris.h>
#include <simPartDebris.h>

#include "marker.h"
#include "fearDcl.h"
#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "stringTable.h"
#include "fearGlobals.h"
#include "dataBlockManager.h"


//------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( Marker, FOURCC('M','R','K','R'), MarkerPersTag );


//-------------------------------------------------------------------------- 

Marker::MarkerData::MarkerData()
{
}

void Marker::MarkerData::pack(BitStream* stream)
{
	Parent::pack(stream);
   stream->writeString(fileName);
}

void Marker::MarkerData::unpack(BitStream* stream)
{
	Parent::unpack(stream);
   char buf[256];
   stream->readString(buf);
   fileName = stringTable.insert(buf);
}


//-------------------------------------------------------------------------- 

Marker::Marker()
{
	type = MarkerObjectType;
   netFlags.set(SimNetObject::Ghostable | SimNetObject::ScopeLocal);
   image.shape = 0;
}

Marker::~Marker()
{
   delete image.shape;
}


//-------------------------------------------------------------------------- 

bool Marker::initResources(GameBaseData * data)
{
	if (!Parent::initResources(data))
		return false;
	if (loadShape() && isGhost()) {
		SimContainer* root = findObject(manager,SimRootContainerId,root);
		root->addObject(this);
	}
	setTeam(-1);
	setPos(lPosition,aPosition);
	return true;
}


//-------------------------------------------------------------------------- 

bool Marker::loadShape()
{
	if (image.shape)
		return true;

	MarkerData *data = static_cast<MarkerData*>(Parent::data);
	char name[256];
	strcpy(name,data->fileName);
	strcat(name,".dts");

   ResourceManager *rm = SimResource::get(manager);
	shape = rm->load(name,true);
	if (bool(shape)) {
		TS::ShapeInstance* si = new TS::ShapeInstance(shape, *rm);
		image.shape = si;
		image.itype = SimRenderImage::Normal;

		const TSShape& shape = si->getShape();
		boundingBox = shape.fBounds;
		collisionImage.bbox = boundingBox;
		collisionImage.sphere.center = shape.fCenter;
		collisionImage.sphere.radius = shape.fRadius;
	   collisionImage.shapeInst = si;
		collisionImage.collisionDetail = 0;
	   collisionImage.trigger = false;
		return true;
	}
	return false;
}

int Marker::getDatGroup()
{
	return DataBlockManager::MarkerDataType;
}

	
//------------------------------------------------------------------------------

void Marker::setTeam(int team)
{
	if (team == -1) {
		for (int i = 0; i < 8; i++)
			setAlwaysVisibleToTeam(i,true);
	}
	else {
		for (int i = 0; i < 8; i++)
			setAlwaysVisibleToTeam(i,false);
		setAlwaysVisibleToTeam(team,true);
	}
	Parent::setTeam(team);
}

void Marker::setPos(const Point3F &pos,const Point3F& rot)
{
	TMat3F mat;
	aPosition = rot;
	mat.set(EulerF(aPosition.x,aPosition.y,aPosition.z),pos);
	setTransform(mat);
	setMaskBits(PositionMask | RotationMask);
}


//------------------------------------------------------------------------------

bool Marker::processEvent(const SimEvent *event)
{
	switch (event->type) 
	{
      onEvent(SimEditEvent);
      onEvent(SimObjectTransformEvent);
   }   
   return false;
}  

bool Marker::onSimEditEvent(const SimEditEvent *event)
{
	if (event->editorId == SimMissionEditorId) {
   	if (event->state) {
			SimContainer* root = findObject(manager,SimRootContainerId,root);
			root->addObject(this);
		}
		else {
			if (getContainer())
				getContainer()->removeObject(this);
		}
	}
	return true;
}

bool Marker::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	EulerF rot;
	event->tmat.angles(&rot);
	aPosition.set(rot.x,rot.y,rot.z);
	setTransform(event->tmat);
	setMaskBits(PositionMask | RotationMask);
   return true;
}


//------------------------------------------------------------------------------

bool Marker::processQuery(SimQuery *query)
{
	switch (query->type)
	{
      onQuery(SimObjectTransformQuery);
      onQuery(SimCollisionImageQuery);
      onQuery(SimRenderQueryImage);
   }
   return false;
}   

bool Marker::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
	query->tmat = getTransform();
   return true;   
}

bool Marker::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
	query->count = 1;
	query->image[0] = &collisionImage;
	collisionImage.transform = getTransform();
	return true;
}

bool Marker::onSimRenderQueryImage(SimRenderQueryImage* query)
{
	if (image.shape) {
		query->count = 1;
		query->image[0] = &image;
		image.transform = getTransform();
		return true;
	}
	return false;
}


//------------------------------------------------------------------------------

void Marker::initPersistFields()
{
   Parent::initPersistFields();
   addField("position", TypePoint3F, Offset(lPosition, Marker));
   addField("rotation", TypePoint3F, Offset(aPosition, Marker));
}


//------------------------------------------------------------------------------

DWORD Marker::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   Parent::packUpdate(gm, mask, stream);
	if (stream->writeFlag(mask & InitialUpdate)) {
		packDatFile(stream);
	}
	if (stream->writeFlag(mask & (PositionMask | RotationMask))) {
		stream->write(sizeof( Point3F ), &lPosition);
		stream->write(sizeof( Point3F ), &aPosition);
   }
   return 0;
}

void Marker::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm, stream);
	if (stream->readFlag()) {
		unpackDatFile(stream);
	}
	if (stream->readFlag()) {
		Point3F pos,rot;
		stream->read(sizeof(pos), &pos);
		stream->read(sizeof(rot), &rot);
		setPos(pos,rot);
	}
}

 
//------------------------------------------------------------------------------

void Marker::inspectRead(Inspect* insp)
{
	Parent::inspectRead(insp);

   Point3F pos;
   Point3F rot;
   insp->read(IDITG_POSITION, pos);
   insp->read(IDITG_ROTATION, rot);
   setPos(pos,rot);
}   

void Marker::inspectWrite(Inspect* insp)
{
	Parent::inspectWrite(insp);
   TMat3F mat = getTransform();

   insp->write(IDITG_POSITION, mat.p );
   insp->write(IDITG_ROTATION, aPosition);
}

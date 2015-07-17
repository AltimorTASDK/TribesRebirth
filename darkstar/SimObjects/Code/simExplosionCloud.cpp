//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <SimTagDictionary.h>
#include <SimPersistTags.h>
#include <NetPacketStream.h>
#include <NetGhostManager.h>
#include <NetEventManager.h>
#include "soundFX.h"
#include "simExplosion.h"


// Imported from SimExplosion.cpp
//
extern Random g_expRand;

//------------------------------------------------------------------------------
// SimExplosionCloud
//------------------------------------------------------------------------------

float SimExplosionCloud::detailScale = 1.0f;

// this constructor is for circle, sphere, screen, and dome (sphere with topOnly)
SimExplosionCloud::SimExplosionCloud(float _megatons, Int32 _typeMask,
                                                int _form, bool _topOnly,
	                                             const Point3F & _center, float _radius,
	                                             const TMat3F * _transform)
{
   type |= SimExplosion::defaultFxObjectType;

	megatons=_megatons * detailScale;
	typeMask=_typeMask;

	form=_form;
	topOnly=_topOnly;
	center=_center;
	radius=_radius;
	hasTransform=_transform;
	if (_transform)
		transform=*_transform;

	// make a box out of a circle...
	if (form==Box)
	{
		box.fMin=box.fMax=center;
		box.fMin -= radius;
		box.fMax += radius;
	}

	detonationMax = 0;
	proportionNow = 0;
	soundId = -1;
	dataReceived = true;
	installDatabase(&database);
}

// this constructor is for box only -- _form is ignored
SimExplosionCloud::SimExplosionCloud(float _megatons, Int32 _typeMask,
                                                int _form, bool _topOnly,
	                                             const Box3F & _box, const TMat3F * _transform)
{
   type |= SimExplosion::defaultFxObjectType;

	megatons=_megatons * detailScale;
	typeMask=_typeMask;

	form=Box; // we'll ignore passed value if not actually a box...
	_form;
	topOnly=_topOnly;
	box=_box;
	
	hasTransform=_transform;
	if (_transform)
		transform=*_transform;

	detonationMax = 0;
	proportionNow = 1;
	soundId = -1;
	dataReceived = true;
	installDatabase(&database);
}

// this constructor copies just enough to send over the net
SimExplosionCloud::SimExplosionCloud(SimExplosionCloud & cloud)
{
   type |= SimExplosion::defaultFxObjectType;

	megatons = cloud.megatons;
	typeMask = cloud.typeMask;
	hasTransform = cloud.hasTransform;
	topOnly = cloud.topOnly;
	proportionNow = cloud.proportionNow;
	soundId = cloud.soundId;
	detonationMax = cloud.detonationMax;
	form = cloud.form;
	box = cloud.box;
	center = cloud.center;
	radius = cloud.radius;
	transform = cloud.transform;
}

void SimExplosionCloud::setDetonation(float _detonationMax, float _proportionNow)
{
	detonationMax = _detonationMax;
	proportionNow = _proportionNow;
}

void SimExplosionCloud::setSound(int _soundId)
{
	soundId = _soundId;
}

void SimExplosionCloud::makeSound()
{
   if (soundId != EXP_nosound)
   {
      Point3F p;
		if (form==Box)
		{
			center  = box.fMin;
			center += box.fMax;
			center *= 0.5f;
		}
		if (hasTransform)
			m_mul(center,transform,&p);
		else
			p = center;

      Sfx::Manager::PlayAt( manager, soundId, TMat3F(EulerF(0, 0, 0), p), Point3F(0, 0, 0));
   }
}

bool SimExplosionCloud::onAdd()
{
	if (!Parent::onAdd())
		return false;
	if (dataReceived)
		finishAddToManager();
	return true;
}

void SimExplosionCloud::finishAddToManager()
{
	// add to timer Set -- we'll give contained explosions timer updates ourself
	addToSet(SimTimerSetId);

	// add ourselves to the render Set so we can find out where camera is
	addToSet(SimRenderSetId);
	removeFromRenderSet = false;

	// add ourself to the root and set our bounding box
	SimContainer *root = NULL;
   
   root = findObject(manager, SimRootContainerId,root);
	containerMask = 0; // so nothing gets in...
	root->addObject(this);
	if (form==Box)
		setBoundingBox(box);
	else
		setBoundingBox(Box3F(center,center));
}

bool SimExplosionCloud::onSimTimerEvent(const SimTimerEvent * event)
{
	if (removeFromRenderSet)
	{
		removeFromSet(SimRenderSetId);
		removeFromRenderSet=false;
	}
	// explosions in a cloud are not added to timer set, but get updates from here
	Vector<SimExplosion*>::iterator itr;
	for (itr=database.explosionList.begin(); itr!=database.explosionList.end(); itr++)
		(*itr)->onSimTimerEvent(event);
	return true;
}

bool SimExplosionCloud::processEvent(const SimEvent * event)
{
	switch (event->type)
	{
		onEvent(SimTimerEvent);
      onEvent(SimMessageEvent);
	}
	return false;
}

bool SimExplosionCloud::onSimMessageEvent(const SimMessageEvent *)
{
   deleteObject();
   return true;
}

bool SimExplosionCloud::onSimRenderQueryImage(SimRenderQueryImage * q)
{
   if (removeFromRenderSet)
      return true;

	TSRenderContext & rc = *q->renderContext;
	// get camera pos
	Point3F camPos;
	TMat3F wCamMat = rc.getCamera()->getTCW();
   camPos = wCamMat.p;

	SimExplosionTable::ShortList sl;
	SimExplosionTable::getShortList(megatons,typeMask,sl,manager);

	// we can put delay in detonationTime after less than
	// following limit "megatons" are left to ignite
	float nowLimit = megatons * (1.0f-proportionNow);

	int numLeft = sl.size();
	while (numLeft)
	{
		int choose = g_expRand.getInt(0,numLeft);
		SimExplosion * explosion = new SimExplosion(-1,sl[choose].index);
		// add explosion to container...
		database.update(explosion);
		addObject(explosion);
		// set off the explosion
		lightFuse(explosion,camPos,megatons>nowLimit);
		megatons -= sl[choose].megatons;
		while (numLeft && sl[numLeft-1].megatons>megatons)
			numLeft--;
	}

	makeSound();

	// no longer need to be in render set
	removeFromRenderSet = true;

   if (database.explosionList.empty())
      SimMessageEvent::post(this,-1);

	return true;
}

bool SimExplosionCloud::processQuery(SimQuery * query)
{
	switch (query->type)
	{
		onQuery(SimRenderQueryImage);
	}
	return false;
}

// decide where to place and then add to the manager
void SimExplosionCloud::lightFuse(SimExplosion * explosion, Point3F & cameraPos, bool igniteNow)
{
	// decide when to detonate
	if (!igniteNow)
		explosion->setDetonationTime(g_expRand.getFloat(0,detonationMax));

	Point3F pos;
   Point3F vec(0, 0, 0);
	Point3F worldPos,worldVec;

	switch (form)
	{
		case Sphere :
		{
			// try to make sure explosions no closer together than 1m in z
			int maxZ = (int) radius;
			if (maxZ)
			{
				if (topOnly)
					vec.z = g_expRand.getInt(0,maxZ);
				else
					vec.z = g_expRand.getInt(-maxZ,maxZ);
			}
			// don't break, fall through to circle
		}
		case Circle :
		{
			float angle = g_expRand.getFloat(0,float(M_2PI));
			float circleRad = m_sqrtf(radius*radius - vec.z *vec.z);
			vec.x  = circleRad * m_cos(angle);
			vec.y  = circleRad * m_sin(angle);
			pos = center;
			pos += vec;
			break;
		}
		case Screen:
		{
			if (radius>0.1f) // if radius==0, skip some work
			{
				vec=cameraPos; 
				vec -= center; 
				vec.normalize(); 
				Point3F dirX,dirZ;
				if (fabs(vec.z) < 0.95)
				{
				   // vec is not near vector (0,0,1), so we can
   				// use it as the pivot vector
   				m_cross(vec, Point3F(0,0,1), &dirX);
					dirX.normalize();
   				m_cross(dirX, vec, &dirZ);
				}
				else
				{
				   // dirY is near vector (0,0,1), so use
   				// pivot Point3F(1,0,0) instead
   				m_cross(Point3F(1,0,0), vec, &dirZ);
   				dirZ.normalize();
   				m_cross(vec, dirZ, &dirX);
				}
				dirX *= g_expRand.getFloat(-radius,radius);
				dirZ *= g_expRand.getFloat(-radius,radius);
				vec *= radius;
				vec += dirX;
				vec += dirZ;
			}
			pos = center;
			pos += vec;
			break;
		}
		case Box :
		{
			int side;
			if (topOnly)
				side = g_expRand.getInt(1,5);
			else
				side = g_expRand.getInt(0,5);
			pos = box.fMax;
			switch (side)
			{
				case 0:
					pos.z = box.fMin.z;
				case 1:
					pos.x = g_expRand.getFloat(box.fMin.x,box.fMax.x);
					pos.y = g_expRand.getFloat(box.fMin.y,box.fMax.y);
					break;
				case 2:
					pos.y = box.fMin.y;
				case 3:
					pos.x = g_expRand.getFloat(box.fMin.x,box.fMax.x);
					pos.z = g_expRand.getFloat(box.fMin.z,box.fMax.z);
					break;
				case 4:
					pos.x = box.fMin.x;
				case 5:
					pos.y = g_expRand.getFloat(box.fMin.y,box.fMax.y);
					pos.z = g_expRand.getFloat(box.fMin.z,box.fMax.z);
					break;
			}
			switch (side)
			{
				case 0:
					vec.set(0,0,box.fMin.z-box.fMax.z);
					break;
				case 1:
					vec.set(0,0,box.fMax.z-box.fMin.z);
					break;
				case 2:
					vec.set(0,box.fMin.y-box.fMax.y,0);
					break;
				case 3:
					vec.set(0,box.fMax.y-box.fMin.y,0);
					break;
				case 4:
					vec.set(box.fMin.x-box.fMax.x,0,0);
					break;
				case 5:
					vec.set(box.fMax.x-box.fMin.x,0,0);
					break;
			}
			break;
		}
	}

	// now put into world coords
	if (hasTransform)
	{
		m_mul(pos,transform,&worldPos);
		m_mul(vec,(RMat3F&)transform,&worldVec);
	}
	else
	{
		worldPos=pos;
		worldVec=vec;
	}

	Point3F camAxis = worldPos;
	camAxis -= cameraPos;
	if (m_dot(camAxis,worldVec)>0.0f)
	{
		worldPos -= worldVec;
		worldPos -= worldVec;
	}
	explosion->setPosition(worldPos);
	explosion->setAxis(worldVec); // move away from the building... ;->
	explosion->setSound(false);
	manager->addObject(explosion);
}

void SimExplosionCloud::createOnClients(SimExplosionCloud * cloud, SimManager * mgr)
{
   SimSet * packetStreams = static_cast<SimSet*>(mgr->findObject(PacketStreamSetId));
   if (packetStreams)
   {
      for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
      {
         Net::PacketStream * pStream = dynamic_cast<Net::PacketStream *>(*itr);
         if (pStream && pStream->getGhostManager()->getCurrentMode() ==
                        Net::GhostManager::GhostNormalMode)
         {
            Net::RemoteCreateEvent * addEvent = new Net::RemoteCreateEvent;
            SimExplosionCloud *cloudCopy = new SimExplosionCloud(*cloud);
            addEvent->newRemoteObject = cloudCopy;
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }
      }
   }
   if (!cloud->manager)
      // this was a temporary object
      delete cloud;
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
// Standard Persistent IO stuff
//
//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG( SimExplosionCloud, SimExplosionCloudPersTag );

void SimExplosionCloud::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
	UInt8 megaI;
	stream->read(sizeof(megaI), (void *) &megaI);
	megatons = megaI * 25;
	megatons *= detailScale;

	stream->read( sizeof(typeMask), (void *) &typeMask);

	hasTransform = stream->readFlag();

	topOnly = stream->readFlag();

	stream->readBits(3,(void*)&form);

	UInt8 tmpInt;
	stream->readBits(4,(void*)&tmpInt);
	proportionNow = ((float) (tmpInt&0x0f)) / 15.0f;

	if (stream->readFlag())
		stream->readBits(32,(void*)&soundId); // note:  for now just read 32 bits
		                                      //        eventually reduce to 8
	else
		soundId = -1;

	UInt8 detI;
	stream->readBits(6, (void *) &detI);
	detonationMax = .1f * (float) detI;

	if (form==Box)
		stream->read( sizeof(box), (void *) &box);
	else
	{
		stream->read( sizeof(center), (void *) &center);
		UInt8 radI;
		stream->read( sizeof(radI), (void *) &radI);
		radius = .5f * (float) radI;
	}

	if (hasTransform)
		stream->read( sizeof(transform), (void *) &transform );

	dataReceived = true;

	if (manager)
		finishAddToManager();
}

DWORD SimExplosionCloud::packUpdate(Net::GhostManager *, DWORD, BitStream *stream)
{
	UInt8 megaI = (megatons / (detailScale*25.0f));
	stream->write(sizeof(megaI), (void *) &megaI);
	stream->write( sizeof(typeMask), (void *) &typeMask);

	stream->writeFlag(hasTransform);
	stream->writeFlag(topOnly);
	stream->writeBits(3,(void*)&form);

	UInt8 tmpInt = proportionNow * 15.0f;
	stream->writeBits(4,(void*)&tmpInt);

	if (soundId!=-1)
	{
		stream->writeFlag(true);
		stream->writeBits(32,(void*)&soundId); // note:  for now just read 32 bits
		                                       //        eventually reduce to 8
	}
	else
		stream->writeFlag(false);

	UInt8 detI = detonationMax * 10.0f;
	stream->writeBits(6, (void *) &detI);

	if (form==Box)
		stream->write( sizeof(box), (void *) &box);
	else
	{
		stream->write( sizeof(center), (void *) &center);
		UInt8 radI = radius * 2.0f;
		stream->write( sizeof(radI), (void *) &radI);
	}

	if (hasTransform)
		stream->write( sizeof(transform), (void *) &transform );
   return 0;
}


//---------------------------------------------------------
// container methods for explosion cloud
//---------------------------------------------------------

//------------- Internal Data base methods ----------------

void SimExplosionCloud::Database::update(SimContainer * object)
{
	SimContainer* cnt = object->getContainer();
	Box3F box = cnt->getBoundingBox();
	Box3F explosionBox = object->getBoundingBox();
	box.fMin.setMin(explosionBox.fMin);
	box.fMax.setMax(explosionBox.fMax);
	cnt->setBoundingBox(box);
}

bool SimExplosionCloud::Database::add(SimContainer* object)
{
	SimExplosion * explosion = static_cast<SimExplosion*>(object);
	explosion->setContainerData(explosionList.size());
	explosionList.push_back(explosion);
	update(object);
	return true;
}

bool SimExplosionCloud::Database::remove(SimContainer * object)
{
	int index = object->getContainerData();
	SimExplosion ** indexed = &explosionList[index];
	*indexed = explosionList.last();
	(*indexed)->setContainerData(index);
	explosionList.decrement();

	// last one? delete ourself
	if (explosionList.empty())
      SimMessageEvent::post(object->getContainer(),-1);
	return true;
}

bool SimExplosionCloud::Database::findIntersections(const SimContainerQuery&,SimContainerList*)
{
	return false;
}

bool SimExplosionCloud::Database::findContained(const SimContainerQuery&,SimContainerList*)
{
	return false;
}

SimContainer* SimExplosionCloud::Database::findOpenContainer(const SimContainerQuery&)
{
	return 0;
}

bool SimExplosionCloud::Database::removeAll(SimContainer *)
{
	explosionList.clear();
	return true;
}

void SimExplosionCloud::Database::render(SimContainerRenderContext& rc)
{
	// Render explosions
   const int size = explosionList.size();
	Vector<SimExplosion*>::iterator itr;
	for (itr = explosionList.begin(); itr != explosionList.end(); itr++ )
		rc.render(*itr);
      
	AssertFatal(size == explosionList.size(),
	  "object changed containers during render cycle");
}

void SimExplosionCloud::Database::scope(SimContainerScopeContext&)
{
	// Only the object itself should be ghosted, in fact
	// we should probably never get here as this object
	// should only exists on a client.
}

//----- Data base access methods for explosion cloud ----------

bool SimExplosionCloud::updateObject(SimContainer * object)
{
	// once in, explosion never gets out, but container bbox may change:
	database.update(object);
	return true;
}


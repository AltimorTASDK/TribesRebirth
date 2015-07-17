//---------------------------------------------------------------------------
//
// LSStampInst.cpp
//
//---------------------------------------------------------------------------

#include <LSStampInst.h>
#include <simtagdictionary.h>
#include <simEvDcl.h>
#include <LSTerrain.h>

void LSStampInstance::convertRotationToEuler( LS_ROT_90S rot, EulerF & eul )
{
	switch (rot)
	{
		case CCW_0:
			eul.set(0.0f,0.0f,0.0f);
			break;
		case CCW_90:
			eul.set(0.0f,0.0f,M_PI*.5f);
			break;
		case CCW_180:
			eul.set(0.0f,0.0f,M_PI);
			break;
		case CCW_270:
			eul.set(0.0f,0.0f,M_PI*1.5f);
			break;
		default:
			break;
	}
}

LS_ROT_90S LSStampInstance::convertVectorToRotation(Point2F & p)
{
	if (p.y>=0.0f && fabs(p.x)<=p.y)
		return CCW_0;
	if (p.y<=0.0f && fabs(p.x)<=-p.y)
		return CCW_180;
	if (p.x>=0.0f && fabs(p.y)<=p.x)
		return CCW_270;
	if (p.x<=0.0f && fabs(p.y)<=-p.x)
		return CCW_90;
	AssertFatal(1,"LSStampInstance::convertVectorToRotation:  cannot get here.");
	return (LS_ROT_90S) 0;
}

// static method for applying all stamps in stamp group
// to given terrain
bool LSStampInstance::applyStamps(LSTerrain * terrain, const Point2I & blk, SimManager * m)
{
	SimSet *set = (SimSet*)m->findObject( LSStampSetId );

	if (!set)
		return true; // no stamps, return w/o error

	for (SimSet::iterator itr = set->begin(); itr!=set->end(); itr++)
	{
		LSStampInstance * stamp = dynamic_cast<LSStampInstance *>(*itr);
		const Point2I & block = stamp->getBlock(terrain);
		if (block.x!=blk.x || block.y!=blk.y)
			continue;
		if ( !stamp || !stamp->applyStamp(terrain) )
				return false;
	}

	terrain->setHeightField(blk);

	return true;
}

void LSStampInstance::clearStamps( SimManager * m)
{
	SimSet * set = (SimSet*) m->findObject( LSStampSetId );
	if (set)
		m->deleteObject( set );
}

// method for applying this stamp to terrain
bool LSStampInstance::applyStamp( LSTerrain * terrain, bool blockValid)
{
	AssertFatal( hasResources,"LSStampInst::applyStamp:  no stamp set.");
	if (!resourcesLoaded)
		loadResources();

	if (!blockValid)
		getBlock(terrain);

	return terrain->applyTerrainStamp(block,origin,rotation,fpStamp);
}

bool LSStampInstance::setStamp(DWORD id)
{
	useID=true;

	stampID = id;
	char * name = SimTagDictionary::getString(manager, stampID);
	frStamp = SimResource::get(manager)->load( name );
	fpStamp = (LSTerrainStamp *) frStamp;

	hasResources=fpStamp;
	resourcesLoaded=true;
	return hasResources;
}

bool LSStampInstance::setStamp(LSTerrainStamp * pStamp)
{
	useID=false;

	fpStamp= pStamp;

	hasResources=true;
	resourcesLoaded=true;
	return true;
}

const Point2I & LSStampInstance::getBlock(LSTerrain * terrain)
{
	// get origin in terrain space
	const TMat3F & worldToTerr = terrain->getTerrain()->getInvTransform();
	Point3F torg;
	m_mul(origin,worldToTerr,&torg);

	// find block # from transformed origin
	GridFile * tfile = terrain->getTerrain()->getGridFile();
	int groundShift = (tfile->getDetailCount()-1) + tfile->getScale();

	block.x = int(torg.x) >> groundShift;
	block.y = int(torg.y) >> groundShift;

	return block;
}

void LSStampInstance::loadResources()
{
	if (hasResources)
		AssertFatal( setStamp(stampID),"LSStampInst::loadResources: unable to load terrain stamp");
}

void LSStampInstance::unloadResources()
{
	if (hasResources && !useID)
		delete fpStamp;
}

bool LSStampInstance::onAdd()
{
   if (Parent::onAdd() == false)
      return false;
   
   SimSet *set = (SimSet*) manager->findObject( LSStampSetId );
   if ( set ) {
   	set->addObject( this );
   } else {
   	// create the group
   	set = (SimSet*) manager->addObject( new SimSet, LSStampSetId );
   	AssertFatal(set,"LSStampInst::processEvent:  could not add stamp group");
   	set->addObject( this );
   }

   return true;
}


bool LSStampInstance::processEvent(const SimEvent *event)
{
	switch (event->type)
	{
		case SimObjectTransformEventType:
		{
			const SimObjectTransformEvent * e = static_cast<const SimObjectTransformEvent *>(event);
			origin = e->tmat.p;
			Point3F tmpP;
			m_mul(Point3F(0.0f,1.0f,0.0f),e->tmat,&tmpP);
			rotation = convertVectorToRotation(tmpP);
	      return true;
		}
		default:
			return Parent::processEvent(event);
	}
}

bool LSStampInstance::processQuery(SimQuery *query)
{
	switch (query->type)
	{
		case SimObjectTransformQueryType:
		{
			SimObjectTransformQuery * q = static_cast<SimObjectTransformQuery *>(query);
			EulerF euler;
			convertRotationToEuler(rotation,euler);
			q->tmat.set( euler, origin );
			q->centerOffset.set(0);
			
         return true;
		}
		default:
			return Parent::processQuery(query);
	}
}

LSStampInstance::LSStampInstance()
{
	hasResources=false;
}

LSStampInstance::~LSStampInstance()
{
	unloadResources();
}

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG(LSStampInstance,FOURCC('L','S','S','I'));

Persistent::Base::Error LSStampInstance::write(StreamIO &sio, int, int)
{
	AssertFatal(hasResources && useID,"LSStampInst::write:  can\'t write unless stamp set with tag");

	if (!origin.write( sio ))								return Persistent::Base::WriteError;
	if (!sio.write(sizeof(rotation),&rotation))		return Persistent::Base::WriteError;
	if (!sio.write(stampID))								return Persistent::Base::WriteError;

	return Persistent::Base::Ok;   
}

Persistent::Base::Error LSStampInstance::read(StreamIO &sio, int, int)
{

	if (!origin.read( sio ))								return Persistent::Base::ReadError;
	if (!sio.read(sizeof(rotation),&rotation))		return Persistent::Base::ReadError;
	if (!sio.read( &stampID ))								return Persistent::Base::ReadError;
	hasResources=useID=true;
	resourcesLoaded=false;

	return Persistent::Base::Ok;   
}   



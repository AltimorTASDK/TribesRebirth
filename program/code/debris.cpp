#include "simRenderGrp.h"
#include <Debris.h>
#include <explosion.h>
#include <SimTerrain.h>
#include <SimPersistTags.h>
#include <Soundfx.h>
#include <NetPacketStream.h>
#include <NetGhostManager.h>
#include <NetEventManager.h>
#include <ml.h>
#include "simResource.h"
#include "console.h"
#include "fearDcl.h"
#include <stringTable.h>
#include "fearGlobals.h"
#include "datablockmanager.h"

//------------------------------------------------------------------------------
//-------------------------------------- Dynamic Data structure
//
Debris::DebrisData::DebrisData()
{
   type                  = 0;
   pShapeName            = NULL;
   imageType             = 0;
   imageSortValue        = 0;
   mass                  = 1.0f;
   elasticity            = 1.0f;
   friction              = 1.0f;
   center[0]             = 0.0f;
   center[1]             = 0.0f;
   center[2]             = 0.0f;
   collisionMask         = SimTerrainObjectType | SimInteriorObjectType;
   knockMask             = 0;
   animationSequence     = -1;
   animRestartTime       = 0.0f;
   minTimeout            = 1.0f;
   maxTimeout            = 2.0f;
   explodeOnBounce       = 0.3f;
   damage                = 100;
   damageThreshold       = 1;
   spawnedDebrisMask     = 1;
   spawnedDebrisStrength = 50.0f;
   spawnedDebrisRadius   = 0.2;
   spawnedExplosionID    = -1;
   explodeOnRest         = true;
   collisionDetail       = 0;
}

void
Debris::DebrisData::pack(BitStream* bs)
{
   Parent::pack(bs);

   bs->write(type);
   bs->writeString(pShapeName);
   bs->write(imageType);
   bs->write(imageSortValue);
   bs->write(mass);
   bs->write(elasticity);
   bs->write(friction);
   bs->write(sizeof(float) * 3, (void*)center);
   bs->write(collisionMask);
   bs->write(knockMask);
   bs->write(animationSequence);
   bs->write(animRestartTime);
   bs->write(minTimeout);
   bs->write(maxTimeout);
   bs->write(explodeOnBounce);
   bs->write(damage);
   bs->write(damageThreshold);
   bs->write(spawnedDebrisMask);
   bs->write(spawnedDebrisStrength);
   bs->write(spawnedDebrisRadius);
   bs->write(spawnedExplosionID);
   bs->write(p);
   bs->writeFlag(explodeOnRest);
   bs->writeInt(collisionDetail, 2);
}

void
Debris::DebrisData::unpack(BitStream* bs)
{
   Parent::unpack(bs);

   bs->read(&type);

   char buf[256];
   bs->readString(buf);
   pShapeName = stringTable.insert(buf, true);

   bs->read(&imageType);
   bs->read(&imageSortValue);
   bs->read(&mass);
   bs->read(&elasticity);
   bs->read(&friction);
   bs->read(sizeof(float) * 3, (void*)center);
   bs->read(&collisionMask);
   bs->read(&knockMask);
   bs->read(&animationSequence);
   bs->read(&animRestartTime);
   bs->read(&minTimeout);
   bs->read(&maxTimeout);
   bs->read(&explodeOnBounce);
   bs->read(&damage);
   bs->read(&damageThreshold);
   bs->read(&spawnedDebrisMask);
   bs->read(&spawnedDebrisStrength);
   bs->read(&spawnedDebrisRadius);
   bs->read(&spawnedExplosionID);
   bs->read(&p);

   explodeOnRest   = bs->readFlag();
   collisionDetail = bs->readInt(2);
}








#define IN_SERVER (manager->isServer())
#define NORMALIZE_MASK (0x04) // normalize matrix every 8th timer update
#define DEBRIS_MAX_MOVE_PER_DT (5.0f)
//------------------------------------------------------------------------------ 
// GridPolyStore -- used for storing what terrain polys we are over, for
//                  speeding up collision checking for debris
//------------------------------------------------------------------------------

class GridPolyStore
{
	public:
	enum { Overflow=-99};

	struct GridPoly
	{
		Point2F v1,v2,v3;
		Point3F normal;
		float k;
	};
	
	private:
	const int maxPolys;            // maximum number of GridPolys allowed in list
	int numPolys;                  // number of grid polys currently in list
	int hiPoly;                    // highest grid poly index used
	GridPoly * polys;	             // the grid polys
	int * lockCount;               // how many users of each poly

	// copied from math library, but specialized for here
	bool pointInTriangle(Point3F & pos, GridPoly & triangle);

	public:

	GridPolyStore(int _maxPolys);
	~GridPolyStore();

	GridPoly * getPoly(Point3F & pos, int & curPoly,SimManager*); // currPoly == -1 if none
	bool constructPoly(Point3F & pos, GridPoly & poly, SimTerrain*);
	void releasePoly(int pIndex);
	void lockPoly(int pIndex);

} gridPolyStore(50);

GridPolyStore::GridPolyStore(int _maxPolys) : maxPolys(_maxPolys)
{
	numPolys=0;
	hiPoly = -1;
	polys = new GridPoly[maxPolys];
	lockCount = new int[maxPolys];
}

GridPolyStore::~GridPolyStore()
{
	delete [] polys;
	delete [] lockCount;
}

bool GridPolyStore::pointInTriangle(Point3F & pos, GridPoly & triangle)
{
	Point2F & v1 = triangle.v1;
	Point2F & v2 = triangle.v2;
	Point2F & v3 = triangle.v3;

	float dot1,dot2,dot3;
	dot1 = (v1.x-pos.x) * (v1.y-v2.y) + (v1.y-pos.y) * (v2.x-v1.x);
	dot2 = (v2.x-pos.x) * (v2.y-v3.y) + (v2.y-pos.y) * (v3.x-v2.x);
	dot3 = (v3.x-pos.x) * (v3.y-v1.y) + (v3.y-pos.y) * (v1.x-v3.x);

	return (dot1*dot2>=0 && dot2*dot3>=0);
}

// first checks to see if curPoly is still correct
// if not, then searches poly store for current poly
// if not there, gets poly from terrain
// in case of overflow, returns currPoly == GridPolyStore::Overflow
// and null grid poly
GridPolyStore::GridPoly * GridPolyStore::getPoly(Point3F & pos, int & curPoly, 
                                                 SimManager *mgr)
{
	// currPoly still valid?
	if ( curPoly>=0 && pointInTriangle(pos,polys[curPoly]) )
		return &polys[curPoly];

	// currPoly no longer valid
	if (curPoly>=0)
		releasePoly(curPoly);
	curPoly = -1;

	// is another stored poly valid?
	int i;
	for (i=0; i<=hiPoly; i++)
		if ( lockCount[i] && pointInTriangle(pos,polys[i]) )
		{
			curPoly = i;
			lockCount[i]++;
			return &polys[i];
		}

	// is the buffer full?
	if (numPolys==maxPolys)
	{
		curPoly = Overflow;
		return 0;
	}

	// we'll need a terrain for the next step...
	SimTerrain * terrain = static_cast<SimTerrain *>(mgr->findObject(SimTerrainId));
	if (!terrain)
	{
		curPoly = -1;
		return 0;
	}

	// get a new poly
	for (i=0; i<=hiPoly && lockCount[i]; i++);
	if (constructPoly(pos,polys[i],terrain))
	{
		curPoly = i;
		numPolys++;
		if (curPoly>hiPoly)
			hiPoly=curPoly;
		lockCount[curPoly]=1;
		return &polys[curPoly];
	}
	else
		return 0;
}

void GridPolyStore::lockPoly(int pIndex)
{
	lockCount[pIndex]++;
}

void GridPolyStore::releasePoly(int pIndex)
{
	if (!--lockCount[pIndex])
	{
		// last one to index this poly
		// update hiPoly index if necessary
		if (pIndex==hiPoly)
			for (hiPoly--; hiPoly>=0 && !lockCount[hiPoly]; hiPoly--);
		numPolys--;
	}
}

bool GridPolyStore::constructPoly(Point3F & pos, GridPoly & p, SimTerrain * terrain)
{
	// assumes terrain is set

	Point3F tp;
	m_mul(pos,terrain->getInvTransform(),&tp);

	GridFile * file = terrain->getGridFile();

	int groundShift = (file->getDetailCount() - 1) + file->getScale();

	Point2I bpos;
	bpos.x = int(tp.x) >> groundShift;
	bpos.y = int(tp.y) >> groundShift;

	Point2I size = file->getSize();

	GridBlock * block;
	if (bpos.x >=0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y &&
	    (block = file->getBlock(bpos)) != NULL)
	{
		// get position in block space
		Point2F blockOffset;
		blockOffset.x = tp.x - float(bpos.x << groundShift);
		blockOffset.y = tp.y - float(bpos.y << groundShift);

		// get square and offset
		Point2I spos;
		spos.x = int(blockOffset.x) >> file->getScale();
		spos.y = int(blockOffset.y) >> file->getScale();
		if (spos.x < 0 || spos.x >= block->getSize().x ||
		    spos.y < 0 || spos.y >= block->getSize().y)
			return false; // square out of range (should never happen)
		if (block->getMaterial(0,spos)->getEmptyLevel())
			return false; // empty square
		Point2F squareOffset;
		squareOffset.x = blockOffset.x - float(spos.x << file->getScale());
		squareOffset.y = blockOffset.y - float(spos.y << file->getScale());

		// use height struct to get normal
		GridHeight ht;
		ht.width = float(1 << file->getScale());
		ht.heights = block->getHeight(0,spos);
		ht.split = ((spos.x ^ spos.y) & 1) ? GridHeight::Split135 : GridHeight::Split45;
		ht.dx = 1;
		ht.dy = block->getHeightMapWidth();
		ht.getNormal(squareOffset,&p.normal);
		p.normal.normalize();

		// now get actual poly
		Point3F somePoint;
		Point3F v1,v2,v3;
		if (ht.split == GridHeight::Split45)
		{
			if (squareOffset.x<squareOffset.y)
			{
				v1.set(0,0);
				v2.set(0,ht.width);
				v3.set(ht.width,ht.width);
				somePoint.x = v1.x;
				somePoint.y = v1.y;
				somePoint.z = (*ht.heights).height;
			}
			else
			{
				v1.set(0,0);
				v2.set(ht.width,ht.width);
				v3.set(ht.width,0);
				somePoint.x = v1.x;
				somePoint.y = v1.y;
				somePoint.z = (*ht.heights).height;
			}
		}
		else
		{
			if (ht.width-squareOffset.x>squareOffset.y)
			{
				v1.set(0,0);
				v2.set(0,ht.width);
				v3.set(ht.width,0);
				somePoint.x = v1.x;
				somePoint.y = v1.y;
				somePoint.z = (*ht.heights).height;
			}
			else
			{
				v1.set(0,ht.width);
				v2.set(ht.width,ht.width);
				v3.set(ht.width,0);
				somePoint.x = v1.x;
				somePoint.y = v1.y;
				somePoint.z = (ht.heights[ht.dy]).height;
			}
		}

		// now put v1,v2,v3 and somePoint into world space (from square space)
		float sx,sy;
		Point3F tmpP;
		sx = (spos.x << file->getScale()) + (bpos.x << groundShift);
		sy = (spos.y << file->getScale()) + (bpos.y << groundShift);

		v1.x += sx;
		v1.y += sy;
		v1.z = 0;
		m_mul(v1,terrain->getTransform(),&tmpP);
		p.v1.x = tmpP.x;
		p.v1.y = tmpP.y;

		v2.x += sx;
		v2.y += sy;
		v2.z = 0;
		m_mul(v2,terrain->getTransform(),&tmpP);
		p.v2.x = tmpP.x;
		p.v2.y = tmpP.y;

		v3.x += sx;
		v3.y += sy;
		v3.z = 0;
		m_mul(v3,terrain->getTransform(),&tmpP);
		p.v3.x = tmpP.x;
		p.v3.y = tmpP.y;

		somePoint.x += sx;
		somePoint.y += sy;
		m_mul(somePoint,terrain->getTransform(),&tmpP);
		p.k = m_dot(p.normal,tmpP);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------ 

// a random number generator just for debris stuff
Random dbRand;

#define numRandomRotations 16
RMat3F randomRotations[numRandomRotations] =
{
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) )),
   RMat3F(EulerF(dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI),
                 dbRand.getFloat(-M_PI,M_PI) ))
};

Debris::Debris(const Debris &from)
{
   // Set the gamebase dat file name...
   datFileId    = from.datFileId;
   m_debrisData = from.m_debrisData;

	debrisMass         = from.debrisMass;
	animationSequence  = from.animationSequence;
	animRestartTime    = from.animRestartTime;
   duration           = from.duration;
   spawnedExplosionId = from.spawnedExplosionId;
   spawnImpulse       = from.spawnImpulse;
   explodeProb        = from.explodeProb;
   elastic            = from.elastic;
   sinkDepth          = from.sinkDepth;
   explodeOnRest      = from.explodeOnRest;
   shapeDetailScale   = from.shapeDetailScale;

	ghostMe            = from.ghostMe;
	spawnTimeout       = from.spawnTimeout;
   setTransform(from.getTransform());
   setLinearVelocity(from.lVelocity);
   setAngulerVelocity(from.aVelocity);

   m_debrisData = NULL;

   if (from.shapeName)
      shapeName = stringTable.insert(from.shapeName, true);

	flags.set(UseAngulerVelocity);
	flags.set(UseElasticity);
   flags.clear(AlwaysCalcAVelocity);
	image.shape=0;
   hitThisTime=hitLastTime=false;
	collisionMask = 0;
   avgLV.set(0,0,0);
   avgAV.set(0,0,0);
   potentialEnergyDeficit = 0;
   underSurfaceLastTime = false;
   inCloud = false;
	expire = false;

	gridPoly = -1; // GridPolyStore code: set to no grid poly

   // temp. measure so forces get added when added to container
   forceMask = -1;

	netFlags.clear(Ghostable);
}

Debris::Debris(int in_datFileId, float spawnTO, bool ghost)
{
   // Set the gamebase dat file name...
   datFileName = NULL;
   datFileId   = in_datFileId;
   AssertFatal(datFileId != -1, "Error, must have a dat file");

   m_debrisData = NULL;
   shapeName = NULL;
	debrisMass = -1;
	animationSequence = -1;
	animRestartTime = -1;
   duration = -1;
   spawnedExplosionId = -1;
   spawnImpulse = -1;
   explodeProb = -1;
   explodeOnRest = true;
   elastic = -1;
   sinkDepth = 0;
	ghostMe=ghost;
	spawnTimeout=spawnTO;
	flags.set(UseAngulerVelocity);
	flags.set(UseElasticity);
   flags.clear(AlwaysCalcAVelocity);
	image.shape=0;
   shapeDetailScale=NULL;
   hitThisTime=hitLastTime=false;
	collisionMask = 0;
   avgLV.set(0,0,0);
   avgAV.set(0,0,0);
   potentialEnergyDeficit = 0;
   underSurfaceLastTime = false;
   inCloud = false;
	expire = false;

	gridPoly = -1; // GridPolyStore code: set to no grid poly

   // temp. measure so forces get added when added to container
   forceMask = -1;

	netFlags.clear(Ghostable);
}

int
Debris::getDatGroup()
{
   return DataBlockManager::DebrisDataType;
}

bool
Debris::initResources(GameBase::GameBaseData* io_pDat)
{
   if (Parent::initResources(io_pDat) == false)
      return false;

   m_debrisData = dynamic_cast<DebrisData*>(io_pDat);
   return m_debrisData != NULL;
}


// this method only used if you want to over-ride shape, animation seq., and/or mass
void Debris::setShape(int _replaceMass,
                      int _replaceAnimation, float _replaceRestart)
{
	debrisMass = _replaceMass;
	setMass(debrisMass); // in case caller wants to apply a force
	animationSequence = _replaceAnimation;
	animRestartTime = _replaceRestart;
}

// this method only used if you want to over-ride shape, animation seq., and/or mass
void Debris::setShape(const char *_replaceName, int _replaceMass,
                      int _replaceAnimation, float _replaceRestart)
{
   AssertFatal(_replaceName && strlen(_replaceName) > 0, "bad debris shape name");

   shapeName = stringTable.insert(_replaceName, true);

	debrisMass = _replaceMass;
	setMass(debrisMass); // in case caller wants to apply a force
	animationSequence = _replaceAnimation;
	animRestartTime = _replaceRestart;
}

void Debris::setDuration(float _duration)
{
   duration = _duration;
}

Debris::~Debris()
{
	if (image.shape)
		delete image.shape;

	// release terrain poly from gridPolyStore
	if (gridPoly>=0)
		gridPolyStore.releasePoly(gridPoly);
}

void Debris::setCollisionMask(int _newMask)
{
   collisionMask = _newMask;
   if (inCloud)
      static_cast<DebrisCloud*>(getContainer())->addCollisionMask(_newMask);
}
//--------------------------------------------------------------------------- 


bool Debris::processEvent(const SimEvent* event)
{
	switch (event->type) 
	{
		onEvent(SimTimerEvent);
      onEvent(SimMessageEvent);
		default:
			return false;
	}
}

bool Debris::onAdd()
{
   // count # of timer updates, used for determining if
   // we've had an update yet and for normalizing matrix
   // every so often
   timerCounter = 0;

	if (!Parent::onAdd())
		return false;
   
   finishAddToManager();
	// else, initial packet not yet received
	return true;
}

void Debris::finishAddToManager()
{
   AssertFatal(m_debrisData != NULL, "No data block assigned");

	debrisTime = (float) manager->getCurrentTime();

   lastUpdateTime = debrisTime;

	if (shapeName == NULL)
		shapeName = m_debrisData->pShapeName;

	if (debrisMass<0)
		debrisMass = m_debrisData->mass;
	if (spawnedExplosionId<0)
		spawnedExplosionId = m_debrisData->spawnedExplosionID;
   if (spawnImpulse<0)
      spawnImpulse = m_debrisData->spawnedDebrisStrength;
   if (explodeProb<0)
      explodeProb = m_debrisData->explodeOnBounce;
   if (elastic<0)
      elastic = m_debrisData->elasticity;
   
   explodeOnRest = m_debrisData->explodeOnRest != 0;

   damage = m_debrisData->damage;

	initShape();

	// add to container
	if (getContainer())
		inCloud = true; // already added to DebrisCloud container
	else
	{
		SimContainer *root = findObject(manager, SimRootContainerId,root);
		root->addObject(this);
		// add to timer Set, if inCloud don't have to
		addToSet(SimTimerSetId);
		inCloud=false;
	}

	// set movement stuff
	setCollisionMask(m_debrisData->collisionMask | m_debrisData->knockMask);
	setMass(debrisMass);
	setElasticity(elastic);
   setFriction(m_debrisData->friction);
	forceMask = -1; 

	// if we're a ghost, we don't need to do the rest...
	if (isGhost())
	{
		netFlags.clear(Ghostable);
		return;
	}

	// set timeout time
   if (duration<0)
   {
   	const float & minT = m_debrisData->minTimeout;
	   const float & maxT = m_debrisData->maxTimeout;
	   timeout = debrisTime + minT + (maxT - minT) * dbRand.getFloat();
   }
   else
      timeout = debrisTime + duration;

	if (ghostMe)
		netFlags.set(Ghostable);
	else
		netFlags.clear(Ghostable);
}

void Debris::initShape()
{
	// get shape
	ResourceManager *rm = SimResource::get(manager);
 	Resource<TSShape> rShape = rm->load(shapeName, true);
	AssertFatal((bool) rShape, avar("Unable to load debris shape \"%s\".", shapeName));

	// set-up render image
	image.shape = new TSShapeInstance(rShape,*rm);
	image.animated = m_debrisData->animationSequence!=-1 || animationSequence!=-1;
	image.itype = SimRenderImage::Normal;

	// animate? -- set reanimateTime
	if (image.animated && !manager->isServer())
	{
		image.shape->CreateThread();
		TSShapeInstance::Thread * t = image.shape->getThread(0);
		t->setTimeScale(1.0f);
		// if animationSequence is set, it over-rides value in info
		if (animationSequence!=-1)
			t->SetSequence(animationSequence);
		else
			t->SetSequence(m_debrisData->animationSequence);
		// if animRestartTime is set, it over-rides value in info
		if (animRestartTime>0)
			reanimateTime = debrisTime + animRestartTime;
		else if (m_debrisData->animRestartTime>0)
			reanimateTime = debrisTime + m_debrisData->animRestartTime;
		else
			reanimateTime = -1;
	}
	image.shape->animate();

	// set-up collision image
	float rad = image.shape->getShape().fRadius;
	collisionImage.radius  = rad;
	collisionImage.center  = image.shape->getShape().fCenter;
	collisionImage.center += image.shape->fRootDeltaTransform.p;

	// set box -- use shapes radius
	boundingBox.fMax = boundingBox.fMin = collisionImage.center;
	boundingBox.fMin.x -= rad; boundingBox.fMin.y -= rad; boundingBox.fMin.z -= rad;
	boundingBox.fMax.x += rad; boundingBox.fMax.y += rad; boundingBox.fMax.z += rad;

	// get box around "bounds" object
	// use for bouncing box and detailed collision
	// only use a "tight fit" if radius is small
   if (m_debrisData->collisionDetail != SphereCollisions)
   {
   	Box3F bounceBox;
	   TS::ObjectInstance * obj = image.shape->getNode(0)->fObjectList[0];
   	obj->getBox(image.shape,bounceBox);
	   radii  = bounceBox.fMax;
   	radii -= bounceBox.fMin;
	   radii *= .5f;
	   if (radii.x < .5f)
		   radii.x = .5f;
   	if (radii.y < .5f)
	   	radii.y = .5f;
	   if (radii.z < .5f)
		   radii.z = .5f;
   }

	// set world space bounding box -- SimMovement methods
	// handle it from now on
	Box3F box;
	buildContainerBox(getTransform(),&box);
	setBoundingBox(box);
}

bool Debris::onSimTimerEvent(const SimTimerEvent *event)
{
   if ( (hitThisTime && !hitLastTime && 
         dbRand.getFloat(1.0f) < explodeProb) ||
        damage < 0.0f  || (hitThisTime && hitLastTime))
      expire = true;
   hitLastTime = hitThisTime;
   hitThisTime = false;

	Point3F vel = getLinearVelocity ();
   if (isEqual (vel.x, 0) && isEqual (vel.y, 0) && explodeOnRest)
      expire = true;

	if (expire)
	{
      SimMessageEvent::post(this,-1);
      return true;
	}

	float curTime = manager->getCurrentTime();
   lastUpdateTime = curTime; // not used in timer update, but for render interpolation
	const SimTime & delta = event->timerInterval;

	// update animation whether ghost or not
	if (image.animated)
	{
		TSShapeInstance::Thread * thread = image.shape->getThread(0);
		if (reanimateTime>=0 && curTime>reanimateTime)
		{
			thread->SetSequence(0);
			if (animRestartTime>0)
				reanimateTime = curTime + animRestartTime;
			else
				reanimateTime = curTime + m_debrisData->animRestartTime;
		}
		else
			thread->AdvanceTime(delta);
	}

	setTimeSlice(delta);

   underSurfaceThisTime = false;

	TMat3F mat;
	updateMovement(&mat);

   // if we've built up an energy deficit (z-direction only)
   // then pay it back now if we can
   if (potentialEnergyDeficit > 0)
   {
      if (0.5f * lVelocity.z > potentialEnergyDeficit)
      {
         lVelocity.z -= potentialEnergyDeficit;
         potentialEnergyDeficit = 0;
      }
      else if (0.5f * lVelocity.z < -potentialEnergyDeficit)
      {
         lVelocity.z += potentialEnergyDeficit;
         potentialEnergyDeficit *= 0;
      }
      else
      {
         potentialEnergyDeficit -= 0.5f * fabs(lVelocity.z);
         lVelocity.z *= 0.5f;
      }
   }

	// use gridPolyStore for special collision check against terrain (for speed)
   bool putBackTerr = false;
   if (collisionMask & SimTerrainObjectType)
   {
      putBackTerr = true;
   	collisionMask &= ~SimTerrainObjectType;

		GridPolyStore::GridPoly * terrPoly = 
			gridPolyStore.getPoly(mat.p, gridPoly, manager);

		if (gridPoly>=0)
		{
			// deal with terrain collision here
			if (bounce(mat,terrPoly->normal,terrPoly->k))
			   hitThisTime = true;
		}
		else if (gridPoly==GridPolyStore::Overflow)
			// buffer full -- use standard collision system
			collisionMask |= SimTerrainObjectType;
	}
   
   if (timerCounter++ & NORMALIZE_MASK)
      mat.normalize();

	setPosition(mat);

   if (putBackTerr)
      collisionMask |= SimTerrainObjectType;

   // at rest?
   avgLV *= 0.9f;
   avgLV.x += 0.1f * lVelocity.x;
   avgLV.y += 0.1f * lVelocity.y;
   avgLV.z += 0.1f * lVelocity.z;
   avgLV.z *= 0.25f; // dampen this component
   avgAV *= 0.9f;
   avgAV.x += 0.1f * aVelocity.x;
   avgAV.y += 0.1f * aVelocity.y;
   avgAV.z += 0.1f * aVelocity.z;
   if (curTime>debrisTime+1.0f && // don't be at rest till 1 sec after birth
       hitThisTime &&             // don't be at rest unless we hit something
       m_dot(avgLV,avgLV)<0.1f && // average lVelocity small
       m_dot(avgAV,avgAV)<0.1f)   // average aVelocity small
   {
      lVelocity.set(0,0,0);
      setAngulerVelocity(Vector3F(0,0,0));
      flags.set(AtRest);
   }

	if (curTime>timeout)
		expire=true;

	if (ghostMe)
		setMaskBits(netUpdate);

   underSurfaceLastTime = underSurfaceThisTime;
	return true;
}

bool Debris::onSimMessageEvent(const SimMessageEvent *)
{
	float elapsed = (float) manager->getCurrentTime() - debrisTime;
	spawnTimeout -= elapsed;

	// create new debris cloud and explosion?
	if (spawnTimeout>0.0f)
	{
		if (m_debrisData->spawnedDebrisMask)
		{
			DebrisCloud * newCloud = new DebrisCloud(0.9f * debrisMass,
			                                         m_debrisData->spawnedDebrisMask,
	   		                                      getTransform().p,
	      		                                   m_debrisData->spawnedDebrisRadius,
	         		                                getLinearVelocity(),
	            		                             spawnImpulse,
	               		                          Point3F(0, 0, 0),
                                                  float(M_PI),                                                        Point3F(0, 0, 0),
	               		                          spawnTimeout);
			if (inCloud)
			{
				DebrisCloud * motherCloud = static_cast<DebrisCloud*>(this->getContainer());
				newCloud->setSpawnedFrom(motherCloud);
			}
			manager->addObject(newCloud);
		}

		if (spawnedExplosionId != -1)
		{
			Explosion * exp = new Explosion(spawnedExplosionId);
			exp->setPosition(getTransform().p);
			manager->addObject(exp);
		}
	}
	manager->deleteObject(this);

   return true;
}

void Debris::onRemove()
{
	// remove from whatever container we're in
	if (getContainer())
		getContainer()->removeObject(this);

	Parent::onRemove();
}

//--------------------------------------------------------------------------- 

// returns true if hit something
bool Debris::bounce(TMat3F & objectToWorld, Point3F & normal, float k)
{
   // sink into the ground a certain depth
   k -= sinkDepth;

	// get box center in world space
	Point3F boxCenter;
	m_mul(collisionImage.center,objectToWorld,&boxCenter);

	float penetration; // maximum extent of box penetration of plane
	float dist;        // distance of box center from plane

	dist        = fabs(k-m_dot(normal,boxCenter));
	if (dist>collisionImage.radius)
		return false; // no collision
      
   // if we're only doing sphere collisions, take care of that here
   if (m_debrisData->collisionDetail == SphereCollisions)
      return bounceSphere(normal);

	// get object coordinate basis
	Point3F v1,v2,v3;
	objectToWorld.getRow(0,&v1);
	objectToWorld.getRow(1,&v2);
	objectToWorld.getRow(2,&v3);

	penetration = fabs(m_dot(normal,v1)*radii.x) +
	              fabs(m_dot(normal,v2)*radii.y) +
	              fabs(m_dot(normal,v3)*radii.z) -
	              dist;
	if (penetration<0)
		return false; // no collision

   // if we're at rest, we're satisfied with box collision
   // no need to find point of contact or do detailed collision check
   // we were at rest, remain at rest
   if (flags.test(AtRest))
   {
      lVelocity.set(0,0,0);
      setAngulerVelocity(Vector3F(0,0,0));
      objectToWorld = getTransform();
      return true;
   }

   CollisionSurfaceList list;
   if (m_debrisData->collisionDetail == DetailedCollisions)
   {
      TMat3F toShape = objectToWorld; toShape.inverse();
      image.shape->collidePlane(0,normal,k,&toShape,&list,true);
      if (list.empty())
         return false;
   }
   else // just use bounding box
   {
      Point3F pushPoint;
		if (m_dot(v1,normal)<0)
			pushPoint.x = radii.x;
		else
			pushPoint.x = -radii.x;

		if (m_dot(v2,normal)<0)
			pushPoint.y = radii.y;
		else
			pushPoint.y = -radii.y;

		if (m_dot(v3,normal)<0)
			pushPoint.z = radii.z;
		else
			pushPoint.z = -radii.z;

      list.increment();
      CollisionSurface & cs = list.last();
      cs.position = pushPoint;
      cs.distance = penetration;
   }

   // we have contact, now stop the movement, bounce, and apply friction tangentially
   // also apply damage
   // friction stuff should be moved into SimMovement, but that's in flux right now,
   // so we'll temporarily keep it here

   float damageTaken = m_dot(lVelocity,normal) - m_debrisData->damageThreshold;
   if (damageTaken > 0.0f)
      damage -= damageTaken;

   float maxPenetration = -1.0f;
   float invNum = 1.0f / (float)list.size();
   for (int i=0; i<list.size(); i++)
   {
      CollisionSurface & cs = list[i];

      Point3F worldPushPoint;
      m_mul(cs.position,objectToWorld,&worldPushPoint);

      if (cs.distance>maxPenetration)
         maxPenetration = cs.distance;

      // bounce
      Parent::collide(worldPushPoint,normal);

      // now figure how much to push tangent to surface for friction
      worldPushPoint -= objectToWorld.p;
      Point3F pointVel;
      getVelocity(worldPushPoint,&pointVel);
      float pdot = m_dot(pointVel,normal);
      pointVel.x = pointVel.x - pdot * normal.x;
      pointVel.y = pointVel.y - pdot * normal.y;
      pointVel.z = pointVel.z - pdot * normal.z;
      pointVel *= -coefficient.friction * coefficient.mass * invNum;
      applyImpulse(worldPushPoint,pointVel);
   }

  	// move object to be on right side of plane
   // but only if plane is a bottom piece (i.e., the terrain or a floor of an interior)
   // and only if we were under the surface last time too
   // this last provision is so that current physics have a chance to do the job for us,
   // without going to our backup system.  If we do have to put the object above a surface,
   // we'll have added potential energy to the system.  So we keep track of our deficit
   // and "pay" it back when we can...
   if (maxPenetration>0.0f && normal.z > 0.5f)
   {
      if (timerCounter == 0)
         sinkDepth += maxPenetration;
      else if (underSurfaceLastTime)
      {
         Point3F putBack = normal;
  	      putBack *= maxPenetration;
         lPosition += putBack;
     	   objectToWorld.p += putBack;

         // compensate for potential energy added when putBack applied
         Point3F lv = getLinearVelocity();
         Point3F gravity;
         getForce(SimMovementGravityForce,&gravity);
         float lzSq = lv.z*lv.z + putBack.z * gravity.z;
         potentialEnergyDeficit += m_sqrt(fabs(lzSq));
      }
      underSurfaceThisTime = true;
   }

   return true;
}

bool Debris::bounceSphere(Point3F & normal)
{
   // take out velocity normal to the plane
   float dot = m_dot(normal,lVelocity);
   if (dot > 0)
      return false;

   Point3F lv = normal;
   lv *= dot;
   lVelocity -= lv;

   // now apply friction -- based on SimMovement code
   Point3F fv = lVelocity;
   float f = -dot * coefficient.friction;
   float len = fv.len();
   if (len > f)
      fv *= f / len;
   lVelocity -= fv;
   
   // add in velocity due to elasticity
   lv *= elastic;
   lVelocity -= lv;

   // now give it a different spin -- on average smaller than what it came in with
   float avgAVel = 1.5f * (fabs(aVelocity.x) + fabs(aVelocity.y) + fabs(aVelocity.z))/3.0f;
   aVelocity.x = dbRand.getFloat(-avgAVel,avgAVel);
   aVelocity.y = dbRand.getFloat(-avgAVel,avgAVel);
   aVelocity.z = dbRand.getFloat(-avgAVel,avgAVel);
   return true;
}

bool Debris::bounce(SimMovementInfo* minfo)
{
	bool hit = false;
	for (int j = 0; j < minfo->collisionList.size(); j++) 
	{
      SimCollisionInfo & ci = minfo->collisionList[j];
      if (ci.object->getType() & m_debrisData->knockMask)
      {
         // hit something (something hit us) that sends us reeling
         // use its velocity if it has one...
         SimMovement * sm = dynamic_cast<SimMovement*>(ci.object);
         if (sm)
         {
            // add an additional 25% to get separation
            const Point3F & addVel = sm->getLinearVelocity();
            float velDot = m_dot(addVel,lVelocity);
            if (velDot > 0.75f * m_dot(addVel,addVel))
            {
               // treat this case different -- we're already going
               // at least just an order of magnitude slower than
               // knocking object in the same direction
               // just set lVelocity and don't expire
               lVelocity.x = 1.25f * addVel.x;
               lVelocity.y = 1.25f * addVel.y;
               lVelocity.z = 1.25f * addVel.z;
               return false;
            }
            lVelocity.x = 1.25f * addVel.x;
            lVelocity.y = 1.25f * addVel.y;
            lVelocity.z = 1.25f * addVel.z;
         }
         expire = true;
         return false;
      }
		CollisionSurfaceList& surfaceList = ci.surfaces;
		for (int i = 0; i < surfaceList.size(); i++) 
		{
			CollisionSurface& surface = surfaceList[i];
			if (surface.distance < 0.0f)
				continue;

			// get normal (in world space)
			Point3F normal;
			m_mul(surface.normal,static_cast<RMat3F&>(surfaceList.tWorld),&normal);

			// put contact point into world space
			Point3F hitPoint;
			m_mul(surface.position,surfaceList.tWorld,&hitPoint);

			hit |= bounce(*minfo->target,normal,m_dot(normal,hitPoint));
		}
	}

	return hit;
}

bool Debris::processCollision(SimMovementInfo * moveInfo)
{
	hitThisTime |= bounce(moveInfo);

	return false; // never retry
}

//--------------------------------------------------------------------------- 
bool Debris::getImageList(const Point3F& pos,ImageList* list)
{
   if (!inCloud)
      return SimMovement::getImageList(pos,list);
      
   // we're in a debris cloud, we got a great short-cut
   SimContainerList & iList = static_cast<DebrisCloud*>(getContainer())->getIntersections();
   
   // we need our prospective building box
   TMat3F mat = getTransform();
   mat.p = pos;
   Box3F box;
   buildCollisionBox(mat,&box);

	// Query objects for their images
	SimCollisionImageQuery iquery;
	for (SimContainerList::iterator itr = iList.begin(); itr != iList.end(); itr++)
   {
      if (!(collisionMask&(*itr)->getType()) || !((*itr)->isIntersecting(box)))
         continue;
         
		if ((*itr)->processQuery(&iquery))
			for (int i = 0; i < iquery.count; i++)
				list->push_back(*itr,iquery.image[i]);
	}
	return !list->empty();
}

//--------------------------------------------------------------------------- 

bool Debris::processQuery( SimQuery *query )
{
	switch (query->type)
	{
		onQuery(SimRenderQueryImage);
		onQuery(SimCollisionImageQuery);
		default:
			return false;
	}
}

bool Debris::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   // Set our haze value...
   SimContainer* cnt = getContainer();
   if (cnt)
      image.hazeValue = cnt->getHazeValue(*query->renderContext,
                                          image.transform.p);
	image.transform = getTransform();

   // interpolate movement since last update
   float delta = manager->getCurrentTime() - lastUpdateTime;
   image.transform.p.x += delta * lVelocity.x;
   image.transform.p.y += delta * lVelocity.y;
   image.transform.p.z += delta * lVelocity.z;

   if (shapeDetailScale)
      image.shape->setDetailScale(*shapeDetailScale);

	query->count =1;
	query->image[0] = &image;
	return true;
}

bool Debris::onSimCollisionImageQuery(SimCollisionImageQuery * query)
{
	query->count=1;
	query->image[0]=&collisionImage;
	collisionImage.transform = getTransform();
	return true;
}

void Debris::createOnClients(Debris * debris, SimManager * mgr, float scopeDist)
{
   SimSet * packetStreams = static_cast<SimSet*>(mgr->findObject(PacketStreamSetId));
   if (packetStreams)
   {
      for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
      {
         Net::PacketStream * pStream = dynamic_cast<Net::PacketStream *>(*itr);
         if (!pStream)
            break;
         Net::GhostManager * gm = pStream->getGhostManager();
         if (gm->getCurrentMode() == Net::GhostManager::GhostNormalMode)
         {
            #ifdef debug
            SimMovement * scope = dynamic_cast<SimMovement*>(gm->getScopeObject());
            AssertFatal(scope,
               "DebrisCloud::createOnClients:  scope object not SimMovement...better change this then.");
            #else
            SimMovement * scope = static_cast<SimMovement*>(gm->getScopeObject());
            #endif
            Point3F offset = scope->getTransform().p - debris->getTransform().p;
            if (m_dot(offset,offset) > scopeDist * scopeDist)
               break;
            Net::RemoteCreateEvent * addEvent = new Net::RemoteCreateEvent;
            Debris *debris2 = new Debris(*debris);
            addEvent->newRemoteObject = debris2;
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }
      }
   }
   if (!debris->manager)
      // this was a temporary object
      delete debris;
}


//--------------------------------------------------------------------------- 
// DebrisCloud
//--------------------------------------------------------------------------- 

float DebrisCloud::detailScale = 1.0f;

void DebrisCloud::getAxes(const Point3F & axis, Point3F * axes, float & angleRange)
{
	Point3F & x = axes[0];
	Point3F & y = axes[1];
	Point3F & z = axes[2];

	// find axes to use for tossing ('axis' will be z)
	float len2 = m_dot(axis,axis);
	if (!IsEqual(len2,0.0f))
	{
		z = axis;
		if (!IsEqual(len2,1.0f))
			z *= m_invsqrtf(len2);
		if (IsEqual((float)fabs(z.z),1.0f))
		{
			// special case
			x.x=y.y=1.0f;
			x.y=x.z=y.x=y.z=0.0f;
		}
		else
		{
			x.x = z.y; x.y = - z.x; x.z = 0;
			// normalize x
			float tmp = 1.0f/(1.0f-z.z);
			x.x *= tmp; x.y *= tmp;
			m_cross(x,z,&y);
		}
		x.normalize(); y.normalize();
	}
	else
	{
		x.x=y.y=z.z=1.0f;
		x.y=x.z=y.x=y.z=z.x=z.y=0;
		angleRange = float(M_PI);
	}
}

void DebrisCloud::toss(Debris * debris, 
                       const Point3F & pos, float radius, 
                       const Point3F & vel, float outwardImpulse,
                       const Point3F * axes, float angleRange,
                       const Point3F & incomingImpulse, SimManager * mgr)
{
	const Point3F & x = axes[0];
	const Point3F & y = axes[1];
	const Point3F & z = axes[2];

	// first, find point on unit sphere (or cone if angleRange!=M_PI
	float rot1 = dbRand.getFloat(float(M_2PI));
	float rot2 = dbRand.getFloat(-angleRange,angleRange);

	Point3F vec;
	float s2 = sin(rot2);
	float c1s2 = cos(rot1) * s2;
	float s1s2 = sin(rot1) * s2;
	float c2 = cos(rot2);

	//	vec = x * c1 * s2 + y * s1 * s2 + z * c2
	vec.x = x.x * c1s2 + y.x * s1s2 + z.x * c2;
	vec.y = x.y * c1s2 + y.y * s1s2 + z.y * c2;
	vec.z = x.z * c1s2 + y.z * s1s2 + z.z * c2;

	Point3F loc = vec;
	loc *= radius;
	loc += pos;
   TMat3F tmat;
   (RMat3F&)tmat = randomRotations[dbRand.getInt(0,numRandomRotations)];
   tmat.p = loc;
	debris->setPosition(tmat,true);
	debris->setLinearVelocity(vel);

   // add some random anguler [sic] velocity using magic numbers
   float amag = outwardImpulse * 0.015f;
   if (amag>6.0f)
      amag = 6.0f;
   Point3F avel = Point3F(dbRand.getFloat(amag)-amag*0.5f,
                          dbRand.getFloat(amag)-amag*0.5f,
                          dbRand.getFloat(amag)-amag*0.5f);
	debris->setAngulerVelocity(avel);

	mgr->addObject(debris);

	vec *= outwardImpulse;
	debris->applyImpulse(vec);
	debris->applyImpulse(incomingImpulse);
}

void DebrisCloud::createOnClients(DebrisCloud * cloud, SimManager * mgr, float scopeDist)
{
   SimSet * packetStreams = static_cast<SimSet*>(mgr->findObject(PacketStreamSetId));
   if (packetStreams)
   {
      for (SimSet::iterator itr = packetStreams->begin(); itr != packetStreams->end(); itr++)
      {
         Net::PacketStream * pStream = dynamic_cast<Net::PacketStream *>(*itr);
         if (!pStream)
            break;
         Net::GhostManager * gm = pStream->getGhostManager();
         if (gm->getCurrentMode() == Net::GhostManager::GhostNormalMode)
         {
            #ifdef debug
            SimMovement * scope = dynamic_cast<SimMovement*>(gm->getScopeObject());
            AssertFatal(scope,
               "DebrisCloud::createOnClients:  scope object not SimMovement...better change this then.");
            #else
            SimMovement * scope = static_cast<SimMovement*>(gm->getScopeObject());
            #endif
            Point3F offset = scope->getTransform().p - cloud->pos;
            if (m_dot(offset,offset) > scopeDist * scopeDist)
               break;
            Net::RemoteCreateEvent * addEvent = new Net::RemoteCreateEvent;
            DebrisCloud *cloud2 = new DebrisCloud(*cloud);
            addEvent->newRemoteObject = cloud2;
            pStream->getEventManager()->postRemoteEvent(addEvent);
         }
      }
   }
   if (!cloud->manager)
      // this was a temporary object
      delete cloud;
}

DebrisCloud::DebrisCloud(const DebrisCloud &cloud)
{
	mass = cloud.mass;
	debrisMask = cloud.debrisMask;
	pos = cloud.pos;
	radius = cloud.radius;
	vel = cloud.vel;
	outwardImpulse = cloud.outwardImpulse;
	axis = cloud.axis;
	angleRange = cloud.angleRange;
	incomingImpulse = cloud.incomingImpulse;
	spawnTimeout = cloud.spawnTimeout;
	spawnedFrom = 0;
	soundId = cloud.soundId;
	dataReceived = true;

	netFlags.clear(Ghostable);
	installDatabase(&database);
}

DebrisCloud::DebrisCloud()
{ 
	netFlags.clear(Ghostable); 
	soundId = 0;
	spawnedFrom=0;

	dataReceived = false;
	installDatabase(&database);
}

DebrisCloud::DebrisCloud(float kg, Int32 typeMask,
                                          const Point3F &p, float r,
	                                       const Point3F &v, float outJ,
	                                       const Point3F & ax, float angleR,
	                                       const Point3F & inJ, float spawnTO)
{
	mass = kg;
	debrisMask = typeMask;
	pos = p;
	radius = r;
	vel = v;
	outwardImpulse = outJ;
	axis = ax;
	angleRange = angleR;
	incomingImpulse = inJ;
	spawnTimeout = spawnTO;
	soundId = 0;
	spawnedFrom = 0;
	dataReceived = true;

	netFlags.clear(Ghostable);
	installDatabase(&database);
}

DebrisCloud::~DebrisCloud()
{
   installDatabase(NULL);
}

void DebrisCloud::setSpawnedFrom(DebrisCloud * _sf)
{
	spawnedFrom = _sf;
}

bool DebrisCloud::processEvent(const SimEvent* event)
{
	switch (event->type) 
	{
		onEvent(SimTimerEvent);
      onEvent(SimMessageEvent);
	}
	return SimContainer::processEvent(event);
}

bool DebrisCloud::onAdd()
{
	AssertFatal(!IN_SERVER,"DebrisCloud:onSimAddNotifyEvent: attempt to make debris in server.");
	if (!Parent::onAdd())
		return false;
	netFlags.clear(Ghostable);

	if (dataReceived)
		finishAddToManager();
	return true;
}

void DebrisCloud::finishAddToManager()
{
	// if we are not a spawned debris cloud, add to root container
	if (!spawnedFrom)
	{
		// add ourself to the root and set our bounding box
		SimContainer *root = findObject(manager, SimRootContainerId,root);
		containerMask = 0; // so nothing gets in...
		root->addObject(this);
		setBoundingBox(Box3F(pos,pos));

   	// add to timer Set -- if we're a spawned cloud, we won't do this
	   addToSet(SimTimerSetId);
	}

	int i;
	float p; // sum of all probability densities

	mass *= detailScale;

	DebrisTable::ShortList sl;
	p = DebrisTable::getShortList(mass,debrisMask,sl);

	// find coordinate axes to use
	getAxes(axis,axes,angleRange);

	int numLeft = sl.size();
	while (numLeft)
	{
		float choose = dbRand.getFloat(0,p);
		i=numLeft-1;
		while (i>0 && choose>sl[i].p)
			choose -= sl[i--].p;
		Debris *debris = new Debris(sl[i].index, spawnTimeout);

      // temporarily set our position so that debris bounding box doesn't
      // get out of hand
      const_cast<TMat3F&>(debris->getTransform()).p = pos;

		// add debris to debris cloud we're spawned from or else add to ourself
		if (spawnedFrom)
		{
			spawnedFrom->database.update(debris, spawnedFrom);
         spawnedFrom->containerMask = -1;
			spawnedFrom->addObject(debris);
         spawnedFrom->containerMask = 0;
		}
		else
		{
         // we need to let the debris pieces in
			database.update(debris, this);
         containerMask = -1;
			addObject(debris);
         containerMask = 0;
		}

		toss(debris,pos,radius,vel,outwardImpulse,axes,angleRange,incomingImpulse,manager);
		mass -= sl[i].mass;
		while (numLeft && sl[numLeft-1].mass>mass)
			p -= sl[--numLeft].p;
	}
   containerMask = 0; // so nothing else gets in...

	if (soundId>0)
		// make sound
      Sfx::Manager::PlayAt( manager, soundId, TMat3F(EulerF(0, 0, 0),pos), Point3F(0, 0, 0));

   // nothing got added, delete ourself
   if (database.debrisList.empty())
      SimMessageEvent::post(this,-1);
}

bool DebrisCloud::onSimTimerEvent(const SimTimerEvent * event)
{
   // construct query in order to find intersections of debris cloud
   SimContainerQuery query;
   query.id = getId();
   query.type = -1;
   query.mask = database.unionCollisionMask;
   query.box = getBoundingBox();
   // expand the box a little to account for movement
   query.box.fMin -= DEBRIS_MAX_MOVE_PER_DT;
   query.box.fMax += DEBRIS_MAX_MOVE_PER_DT;
   query.detail = SimContainerQuery::DefaultDetail;
   // ok, find intersections now
   intersectionList.clear();
	SimContainer* root = findObject(manager,SimRootContainerId,root);
   root->findIntersections(query,&intersectionList);

   database.inTimer = true;

	// debris in a debris cloud is not added to timer set, but gets its updates from here
	Vector<Debris*>::iterator itr;
	for (itr=database.debrisList.begin(); itr!=database.debrisList.end(); itr++)
	{
      #ifdef DEBUG
      Debris *debris = dynamic_cast<Debris *>(*itr);
      AssertFatal(debris, "non-debris thing in debris cloud");
      #endif
      AssertFatal(!(*itr)->isRemoved(),"DebrisCloud::onSimTimerEvent:  object's been removed/deleted.");
  		(*itr)->onSimTimerEvent(event);
	}

   database.inTimer = false;
   setBoundingBox(getBoundingBox());   
   
	return true;
}

bool DebrisCloud::onSimMessageEvent(const SimMessageEvent *)
{
   AssertFatal(!isDeleted(),"DebrisCloud::onSimMessageEvent:  received 2 delete message events");
   deleteObject();
   return true;
}

//------------------------------------------------------------------------------ 
// Persistent IO stuff -- generic routines for storing floats and vectors small
//                        floats are stored as 1 byte; unit vectors as 1;
//                        vectors with length as 2 bytes (or 1 byte if 0 length)
//------------------------------------------------------------------------------ 

static void readUnitVectorSmall(StreamIO &s, Point3F &v)
{
	UInt8 vIndex;
	s.read( sizeof(vIndex), (void *) &vIndex);

	float angle1 = (vIndex % 16) * M_2PI/16;
	float angle2 = (vIndex >> 4) * M_PI/16 - 0.5f * M_PI;

	v.x = cos(angle1)*cos(angle2);
	v.y = sin(angle1)*cos(angle2);
	v.z = sin(angle2);
}

static void writeUnitVectorSmall(StreamIO &s, const Point3F &v)
{
	float angle1, angle2;
	if (!IsEqual(v.x,0.0f))
		angle1 = m_atan(v.x,v.y);
	else if (v.y>0.0f)
		angle1=float(M_PI)*.5f;
	else
		angle1=1.5f*float(M_PI);

	float L = m_sqrt(v.x*v.x+v.y*v.y);
	if (L!=0)
		angle2=m_atan(L,v.z);
	else if (v.z>0)
		angle2=  0.5f*float(M_PI);
	else
		angle2= -0.5f*float(M_PI);

	if (angle1<0)
		angle1 += float(M_2PI);
	angle2 += 0.5f * float(M_PI);

	int a2I = 16.0f * angle2 / float(M_PI);
	if (a2I>15)
		a2I=15;

	UInt8 vIndex = ((UInt8)(16.0f*angle1/float(M_2PI))) + ((UInt8)(a2I)<<4);

	s.write( sizeof(vIndex), (void *) &vIndex );
}

static void readVectorSmall(StreamIO &s, Point3F &v, float precision)
{
	UInt8 absV;
	s.read( sizeof(absV), (void *) &absV);
	if (absV==0)
		v.set(0, 0, 0);
	else
	{
		readUnitVectorSmall(s,v);
		v *= precision * (float) absV;
	}
}

static void writeVectorSmall(StreamIO &s, const Point3F &v, float precision)
{
	float absV = v.len();
	UInt8 mag;
	if (absV>255.0f * precision)
		mag=255;
	else
		mag=absV/precision;
	s.write( sizeof(mag), (void *) &mag );
	if (mag!=0)
	{
		Point3F tmpV=v;
		tmpV *= 1.0f/absV;
		writeUnitVectorSmall(s,tmpV);
	}
}

static void readFloatSmall(StreamIO &s, float & f, float precision)
{
	UInt8 num;
	s.read( sizeof(num), (void *) &num);
	f = precision * (float) num;
}

static void writeFloatSmall(StreamIO &s, float f, float precision)
{
	UInt8 num;
	if (f>precision*255)
		num = 255;
	else
		num = f/precision;
	s.write( sizeof(num), (void *) &num);
}

//--------------------------------------------------------------------------- 
// Standard Persistent IO stuff
//
//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT_TAG( Debris, DebrisPersTag );

void Debris::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm, stream);

	// read transform and velocity whether this is first packet or not
	TMat3F trans;
	stream->read( sizeof( trans ), (void *)&trans );
	setTransform(trans);
	//
	Point3F vel,av;
	stream->read( sizeof( vel ), (void *)&vel);
	stream->read( sizeof( av ), (void *)&av);
	setLinearVelocity(vel);
	setAngulerVelocity(av);

	// is this the initial packet (will be set if it is)
	// if it isn't, already got our update...
	if (!stream->readFlag())
		return;

   Parent::unpackDatFile(stream);
	stream->read(sizeof(spawnTimeout), (void*) &spawnTimeout);

   char buf[256];
   stream->readString(buf);
   shapeName = stringTable.insert(buf, true);

	if (stream->readFlag())
	{
		UInt8 massI;
		stream->read( sizeof(massI), (void*)&massI);
		debrisMass = massI * 50;
	}
	else
		debrisMass = -1;

	if (stream->readFlag())
		stream->readBits(4, (void*)&animationSequence);

	if (stream->readFlag())
	{
		UInt8 atimeI;
		stream->read( sizeof(atimeI), (void*)&atimeI);
		animRestartTime = ((float)atimeI)/10.0f;
	}

	ghostMe=false;

	if (manager)
		finishAddToManager();
}

DWORD Debris::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   DWORD parentPunt = Parent::packUpdate(gm, mask, stream);

	// write transform and velocity whether this is first packet or not
	stream->write( sizeof( TMat3F ), (void *)&getTransform() );
	stream->write( sizeof( getLinearVelocity() ), (void *)&getLinearVelocity() );
	stream->write( sizeof( getAngulerVelocity() ), (void *)&getAngulerVelocity() );

	// if this is the initial packet, set bit
	if (mask & netInit)
		stream->writeFlag(true);
	else
	{
		stream->writeFlag(false);
      return parentPunt;
	}

   Parent::packDatFile(stream);
	stream->write(sizeof(spawnTimeout), (void*) &spawnTimeout);

   stream->writeString(shapeName);

	if (debrisMass>0)
	{
		stream->writeFlag(true);
		UInt8 massI = debrisMass/50.0f;
		stream->write( sizeof(massI), (void*)&massI);
	}
	else
		stream->writeFlag(false);

	if (animationSequence>0)
	{
		stream->writeFlag(true);
		stream->writeBits(4, (void*)&animationSequence);
	}
	else
		stream->writeFlag(false);

	if (animRestartTime>0)
	{
		stream->writeFlag(true);
		UInt8 atimeI = animRestartTime * 10.0f;
		stream->write( sizeof(atimeI), (void*)&atimeI);
	}
	else
		stream->writeFlag(false);

   return parentPunt;
}


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 
// Standard Persistent IO stuff
//

IMPLEMENT_PERSISTENT_TAG( DebrisCloud, DebrisCloudPersTag );

void DebrisCloud::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
	readFloatSmall(*stream,mass,50.0f);
	stream->read( sizeof(debrisMask), (void *) &debrisMask);
	stream->read( sizeof(pos), (void *)&pos );
	readFloatSmall(*stream,radius,.5f);
	readVectorSmall(*stream,vel,10.0f);
	readFloatSmall(*stream,outwardImpulse,50.0f);

	UInt8 angleI;
	stream->read( sizeof(angleI), (void *) &angleI);
	if (angleI==255)
	{
		axis.set(0, 0, 0);
		angleRange = float(M_PI);
	}
	else
	{
		angleRange = .0125f * (float) angleI;
		readUnitVectorSmall(*stream,axis);
	}

	readVectorSmall(*stream,incomingImpulse,100.0f);
	readFloatSmall(*stream,spawnTimeout,1.0f);

	if (stream->readFlag())
		soundId = stream->readInt(20);

	dataReceived = true;
	if (manager)
		finishAddToManager();
}

DWORD DebrisCloud::packUpdate(Net::GhostManager *, DWORD, BitStream *stream)
{
	writeFloatSmall(*stream,mass,50.0f);
	stream->write( sizeof(debrisMask), (void *) &debrisMask);
	stream->write( sizeof(pos), (void *) &pos );
	writeFloatSmall(*stream,radius,.5f);
	writeVectorSmall(*stream,vel,0.1f);
	writeFloatSmall(*stream,outwardImpulse,50.0f);

	float aLen = m_dot(axis,axis);
	UInt8 angleI;
	if ( IsEqual(aLen,0.0f) || angleRange>M_PI-0.1f)
	{
		angleI=255;
		stream->write( sizeof(angleI), (void *) &angleI );
	}
	else
	{
		angleI = angleRange / .0125f; // puts 0..pi between 0..255
		stream->write( sizeof(angleI), (void *) &angleI );
		aLen = m_sqrt(aLen);
		axis *= 1.0f/aLen;
		writeUnitVectorSmall(*stream,axis);
	}

	writeVectorSmall(*stream,incomingImpulse,100.0f);
	writeFloatSmall(*stream,spawnTimeout,1.0f);
	if (stream->writeFlag(soundId>0))
		stream->writeInt(soundId,20);
   return 0;
}

//---------------------------------------------------------
// container methods for debris cloud
//---------------------------------------------------------

//------------- Internal Data base methods ----------------
bool setMinMax(Point3F & minVal, Point3F & maxVal, Point3F & newMin, Point3F & newMax)
{
   int noChange = 0;

   if (newMin.x < minVal.x)
      minVal.x = newMin.x;
   else
      noChange++;
      
   if (newMax.x > maxVal.x)
      maxVal.x = newMax.x;
   else
      noChange++;

   if (newMin.y < minVal.y)
      minVal.y = newMin.y;
   else
      noChange++;

   if (newMax.y > maxVal.y)
      maxVal.y = newMax.y;
   else
      noChange++;

   if (newMin.z < minVal.z)
      minVal.z = newMin.z;
   else
      noChange++;

   if (newMax.z > maxVal.z)
      maxVal.z = newMax.z;
   else
      noChange++;

   return noChange != 6;
}

void DebrisCloud::Database::update(SimContainer * object)
{
	SimContainer* cnt = object->getContainer();
	Box3F box = cnt->getBoundingBox();
	Box3F debrisBox = object->getBoundingBox();
   
   if (setMinMax(box.fMin,box.fMax,debrisBox.fMin,debrisBox.fMax))
   {
      if (!inTimer)
         // only update bounding box if this extended the current box
      	cnt->setBoundingBox(box);
      else
         // a little cheating to avoid endless container updating
         const_cast<Box3F&>(cnt->getBoundingBox()) = box;
   }      
}

void DebrisCloud::Database::update(SimContainer *object, SimContainer *cnt)
{
	Box3F box = cnt->getBoundingBox();
	Box3F debrisBox = object->getBoundingBox();

   if (setMinMax(box.fMin,box.fMax,debrisBox.fMin,debrisBox.fMax))
   {
      if (!inTimer)
         // only update bounding box if this extended the current box
      	cnt->setBoundingBox(box);
      else
         // a little cheating to avoid endless container updating
         const_cast<Box3F&>(cnt->getBoundingBox()) = box;
   }      
}

bool DebrisCloud::Database::add(SimContainer* object)
{
   #ifdef DEBUG
   Debris *debris = dynamic_cast<Debris*>(object);
   AssertFatal(debris,"DebrisCloud::dbAdd: not debris");
   #else
   Debris *debris = static_cast<Debris*>(object);
   #endif
	debris->setContainerData(debrisList.size());
	debrisList.push_back(debris);
   unionCollisionMask |= debris->getCollisionMask();
	update(object);
	return true;
}

bool DebrisCloud::Database::remove(SimContainer * object)
{
	int index = object->getContainerData();
	Debris ** indexed = &debrisList[index];
	*indexed = debrisList.last();
	(*indexed)->setContainerData(index);
	debrisList.decrement();

	// last one? delete ourself
	if (debrisList.empty())
      SimMessageEvent::post(object->getContainer(),-1);
	return true;
}

bool DebrisCloud::Database::findIntersections(const SimContainerQuery&,SimContainerList*)
{
	return false;
}

bool DebrisCloud::Database::findContained(const SimContainerQuery&,SimContainerList*)
{
	return false;
}

SimContainer* DebrisCloud::Database::findOpenContainer(const SimContainerQuery&)
{
	return 0;
}

bool DebrisCloud::Database::removeAll(SimContainer *)
{
	debrisList.clear();
	return true;
}

void DebrisCloud::Database::render(SimContainerRenderContext& rc)
{
	// Render debris
   const int size = debrisList.size();
	Vector<Debris*>::iterator itr;
	for (itr = debrisList.begin(); itr != debrisList.end(); itr++ )
		rc.render(*itr);

	AssertFatal(size == debrisList.size(),
	  "object changed containers during render cycle");
}

void DebrisCloud::Database::scope(SimContainerScopeContext&)
{
	// Only the object itself should be ghosted, in fact
	// we should probably never get here as this object
	// should only exists on a client.
}	

//------------- Data base access methods ------------------

bool DebrisCloud::updateObject(SimContainer * object)
{
	// once in, debris never gets out, but container bbox may change:
	database.update(object);
	return true;
}



//--------------------------------------------------------------------------- 
// DebrisTable
//--------------------------------------------------------------------------- 
float DebrisTable::getShortList(float mass, Int32 typeMask, ShortList & sl)
{
	sl.clear();

	float p = 0.0f;

	for (int i = 0;
	     i < wg->dbm->getDataGroupSize(DataBlockManager::DebrisDataType); i++) {
      Debris::DebrisData* pData =
         static_cast<Debris::DebrisData*>(wg->dbm->lookupDataBlock(i, DataBlockManager::DebrisDataType));

		if (pData->mass > mass)
			break;
		if ((pData->type & typeMask) != 0) {
			sl.increment();
			ShortListEntry & sle = sl.last();
			sle.mass  = pData->mass;
			sle.index = i;
			sle.p     = pData->p;
			p        += pData->p;
		}
	}
	return p;
}

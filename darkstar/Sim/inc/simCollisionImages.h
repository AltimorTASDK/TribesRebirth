
#ifndef _SIMCOLLISIONIMAGES_H_
#define _SIMCOLLISIONIMAGES_H_

#include <sim.h>
#include <simCollision.h>
#include <m_collision.h>
#include <itrCollision.h>

//----------------------------------------------------------------------------

class GridFile;
class ITRInstance;

class SimCollisionTerrImage: public SimCollisionImage
{
public:
	GridFile* gridFile;
	bool editor;  // Lets the editor LOS collide with holes
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
	SimCollisionTerrImage();
};


//----------------------------------------------------------------------------


class SimCollisionItrImage: public SimCollisionImage
{
public:
	ITRInstance* instance;
	ITRCollision collision;
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
	SimCollisionItrImage();
};


//----------------------------------------------------------------------------


class DLLAPI SimCollisionTS3Image: public SimCollisionImage
{
private:
	virtual bool checkSphere(SimCollisionSphereImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkBox(SimCollisionBoxImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkTube(SimCollisionTubeImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkLine(SimCollisionLineImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkTerrain(SimCollisionTerrImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkInterior(SimCollisionItrImage*, TMat3F *, CollisionSurfaceList*);
	virtual bool checkOtherShape(SimCollisionTS3Image*, TMat3F *, CollisionSurfaceList*);

public:
	enum { CollideBBox, CollideFaces } collisionLevel;
	TSShapeInstance * shapeInst;
	Box3F bbox;
	int collisionDetail;
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
	SimCollisionTS3Image();
};


//----------------------------------------------------------------------------

#endif
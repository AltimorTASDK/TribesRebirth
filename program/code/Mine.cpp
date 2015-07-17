//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4710)
#pragma warning(disable : 4101)
#endif

#include <console.h>
#include "mine.h"
#include "explosion.h"
#include "fearDcl.h"
#include "DataBlockManager.h"


IMPLEMENT_PERSISTENT_TAGS( Mine, FOURCC('M','I','N','E'), MinePersTag );

//----------------------------------------------------------------------------

static const float DeleteTimeout = 0.25;


//----------------------------------------------------------------------------

Mine::MineData::MineData()
{
	explosionRadius = 1.0;
	damageValue = 1.0;
	damageType = 0;
	kickBackStrength = 100;
}

void Mine::MineData::pack(BitStream *stream)
{
   Parent::pack(stream);
}

void Mine::MineData::unpack(BitStream *stream)
{
   Parent::unpack(stream);
}


//----------------------------------------------------------------------------

Mine::Mine(void)
{
	type = MineObjectType;

	// Item & static base class variables.
	isRotate = false;
	isCollideable = true;
	deleteOnDestroy = true;

	groundNormal.set (0, 0, 0);
	conformed = false;
}

Mine::~Mine()
{
}


//--------------------------------------------------------------------------- 

bool Mine::processArguments(int argc, const char **argv)
{
   if (argc != 1) {
     	Console->printf("Mine: <dataFile>");
      return false;
   }
	// Skip the Item argument processing
   if(!StaticBase::processArguments(1, argv))
      return false;

   return true;
}

bool Mine::initResources(GameBase::GameBaseData *data)
{
	if (!Parent::initResources(data))
		return false;

//   MineData* mineData = static_cast<MineData *>(getDatPtr());

	// hack for now... need bbox to terrain collision... hint, hint.
	collisionImage.sphere.radius /= 2;
	return true;
}

int Mine::getDatGroup()
{
   return DataBlockManager::MineDataType;
}


//-----------------------------------------------------------------------------------


bool Mine::processCollision(SimMovementInfo *info)
{
	for (int i = 0; i < info->collisionList.size(); i++)
		{
			if (info->collisionList[i].object->getType() & SimInteriorObjectType ||
				 info->collisionList[i].object->getType() & SimTerrainObjectType)
				{
					CollisionSurfaceList& sList = info->collisionList[i].surfaces;

					groundNormal.set(0, 0, 0);
					for (int j = 0; j < sList.size(); j++)
						{
							Point3F tNorm;
							m_mul(sList[j].normal, static_cast<RMat3F&>(sList.tWorld), &tNorm);
							tNorm.normalize();
							if (tNorm.z > 0)
								{
									groundNormal += tNorm;
									if (groundNormal.x || groundNormal.y || groundNormal.z)
										groundNormal.normalize();
								}
						}
				}
		}

	return Parent::processCollision (info);
}


//--------------------------------------------------------------------------- 

void Mine::onDamageStateChange(DamageState oldState)
{
	Parent::onDamageStateChange(oldState);
	if (getState() == Destroyed && !isGhost()) {
	   MineData* mineData = static_cast<MineData *>(getDatPtr());
		Explosion::applyRadiusDamage(manager,
			mineData->damageType,
			getBoxCenter(),Point3F(0,0,0),
			mineData->explosionRadius,
			mineData->damageValue,
			mineData->kickBackStrength,
			getDamageId());
	}
}

void Mine::onCollisionNotify(GameBase* object)
{
	if (!isGhost() && getState() != Destroyed)
		Parent::onCollisionNotify(object);
}

void Mine::sinkIn (float interval)
{
	float adjust = interval / 0.032;

	if (!conformed && (groundNormal.x || groundNormal.y || groundNormal.z))
		{
			Point3F oldRot;
			Point3F rot = getRot ();
			Point3F newRot;
			RMat3F mat;

			mat.set (EulerF (0, 0, -rot.z));

			m_mul (groundNormal, mat, &newRot);
			rot.x = m_atan (newRot.y, newRot.z) - (M_PI /  2);
			rot.y = -(m_atan (newRot.x, newRot.z) - (M_PI /  2));
			oldRot = getRot ();

			if (oldRot.x < rot.x)
				{
					oldRot.x += 0.05 * adjust;
					if (oldRot.x > rot.x)
						oldRot.x = rot.x;
				}
			else
				if (oldRot.x > rot.x)
					{
						oldRot.x -= 0.05 * adjust;
						if (oldRot.x < rot.x)
							oldRot.x = rot.x;
					}

			if (oldRot.y < rot.y)
				{
					oldRot.y += 0.05 * adjust;
					if (oldRot.y > rot.y)
						oldRot.y = rot.y;
				}
			else
				if (oldRot.y > rot.y)
					{
						oldRot.y -= 0.05 * adjust;
						if (oldRot.y < rot.y)
							oldRot.y = rot.y;
					}

			if (oldRot != getRot())
				{
					setRot (oldRot);
					setTransform (TMat3F (EulerF (oldRot.x, oldRot.y, oldRot.z), getPos()));
				}
			else
				conformed = true;

			if (!isGhost())
				setMaskBits(MineRotMask);
		}
}


void Mine::serverProcess (DWORD ct)
{
	Parent::serverProcess (ct);
	sinkIn (0.032);
}

void Mine::clientProcess (DWORD ct)
{
	Parent::clientProcess (ct);
	sinkIn (0.032);
}

//----------------------------------------------------------------------------

DWORD Mine::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
	if (isAtRest())
		mask = mask & ~RotationMask;

	Parent::packUpdate(gm, mask, stream);

//	if (stream->writeFlag(mask != 0xffffffff && mask & MineRotMask))
//		{
//			Point3F rot = getRot();
//
//			stream->writeFloat(rot.x / M_2PI,8);
//			stream->writeFloat(rot.y / M_2PI,8);
//			stream->writeFloat(rot.z / M_2PI,8);
//		}
//
   return 0;
}

void Mine::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
	Parent::unpackUpdate(gm,stream);

//	if (stream->readFlag())
//		setRot(Point3F(M_2PI * stream->readFloat(8), M_2PI * stream->readFloat(8), M_2PI * stream->readFloat(8)));
}



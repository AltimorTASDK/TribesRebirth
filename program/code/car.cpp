#include "car.h"
#include <sim.h>
#include <simResource.h>
#include <console.h>

#include <debris.h>
#include <partDebris.h>

#include "shapeBase.h"
#include "PlayerManager.h"

#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "stringTable.h"
#include "fearGlobals.h"
#include "sensorManager.h"
#include "fearPlayerPSC.h"
#include "dataBlockManager.h"
#include "explosion.h"
#include "tsfx.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "gwDeviceManager.h"

IMPLEMENT_PERSISTENT_TAGS( Car, FOURCC('C','A','R','!'), CarPersTag );

Car::Car()
{
   omega.set(0,0,0);
   velocity.set(0,0,0);
   forwardDir = 1;
   sideDir = 0;
   wheelRot = 0;
   accel = 0;
}

Car::CarData::CarData()
{
   wheelCount = 4;
   wheels[0].steerForward = true;
   wheels[1].steerForward = true;
   wheels[0].steerReverse = false;
   wheels[1].steerReverse = false;
   wheels[2].steerForward = false;
   wheels[3].steerForward = false;
   wheels[2].steerReverse = false;
   wheels[3].steerReverse = false;
   wheels[4].steerForward = false;
   wheels[5].steerForward = false;
   wheels[4].steerReverse = true;
   wheels[5].steerReverse = true;

   wheels[0].wheelBase.set(-3, 3, 1.5);
   wheels[1].wheelBase.set(3, 3, 1.5);
   wheels[2].wheelBase.set(-3, -3, 1.5);
   wheels[3].wheelBase.set(3, -3, 1.5);
   wheels[4].wheelBase.set(-3, -3, 0);
   wheels[5].wheelBase.set(3, -3, 0);
   wheelRadius = 1;
   wheelRestDist = 2.5;
   wheelOneGDist = 2.3;
   wheelSideBrakeForce = 100;
   turnAngle = M_PI / 16;
   wheelShapeName = "generator";
}

void Car::CarData::pack(BitStream* stream)
{
	Parent::pack(stream);
}	

void Car::CarData::unpack(BitStream* stream)
{
	Parent::unpack(stream);
}

int Car::getDatGroup()
{
   return DataBlockManager::CarDataType;
}

void Car::clientProcess (DWORD curTime)
{
   while(lpt < curTime)
   {
      DWORD endTick = (lpt + 8) & ~7;
      if(endTick > curTime)
         endTick = curTime;
      
      if(getControlClient() == int(manager->getId()))
         update(lpt & 7, ((endTick - 1) & 7) + 1, cg.psc->getClientMove(lpt));
      lpt = endTick;
   }
}

void Car::getVelocity(Point3F worldPos, Point3F *vel)
{
   worldPos -= getTransform().p;
   m_cross(omega, worldPos, vel);
   *vel += velocity;
}

void Car::update(DWORD start, DWORD end, PlayerMove *move)
{
   if(!move)
      return;

   float dt = (end - start) * 0.001;

   wheelRot -= move->turnRot * (end - start) / 32.0f;
   if(wheelRot > data->turnAngle)
      wheelRot = data->turnAngle;
   else if(wheelRot < -data->turnAngle)
      wheelRot = -data->turnAngle;

   m_sincos(wheelRot, &sideDir, &forwardDir);

   accel = move->forwardAction - move->backwardAction;

   Point3F sigmaF(0,0,-9.8);
   Point3F sigmaT(0,0,0);
   for(int i = 0; i < data->wheelCount; i++)
      processWheelSpring(i, &sigmaF, &sigmaT);

   // now we have sum of forces, sum of torques

   velocity += sigmaF * dt;
   omega += sigmaT * dt;

   sigmaF.set(0,0,0);
   sigmaT.set(0,0,0);
   for(int i = 0; i < data->wheelCount; i++)
      processWheelMove(i, dt, &sigmaF, &sigmaT);

   velocity += sigmaF * dt;
   omega += sigmaT * dt;

   rotate(omega * dt);
   TMat3F transform = getTransform();
   transform.p += velocity * dt;
   setTransform(transform);
}

void Car::processWheelSpring(int wheel, Point3F *sigmaF, Point3F *sigmaT)
{
   TMat3F transform = getTransform();

   // do the LOS to see if the wheel is contacting the ground:
   Point3F wheelBasePos;
   m_mul(data->wheels[wheel].wheelBase, transform, &wheelBasePos);

   Point3F wheelRestPos;
   transform.getRow(2, &wheelRestPos);
   wheelRestPos *= -( data->wheelRestDist + data->wheelRadius ) * 2;
   wheelRestPos += wheelBasePos;

   // look for the zed normal
   SimContainerQuery query;
   query.id = getId();
   query.type = -1;
   query.mask = SimInteriorObjectType | SimTerrainObjectType;
   query.box.fMax = wheelRestPos;
   query.box.fMin = wheelBasePos;
   
	SimCollisionInfo info;
	SimContainer *root = findObject(manager, SimRootContainerId, root);
   Point3F pos;

   if (root->findLOS(query, &info))
	{
	   if (info.surfaces.size()) {
		   m_mul(info.surfaces[0].position,info.surfaces.tWorld,&pos);
		   m_mul(info.surfaces[0].normal,(RMat3F&)info.surfaces.tWorld,&wheelNormal[wheel]);
	      wheelNormal[wheel].normalize();
	   }
	}
   // compute wheel distance:

   Point3F vec = wheelBasePos - pos;
   float wd = vec.len();
   wd -= wd * data->wheelRadius / m_dot(wheelNormal[wheel], vec);

   if(wd < 0)
      wd = 0;
   if(wd > data->wheelRestDist)
   {
      wheelDist[wheel] = data->wheelRestDist;
      return;
   }
   wheelDist[wheel] = wd;

   // apply spring force:
   // F = kx
   // k = F/x
   // deltaV = Ft
   // k = g/x
   // F = (g/xi) * x;
   // F = [g/(wheelRestDist - wheelOneGDist)] * (wheelRestDist - wd);
   Point3F springForce;
   transform.getRow(2, &springForce);
   float k = (9.8 / (data->wheelRestDist - data->wheelOneGDist)) / data->wheelCount;
   float x = (data->wheelRestDist - wd);

   Point3F vel;
   getVelocity(wheelBasePos, &vel);

   float v = m_dot(vel, springForce);
   float b = -sqrt(k);
   
   springForce *= k * x + v * b;
   Point3F transverseForce = wheelNormal[wheel];
   transverseForce *= m_dot(springForce, wheelNormal[wheel]);
   transverseForce = springForce - transverseForce;

   addAcceleration(wheelBasePos, springForce, sigmaF, sigmaT);
   //addAcceleration(wheelBasePos, transverseForce, sigmaF, sigmaT);
}

#define MaxBrakeForce 3
#define MaxAccel 2
bool Car::cameraZoomAndCrosshairsEnabled()
{
   return true;
}

void Car::getCameraTransform(float camDist, TMat3F *transform)
{
   *transform = getEyeTransform();
   validateEyePoint (transform, camDist * 2);
}

void Car::processWheelMove(int wheel, float dt, Point3F *sigmaF, Point3F *sigmaT)
{
   if(wheelDist[wheel] >= data->wheelRestDist)
      return;

   Point3F wheelBasePos;
   TMat3F transform = getTransform();
   m_mul(data->wheels[wheel].wheelBase, transform, &wheelBasePos);
   Point3F vel;
   getVelocity(wheelBasePos, &vel);

   Point3F sd;
   Point3F fd;
   Point3F up;

   transform.getRow(0, &sd);
   transform.getRow(1, &fd);
   transform.getRow(2, &up);

   if(data->wheels[wheel].steerForward)
   {
      sd *= sideDir;
      fd *= forwardDir;
      fd += sd;
      m_cross(fd, up, &sd);
   }
   else if(data->wheels[wheel].steerReverse)
   {
      sd *= sideDir;
      fd *= forwardDir;
      fd -= sd;
      m_cross(fd, up, &sd);
   }
   float sideDir = m_dot(vel, sd);

   if(sideDir < 0)
   {
      sd = -sd;
      sideDir = -sideDir;
   }
   float amt = sideDir / dt;
   if(amt > MaxBrakeForce)
      amt = MaxBrakeForce;

   addAcceleration(wheelBasePos, sd * -amt, sigmaF, sigmaT);
   addAcceleration(wheelBasePos, fd * accel * MaxAccel, sigmaF, sigmaT);
   return;

//   Point3F r = wheelBasePos - transform.p;
//   Point3F v;
//   m_cross(omega + *sigmaT * dt, r, &v);
//   v += velocity + *sigmaF * dt;
//
//   // which direction are we going?
//   Point3F forwardWheelDir, sideWheelDir;
//
//   if(data->wheels[wheel].steerForward)
//   {
//      Point3F sd;
//      transform.getRow(0, &sd);
//      transform.getRow(1, &fd);
//      sd *= sideDir;
//      fd *= forwardDir;
//      fd += sd;
//   }
//   else if(data->wheels[wheel].steerReverse)
//   {
//      Point3F sd;
//      transform.getRow(0, &sd);
//      transform.getRow(1, &fd);
//      sd *= sideDir;
//      fd *= forwardDir;
//      fd -= sd;
//   }
//   else
//      transform.getRow(1, &fd);
//   Point3F upv;
//   transform.getRow(2, &upv);
//
//   m_cross(fd, upv, &sideWheelDir);
//   sideWheelDir.normalize();
//   forwardWheelDir = fd;
//   // now we've got a forward wheel dir
//   // and a side wheel dir.
//
//   float brakeA = -m_dot(sideWheelDir, v) / dt;
//   addAcceleration(wheelBasePos, sideWheelDir * brakeA, sigmaF, sigmaT);
//
//   // if this were a realistic simulation, we could model
//   // traction based on spring force, surface type, etc.
//   // and probably will have to later.
//
//   // for now, just apply our acceleration in the forward
//   // direction and a braking force to the side...
//
//   //forwardWheelDir *= accel * dt;
//   //applyDampenDeltaV(wheelBasePos, forwardWheelDir, 9.8 * dt, dv, dw);
//   //applyDampenDeltaV(wheelBasePos, sideWheelDir, 9.8 * dt, dv, dw);
}

bool Car::onSimRenderQueryImage (SimRenderQueryImage* query)
{
   if(!Parent::onSimRenderQueryImage(query))
      return false;
   int i;
   for(i = 0; i < data->wheelCount; i++)
   {
      ShapeBase::RenderImage *iptr = wheelRenderImage + i;
      query->image[query->count++] = iptr;

      Point3F wheelPos = data->wheels[i].wheelBase;
      wheelPos.z -= wheelDist[i];
      m_mul(wheelPos, getTransform(), &iptr->transform.p);
      iptr->transform.flags = RMat3F::Matrix_HasRotation | RMat3F::Matrix_HasTranslation;

      Point3F fd;
      Point3F up;
      Point3F sd;

      if(data->wheels[i].steerForward)
      {
         getTransform().getRow(0, &sd);
         getTransform().getRow(1, &fd);
         sd *= sideDir;
         fd *= forwardDir;
         fd += sd;
      }         
      else if(data->wheels[i].steerReverse)
      {
         getTransform().getRow(0, &sd);
         getTransform().getRow(1, &fd);
         sd *= sideDir;
         fd *= forwardDir;
         fd -= sd;
      }
      else
         getTransform().getRow(1, &fd);

      getTransform().getRow(2, &up);
      m_cross(fd, up, &sd);

      iptr->transform.setRow(0, sd);
      iptr->transform.setRow(1, fd);
      iptr->transform.setRow(2, up);
      iptr->castShadow = image.castShadow;
      iptr->hazeValue = image.hazeValue;
      iptr->setImageType();
   }
   return true;
}

bool Car::processQuery(SimQuery* query)
{
	switch (query->type){
		onQuery (SimRenderQueryImage);
     default:
      return Parent::processQuery(query);
	}
}

bool Car::initResources(GameBase::GameBaseData* dat)
{
   lpt = lastProcessTime;
   if(!Parent::initResources(dat))
      return false;
	data = dynamic_cast<CarData *>(dat);
	Resource<TSShape> shape;
	char name[256];
	ResourceManager *rm = SimResource::get(manager);

	if (isGhost() && data->wheelShapeName) {
		strcpy(name, data->wheelShapeName);
		strcat(name, ".dts");
		shape = rm->load(name, true);
      if(bool(shape))
      {
         for(int i = 0; i < data->wheelCount; i++)
         {
            delete wheelRenderImage[i].shape;
            wheelRenderImage[i].shape = new TSShapeInstance(shape, *rm);
         }
      }
   }
   return true;
}

void Car::rotate(Point3F w)
{
	int i;
   TMat3F tmat = getTransform();
	float deltaTheta = w.len();
	if(deltaTheta == 0.0f)
		return; // no rotation.
	w *= 1 / deltaTheta; // normalize omega;
	float sinDeltaTheta, cosDeltaTheta;
	m_sincos(deltaTheta, &sinDeltaTheta, &cosDeltaTheta);
	for(i = 0; i < 3; i++)
	{
		Point3F r;
		tmat.getRow(i, &r);
		Point3F rpw, rorw, rporw;
		rpw = w;
		rpw *= m_dot(w, r);
		rorw = r;
		rorw -= rpw;
		m_cross(w, rorw, &rporw);
		rorw *= cosDeltaTheta;
		rporw *= sinDeltaTheta;
		rpw += rorw;
		rpw += rporw;
		tmat.setRow(i, rpw);
	}
   tmat.normalize();
   setTransform(tmat);
}

void Car::addAcceleration(Point3F r, Point3F &F, Point3F *sigmaF, Point3F *sigmaT)
{
   TMat3F tmat = getTransform();
	r -= tmat.p; // get the actual radius.
   
   *sigmaF += F;
   Point3F torque;
   m_cross(r, F, &torque);
   *sigmaT += torque;
}


/*	float rlen = r.len();
	if(rlen == 0.0f)
	{
		*dV += deltaV;
		return;
	}
	float invRlen = 1 / rlen;
	r *= invRlen; // normalize the radius.

	Point3F velAlongR = r;
	velAlongR *= m_dot(r, deltaV);
	*dV += velAlongR; // applies directly to the velocity
	deltaV -= velAlongR;
	// deltaV is now an omega delta;
	Point3F deltaOmega;
   Point3F rv;
   m_cross(*dw, r, &rv);
   Point3F xx = deltaV;

   xx.normalize();
   float velAdd = m_dot(rv, xx);

   if(velAdd < 0)
   {
      float dvl = deltaV.len();
      velAdd = -velAdd;
      if(dvl < velAdd)
         velAdd = dvl;
      xx *= velAdd * 2;
      *dV += velAdd;
   }

	m_cross(r, deltaV, &deltaOmega);
	deltaOmega *= invRlen; // deltaOmega = ||r|| x deltaV / |r|


	*dw += deltaOmega;
}*/

void Car::applyDampenDeltaV(Point3F r, Point3F &dir, float /*max*/, Point3F* /*dV*/, Point3F* /*dw*/)
{
   TMat3F tmat = getTransform();
	r -= tmat.p; // get the actual radius.

   Point3F deltaV;
   m_cross(omega, r, &deltaV);
   deltaV += velocity;

   dir *= -m_dot(dir, deltaV);
   r += tmat.p;
   //applyDeltaV(r, dir, dV, dw);
}

DWORD Car::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   stream->writeFlag(mask & PositionMask);
	Parent::packUpdate(gm,mask,stream);

	if (stream->writeFlag(mask & InfoMask))
		packDatFile(stream);
	return 0;
}

void Car::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   if(stream->readFlag())
   {
      velocity.set(0,0,0);
      omega.set(0,0,0);
   }
	Parent::unpackUpdate(gm,stream);

	if (stream->readFlag())
		unpackDatFile(stream);
}	


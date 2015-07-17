//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "fearPlayerPSC.h"
#include "fearGlobals.h"
#include "projTargetLaser.h"
#include "dataBlockManager.h"
#include "PlayerManager.h"
#include "player.h"
#include "console.h"

IMPLEMENT_PERSISTENT_TAG(TargetLaser, TargetLaserPersTag);

//-------------------------------------- LaserProjectile
const UInt32 TargetLaser::csm_updateFrequencyMS = 100;
const UInt32 TargetLaser::csm_targetFrequencyMS = 500;

TargetLaser::TargetLaser(const Int32 in_datFileId)
 : LaserProjectile(in_datFileId),
   m_waitingForShutoff(false),
   m_waitingToTarget(true)
{
   //
}

TargetLaser::TargetLaser()
 : LaserProjectile(-1),
   m_waitingForShutoff(false),
   m_waitingToTarget(true)
{
   //
}

TargetLaser::~TargetLaser()
{

}

int
TargetLaser::getDatGroup()
{
   return DataBlockManager::TargetLaserDataType;
}

void
TargetLaser::clientProcess(DWORD)
{
   if (wg->currentTime >= m_interpTimeStart + csm_updateFrequencyMS) {
      m_endPoint = m_interpolateTo;
   } else {
      m_endPoint = m_interpolateFrom + ((m_interpolateTo - m_interpolateFrom) *
                                        (float(wg->currentTime - m_interpTimeStart) /
                                         float(csm_updateFrequencyMS)));
   }
}

void
TargetLaser::serverProcess(DWORD in_currTime)
{
   if (m_waitingToGhost == true &&
       in_currTime > m_lastGhosted + csm_updateFrequencyMS) {
      m_lastGhosted = wg->currentTime;
      setMaskBits(TLPosUpdateMask);
   }

   if (m_waitingForShutoff == true) {
      deleteObject();
   }
}

bool
TargetLaser::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);

     default:
      return Parent::processQuery(query);
   }
}

void
TargetLaser::shutOffProjectile()
{
   AssertFatal(isGhost() == false, "Error, cannot shut off the client side...");

   m_waitingForShutoff = true;
}

bool
TargetLaser::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   // We render through the render group rather than the container system...
   //
   if (query->containerRenderQuery == true || m_overrideRender == true) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   if (readjustEnd() == false) {
      query->count    = 0;
      query->image[0] = NULL;
      return true;
   }

   query->count    = 1;
   query->image[0] = &m_beamRenderImage;

   m_beamRenderImage.m_startPoint = m_startPoint;
   m_beamRenderImage.m_endPoint   = m_endPoint;

   // Face the camera!
   TSRenderContext& rc = *query->renderContext;
   Point3F vec1 = rc.getCamera()->getTCW().p;
   vec1 -= m_beamRenderImage.m_startPoint;
   m_cross(vec1, m_directionVector, &m_beamRenderImage.m_perpVec);
   m_beamRenderImage.m_perpVec.normalize();

   extern float csg_beamRadius;
   float invFactor = rc.getCamera()->transformProjectRadius(m_endPoint, csg_beamRadius);
   if (invFactor < 1.0f)
      m_beamRenderImage.m_endFactor = 1.0f / invFactor;
   else
      m_beamRenderImage.m_endFactor = 1.0f;
   
   invFactor = rc.getCamera()->transformProjectRadius(m_startPoint, csg_beamRadius);
   if (invFactor < 1.0f)
      m_beamRenderImage.m_startFactor = 1.0f / invFactor;
   else
      m_beamRenderImage.m_startFactor = 1.0f;

   m_beamRenderImage.m_time    = 0.75f;
   m_beamRenderImage.m_pBitmap = m_resLaserBitmap;

   return true;
}

void
TargetLaser::updateImageTransform(const TMat3F& in_rTrans)
{
   m_startPoint = in_rTrans.p;
   in_rTrans.getRow(1, &m_directionVector);
   
   // find the first point of contact along this line, out to 4000 meters...
   //
   Point3F newEndPoint  = m_startPoint + (m_directionVector * 4000.0f);
   bool    newHitTarget;

   SimCollisionInfo  collisionInfo;
   SimContainerQuery collisionQuery;

   collisionQuery.id       = m_pShooter != NULL ? m_pShooter->getId() : 0;
   collisionQuery.type     = -1;
   collisionQuery.mask     = csm_collisionMask;
   collisionQuery.detail   = SimContainerQuery::DefaultDetail;
   collisionQuery.box.fMin = m_startPoint;
   collisionQuery.box.fMax = newEndPoint;

   AssertFatal(manager != NULL, "Feh.");
   SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
   AssertFatal(pRoot != NULL, "No root container?");

   if (pRoot->findLOS(collisionQuery, &collisionInfo)) {
      // Translate collision to world coordinates...
      m_mul(collisionInfo.surfaces[0].position, collisionInfo.surfaces.tWorld, &newEndPoint);
      newHitTarget = true;
   } else {
      newHitTarget = false;
   }

   if ((m_endPoint - newEndPoint).len() > 0.25f || newHitTarget != m_hitTarget) {
      m_endPoint  = newEndPoint;
      m_hitTarget = newHitTarget;
      m_waitingToTarget = true;
      m_waitingToGhost  = true;
   }
}

bool
TargetLaser::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   // if (isGhost() == true)   Need for AI targetting.
      addToSet(TargetableSetId);
   m_lastGhosted = wg->currentTime;

   m_interpolateTo  = m_endPoint;
   m_lastTargetSent = wg->currentTime - csm_targetFrequencyMS;
   return true;
}


void
TargetLaser::writeInitialPacket(Net::GhostManager* gm,
                                BitStream*         pStream)
{
   Projectile::writeInitialPacket(gm, pStream);

   pStream->write(sizeof(Point3F), &m_startPoint);
   pStream->write(sizeof(Point3F), &m_endPoint);

   Player* pPlayer = NULL;
   if (m_pShooter != NULL) 
      pPlayer = dynamic_cast<Player*>(m_pShooter);
   AssertFatal(pPlayer != NULL, "Error, owner of a targeting laser must be a player");

   int playerId = pPlayer->getOwnerClient();
   pStream->writeInt(playerId - 2048, 10);

   m_teamId = pPlayer->getTeam();
   pStream->writeInt(m_teamId, 3);

   pStream->writeFlag(m_hitTarget);
}

void
TargetLaser::readInitialPacket(Net::GhostManager* gm,
                               BitStream*         pStream)
{
   Projectile::readInitialPacket(gm, pStream);

   pStream->read(sizeof(Point3F), &m_startPoint);
   pStream->read(sizeof(Point3F), &m_endPoint);
   m_playerId = pStream->readInt(10) + 2048;

   m_interpolateFrom = m_endPoint;
   m_interpolateTo   = m_endPoint;
   m_interpTimeStart = wg->currentTime - csm_updateFrequencyMS;

   m_startIntensity = 1.0f;
   m_directionVector = (m_endPoint - m_startPoint).normalize();

   m_teamId = pStream->readInt(3);

   m_hitTarget = pStream->readFlag();
}

DWORD
TargetLaser::packUpdate(Net::GhostManager* gm,
                        DWORD              mask,
                        BitStream*         pStream)
{
   // We care only about one bit, position, since the velocity and rotation never
   //  change.  Send over velocity only on the initialPacket.  Note that the
   //  Parent::packUpdate() call skips entirely over the Projectile, which
   //  set responsibility for all ghosting on the derived class...
   //
   DWORD parentPunt = Projectile::packUpdate(gm, mask, pStream);
   m_waitingToGhost = false;

   if (mask & Projectile::InitialUpdate) {
      pStream->writeFlag(true);
      writeInitialPacket(gm, pStream);
   } else {
      pStream->writeFlag(false);
      pStream->write(sizeof(Point3F), &m_startPoint);
      pStream->write(sizeof(Point3F), &m_endPoint);

      pStream->writeFlag(m_hitTarget);
   }
   
   return parentPunt;
}

void
TargetLaser::unpackUpdate(Net::GhostManager* gm,
                          BitStream*         pStream)
{
   Projectile::unpackUpdate(gm, pStream);
   
   if (pStream->readFlag() == true) {
      // Is initial packet...
      //
      readInitialPacket(gm, pStream);
   } else {
      pStream->read(sizeof(Point3F), &m_startPoint);

      m_interpolateFrom = m_endPoint;
      pStream->read(sizeof(Point3F), &m_interpolateTo);
      m_interpTimeStart = wg->currentTime;

      m_hitTarget = pStream->readFlag();
   }
}

bool
TargetLaser::getTarget(Point3F* out_pTarget, int* out_pTeam)
{
   if (m_hitTarget == true) {
      *out_pTarget = m_endPoint;
      *out_pTeam   = m_teamId;

      return true;
   } else {
      return false;
   }
}

bool
TargetLaser::isSustained() const
{
   return true;
}

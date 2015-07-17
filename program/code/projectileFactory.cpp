//------------------------------------------------------------------------------
// Description: factory method for creating projectiles...
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------

#include "projectile.h"
#include "projBullet.h"
#include "projGrenade.h"
#include "projRocketDumb.h"
#include "projLaser.h"
#include "projTargetLaser.h"
#include "projSeekMissile.h"
#include "projLightning.h"
#include "projRepair.h"
#include "dataBlockManager.h"
#include "projectileFactory.h"

Projectile* createProjectile(ProjectileDataType& projectile)
{
   Projectile* pRet = NULL;

   switch (projectile.type) {
     case DataBlockManager::BulletDataType:
      pRet = new Bullet(projectile.dataType);
      break;

     case DataBlockManager::GrenadeDataType:
      pRet = new Grenade(projectile.dataType);
      break;

     case DataBlockManager::RocketDataType:
      pRet = new RocketDumb(projectile.dataType);
      break;

     case DataBlockManager::LaserDataType:
      pRet = new LaserProjectile(projectile.dataType);
      break;

     case DataBlockManager::TargetLaserDataType:
      pRet = new TargetLaser(projectile.dataType);
      break;

     case DataBlockManager::SeekingMissileDataType:
      pRet = new SeekingMissile(projectile.dataType);
      break;

     case DataBlockManager::LightningDataType:
      pRet = new Lightning(projectile.dataType);
      break;

     case DataBlockManager::RepairEffectDataType:
      pRet = new RepairEffect(projectile.dataType);
      break;

     default:
      AssertFatal(0, "invalid or misunderstood projectile type...");
   }

   return pRet;
}


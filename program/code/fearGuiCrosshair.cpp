#include "fearGuiHudCtrl.h"
#include "PlayerManager.h"
#include "Player.h"
#include "fearDcl.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "simContainer.h"
#include "simMovement.h"
#include "FearPlayerPSC.h"
#include "FearCam.h"
#include "fearGlobals.h"
#include "projectile.h"
#include "dataBlockManager.h"
#include "fearGuiShellPal.h"

#include <simGuiTSCtrl.h>
#include "simResource.h"
#include <g_font.h>
#include "console.h"

bool gui_ignoreTargetsPref = false;
extern DWORD gPlayerZoomChangeTime;

namespace FearGui
{


class Crosshair : public HudCtrl
{
private:
   typedef HudCtrl Parent;

   Resource<GFXFont> textFont;
   Resource<GFXFont> lrTextFont;
   
   Resource<GFXBitmap> mBitmap;
   Resource<GFXBitmap> mFriendBMP;
   Resource<GFXBitmap> mFoeBMP;
   
   Resource<GFXBitmap> lrBitmap;
   Resource<GFXBitmap> lrFriendBMP;
   Resource<GFXBitmap> lrFoeBMP;
   
   RectI zoomRect;
   bool zoomReticle;
   DWORD zoomDisplayTime;
   
   bool mbLowRes;

public:
   Crosshair();
   ~Crosshair();

   DECLARE_PERSISTENT(Crosshair);
   
   void onRightMouseDown(const SimGui::Event &event);
   void onRightMouseDragged(const SimGui::Event &event);
   void onRightMouseUp(const SimGui::Event &event);

   bool onAdd(void);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   const char* AcquireClosestTarget(GFXSurface*  sfc, float &targetDamage);
   void renderTargets(GFXSurface* sfc);
};

IMPLEMENT_PERSISTENT_TAG(Crosshair, FOURCC('F','G','c','h'));

Crosshair::Crosshair()
{
}

Crosshair::~Crosshair()
{
}

void Crosshair::onRightMouseDown(const SimGui::Event &event)
{
   //do nothing
   event;
}

void Crosshair::onRightMouseDragged(const SimGui::Event &event)
{
   //do nothing
   event;
}

void Crosshair::onRightMouseUp(const SimGui::Event &event)
{
   //do nothing
   event;
}

bool Crosshair::onAdd(void)
{
   if(!Parent::onAdd())
      return false;

   //load the font
   textFont = SimResource::get(manager)->load("sf_white_7.pft");
   lrTextFont = SimResource::get(manager)->load("sf_white_5.pft");

   //load the bitmaps
   mBitmap = SimResource::get(manager)->load("H_Reticle.bmp");
   AssertFatal(mBitmap, "Unable to load bitmap");
   mBitmap->attribute |= BMA_TRANSPARENT;
   
   mFriendBMP = SimResource::get(manager)->load("H_Friend.bmp");
   AssertFatal(mFriendBMP, "Unable to load H_Friend.bmp");
   mFriendBMP->attribute |= BMA_TRANSPARENT;
   
   mFoeBMP = SimResource::get(manager)->load("H_Foe.bmp");
   AssertFatal(mFoeBMP, "Unable to load H_Foe.bmp");
   mFoeBMP->attribute |= BMA_TRANSPARENT;
   
   //low res
   lrBitmap = SimResource::get(manager)->load("LR_H_Reticle.bmp");
   AssertFatal(lrBitmap, "Unable to load bitmap");
   lrBitmap->attribute |= BMA_TRANSPARENT;
   
   lrFriendBMP = SimResource::get(manager)->load("LR_H_Friend.bmp");
   AssertFatal(lrFriendBMP, "Unable to load LR_H_Friend.bmp");
   lrFriendBMP->attribute |= BMA_TRANSPARENT;
   
   lrFoeBMP = SimResource::get(manager)->load("LR_H_Foe.bmp");
   AssertFatal(lrFoeBMP, "Unable to load LR_H_Foe.bmp");
   lrFoeBMP->attribute |= BMA_TRANSPARENT;
   
   //define the extent - the crosshair doesn't get any events
   extent.x = extent.y = 1;
   
   mbLowRes = FALSE;
   
   zoomReticle = FALSE;
   zoomDisplayTime = 0;
   
   return true;
}

//defined in FearCam.cpp   
const float DefaultNearPlane = 0.1f;

// Zoom time change from fearplayerpsc.cpp

void Crosshair::onRender(GFXSurface*  sfc, Point2I, const Box2I&)
{
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      mbLowRes = TRUE;
   }
   else
   {
      mbLowRes = FALSE;
   }
   
   //set the clip rect to the size of the parent window
   Point2I clipTopLeft = parent->localToGlobalCoord(parent->position);
   sfc->setClipRect(&RectI(clipTopLeft.x, clipTopLeft.y,
                         clipTopLeft.x + parent->extent.x - 1, clipTopLeft.y + parent->extent.y - 1));

   //if the parent window changes size, update the position
   position.x = parent->position.x;
   position.y = parent->position.y;
   
   float fov, sniperFov, targetFov;
   DWORD startTime, zoomTime;
   DWORD curTime = cg.currentTime;
   
   if(!cg.psc || !cg.psc->isFirstPerson())
      return;
   GameBase *controlObject = cg.psc->getControlObject();
   if(!controlObject || !controlObject->cameraZoomAndCrosshairsEnabled())
      return;

   cg.psc->getFovVars(&fov, &sniperFov, &targetFov, &startTime, &zoomTime);
   
   int zoomRetWidth, zoomRetHeight;
   if (zoomTime > curTime) 
   {
      if (targetFov < fov)
      {
         if (zoomReticle || (targetFov <= 30)) zoomReticle = TRUE;
         else zoomReticle = FALSE;
      }
      
      if (zoomReticle)
      {
         if (targetFov < fov)
         {
            zoomRetWidth = (parent->extent.x / 2) * (curTime - startTime) / float(zoomTime - startTime);
            zoomRetHeight = (parent->extent.y / 2) * (curTime - startTime) / float(zoomTime - startTime);
         }
         else
         {
            zoomRetWidth = (parent->extent.x / 2) * (zoomTime - curTime) / float(zoomTime - startTime);
            zoomRetHeight = (parent->extent.y / 2) * (zoomTime - curTime) / float(zoomTime - startTime);
         }
      }
   }
   else if (fov <= 30)
   {
      zoomReticle = TRUE;
      zoomRetWidth = parent->extent.x / 2;
      zoomRetHeight = parent->extent.y / 2;
   }
   else zoomReticle = FALSE;
   
   Point2I center(parent->extent.x / 2, parent->extent.y / 2);
   int retWidth, retHeight;
   if (! zoomReticle)
   {
      //see if we should draw the zoomRect
      zoomDisplayTime = gPlayerZoomChangeTime + 2000;
      if (targetFov != fov)
      {
         zoomDisplayTime = 0;
      }
      
      if (zoomDisplayTime > curTime)
      {
         float hWidth = tan(sniperFov * M_PI / 360.0f) * parent->extent.x / 2;
         float hHeight = tan(sniperFov * M_PI / 360.0f) * parent->extent.y / 2;

      	zoomRect.upperL.set(center.x - hWidth, center.y - hHeight);
      	zoomRect.lowerR.set(center.x + hWidth, center.y + hHeight);

         int rectLength = (! mbLowRes ? 7 : 4);
         int color = GREEN;
         Point2I *upperL = &zoomRect.upperL;
         Point2I *lowerR = &zoomRect.lowerR;
         
         sfc->drawLine2d(upperL, &Point2I(upperL->x + rectLength, upperL->y), color);
         sfc->drawLine2d(upperL, &Point2I(upperL->x, upperL->y + rectLength), color);
         
         sfc->drawLine2d(&Point2I(lowerR->x, upperL->y), &Point2I(lowerR->x - rectLength, upperL->y), color);
         sfc->drawLine2d(&Point2I(lowerR->x, upperL->y), &Point2I(lowerR->x, upperL->y + rectLength), color);
         
         sfc->drawLine2d(&Point2I(upperL->x, lowerR->y), &Point2I(upperL->x + rectLength, lowerR->y), color);
         sfc->drawLine2d(&Point2I(upperL->x, lowerR->y), &Point2I(upperL->x, lowerR->y - rectLength), color);
         
         sfc->drawLine2d(lowerR, &Point2I(lowerR->x - rectLength, lowerR->y), color);
         sfc->drawLine2d(lowerR, &Point2I(lowerR->x, lowerR->y - rectLength), color);
         
         char buf[32];
         GFXFont *font = (! mbLowRes ? textFont : lrTextFont);
         Point2I zoomText;
         sprintf(buf, "ZOOM: %dx", parent->extent.x / (lowerR->x - upperL->x));
         zoomText.x = center.x - font->getStrWidth(buf) / 2;
         zoomText.y = center.y + (! mbLowRes ? 30 : 18);
         sfc->drawText_p(font, &zoomText, buf);
      }
   
      //draw in the lines
      GFXBitmap *bmp = (! mbLowRes ? mBitmap : lrBitmap);
      sfc->drawBitmap2d(bmp, &Point2I(center.x - bmp->getWidth() / 2,
                                      center.y - bmp->getHeight() / 2));
   }
   else
   {
      //check the console var
      if (! stricmp(Console->getVariable("pref::SniperCrosshair"), "FALSE"))
      {
         zoomReticle = FALSE;
      }
      
      if (zoomReticle)
      {
         //draw the sniper crosshair
         int color = GREEN;
         sfc->drawLine2d(&Point2I(center.x, center.y - zoomRetHeight),
                           &Point2I(center.x, center.y + zoomRetHeight),
                           color);
         sfc->drawLine2d(&Point2I(center.x - 1, center.y - zoomRetHeight),
                           &Point2I(center.x - 1, center.y - zoomRetHeight / 3),
                           color);
         sfc->drawLine2d(&Point2I(center.x - 1, center.y + zoomRetHeight / 3),
                           &Point2I(center.x - 1, center.y + zoomRetHeight),
                           color);
         sfc->drawLine2d(&Point2I(center.x + 1, center.y - zoomRetHeight),
                           &Point2I(center.x + 1, center.y - zoomRetHeight / 3),
                           color);
         sfc->drawLine2d(&Point2I(center.x + 1, center.y + zoomRetHeight / 3),
                           &Point2I(center.x + 1, center.y + zoomRetHeight),
                           color);
                           
         sfc->drawLine2d(&Point2I(center.x - zoomRetWidth, center.y),
                           &Point2I(center.x + zoomRetWidth, center.y),
                           color);
         sfc->drawLine2d(&Point2I(center.x - zoomRetWidth, center.y - 1),
                           &Point2I(center.x - zoomRetWidth / 3, center.y - 1),
                           color);
         sfc->drawLine2d(&Point2I(center.x + zoomRetWidth / 3, center.y - 1),
                           &Point2I(center.x + zoomRetWidth, center.y - 1),
                           color);
         sfc->drawLine2d(&Point2I(center.x - zoomRetWidth, center.y + 1),
                           &Point2I(center.x - zoomRetWidth / 3, center.y + 1),
                           color);
         sfc->drawLine2d(&Point2I(center.x + zoomRetWidth / 3, center.y + 1),
                           &Point2I(center.x + zoomRetWidth, center.y + 1),
                           color);
      }
      else
      {
         //draw in the lines
         GFXBitmap *bmp = (! mbLowRes ? mBitmap : lrBitmap);
         sfc->drawBitmap2d(bmp, &Point2I(center.x - bmp->getWidth() / 2,
                                             center.y - bmp->getHeight() / 2));
      }
   }

   //update the closest player, and write
   const char *name;
   float damage;
   
   //see if we should use the sensor
   const char *sensorStatus = Console->getVariable("pref::sensorOn");
   if (sensorStatus && (! stricmp(sensorStatus, "FALSE"))) name = NULL;
   else name = AcquireClosestTarget(sfc, damage);
   
   if (name)
   {
      Point2I damBoxSize;
      if (! mbLowRes) damBoxSize.set(27, 6);
      else damBoxSize.set(20, 4);

      Point2I damUpperL;
      damUpperL.x = center.x + (! mbLowRes ? 16 : 10);
      damUpperL.y = center.y + (! mbLowRes ? 16 : 10);

      if(name[0])
      {
         GFXFont *font = (! mbLowRes ? textFont : lrTextFont);
         sfc->drawText_p(font, &damUpperL, name);
      
         //draw the player's damage
         damUpperL.y += font->getHeight() + (! mbLowRes ? 4 : 3);
      }
      Point2I damLowerR = damUpperL;
      damLowerR.x += damBoxSize.x;
      damLowerR.y += damBoxSize.y;
      
      Point3F poly[4];
      for (int j = 0; j < 4; j++)
      {
         poly[j].z = 1.0f;
      }
      poly[0].x = damUpperL.x;
      poly[0].y = damUpperL.y;
      poly[1].x = damLowerR.x;
      poly[1].y = damUpperL.y;
      poly[2].x = damLowerR.x;
      poly[2].y = damLowerR.y;
      poly[3].x = damUpperL.x;
      poly[3].y = damLowerR.y;
      
      if (damage >= 0.0f && damage <= 1.0f)
      {
         //draw the lines
         for (int j = 0; j < 4; j++)
         {
            sfc->drawLine2d(&Point2I(poly[j].x, poly[j].y),
                              &Point2I(poly[(j + 1)%4].x, poly[(j + 1)%4].y),
                              BLUE_HILITE);
         }
      
         //calculate the damage bar width
         int damWidth = max((int)((1.0f - damage) * (damBoxSize.x - 2)), 1);
         if (damage == 1.0f) damWidth = 0;
         if (damWidth > 0)
         {
            damUpperL.x += 1;
            damUpperL.y += 1;
            damLowerR.x = damUpperL.x + damWidth + 1;
            poly[0].x = damUpperL.x;
            poly[0].y = damUpperL.y;
            poly[1].x = damLowerR.x;
            poly[1].y = damUpperL.y;
            poly[2].x = damLowerR.x;
            poly[2].y = damLowerR.y;
            poly[3].x = damUpperL.x;
            poly[3].y = damLowerR.y;
            
            int color = (damWidth >= (! mbLowRes ? 8 : 5) ? GREEN : RED);
            sfc->setFillMode(GFX_FILL_CONSTANT);
            sfc->setShadeSource(GFX_SHADE_NONE);
            sfc->setHazeSource(GFX_HAZE_NONE);
            sfc->setAlphaSource(GFX_ALPHA_NONE);
            sfc->setFillColor(color);
            
            for (int i = 0; i < 4; i++)
            {
               sfc->addVertex(poly + i);
            }
            sfc->emitPoly();
         }
      }
   }
   
   if (gui_ignoreTargetsPref == false)
      renderTargets(sfc);
}


const char* Crosshair::AcquireClosestTarget(GFXSurface*  sfc, float &targetDamage)
{
   ShapeBase *closestTarget = NULL;
   ShapeBase *closestPlayer = NULL;
   bool closestTargetIsPlayer;
   Point2I closestTargetCenter;
   RealF dist = -1;
   
   //make sure the player exists...
   if(!cg.playerManager || ! cg.psc || !cg.psc->getControlObject())
      return NULL;
   PlayerManager::ClientRep *myClientRep = cg.playerManager->findClient(manager->getId());
   if(!myClientRep)
      return NULL;

	Point3F eyePos = cg.psc->getControlObject()->getEyeTransform().p;

   //need the sim container to run a line of sight
   SimContainer *sc = dynamic_cast<SimContainer *>(manager->findObject(SimRootContainerId));
   if (! sc) return NULL;
   
   SimGui::TSControl *myParent = dynamic_cast<SimGui::TSControl*>(parent);
   if (! myParent) return NULL;
   TS::Camera *cam = myParent->getCamera();
   if (! cam) return NULL;
   
   //lock the camera before checking player visibilities
   if (cam->isLocked()) return NULL;
   cam->lock();
           
   SimSet *objSet = dynamic_cast<SimSet*>(manager->findObject(SensorVisibleSetId));
   if (! objSet) return NULL;
   Vector<SimObject*>::iterator jj;
   Point3F playerPosition = cg.psc->getControlObject()->getLinearPosition();
   Point2I center(parent->extent.x / 2, parent->extent.y / 2);
   SimCollisionInfo   info;
   SimContainerQuery  query;
   
   for (jj = objSet->begin(); jj < objSet->end(); jj++)
   {
      // right now we only HUD players... but we can hud any ShapeBase...
		Player *player = NULL;
      ShapeBase *target = NULL;
      
      if (cg.psc->getControlObject() == *jj ||
      		(*jj)->getType().test(CorpseObjectType))
         continue;
       
      target = static_cast<ShapeBase*>(*jj);
      if (! target)
         continue;
     
      bool isPlayer = FALSE;
		if ((*jj)->getType().test(SimPlayerObjectType))
      {
         isPlayer = TRUE;
         player = static_cast<Player*>(*jj);
         
      }
      
      //make sure the player is still alive, and not yourself or
		//the vehicle you are mounted on
      if (isPlayer)
      {
   		if (player->isDead() || 
   				player == cg.psc->getControlObject() ||
   				player->getMountObject() == cg.psc->getControlObject())
   		   continue;
         if(!cg.psc->playerVisible(player->getOwnerClient()))
            continue;
      }
         
      //clip the target's projected position to the view plane
		Point3F tempTargetPoint;
		TS::TransformedVertex tVert;

      TMat3F mi;
      if (isPlayer)
      {
         player->getMountTransform(Player::BackpackMount, &mi);
         tempTargetPoint.x = mi.p.x;
         tempTargetPoint.y = mi.p.y;
         tempTargetPoint.z = mi.p.z + 0.65;
      }
      else
      {
         const Box3F       &box    = target->getBoundingBox();
         float              size   = box.fMax.z - box.fMin.z;
         tempTargetPoint = target->getBoxCenter();
         tempTargetPoint.z += size / 2;
      }

		cam->transformProject(tempTargetPoint, &tVert);
		if (! (tVert.fStatus & TS::TransformedVertex::Projected))
		   continue;
      
      //see if the coords are within the view plane
      Point2I screenCoords = globalToLocalCoord(position);
      Point2I targetCenter;
      targetCenter.x = (int)(tVert.fPoint.x);
      targetCenter.y = (int)(tVert.fPoint.y);
      if ((targetCenter.x < screenCoords.x) || (targetCenter.x > screenCoords.x + parent->extent.x) ||
            (targetCenter.y < screenCoords.y) || (targetCenter.y > screenCoords.y + parent->extent.y))
      {
         continue;
      }

      bool isVis = false;

      if(isPlayer)
      {
         const Box3F       &box    = target->getBoundingBox();
         float              size   = max(box.fMax.z - box.fMin.z, box.fMax.x - box.fMin.x);

         // Build query
         query.id          = cg.psc->getControlObject()->getId();
         query.type        = -1;
         query.mask        = SimTerrainObjectType   | StaticObjectType   | 
                             ItemObjectType         | VehicleObjectType  | 
                             SimInteriorObjectType  | MoveableObjectType;
         query.detail      = SimContainerQuery::DefaultDetail;
         query.box.fMin    = eyePos;        // Shoot from the eye
         query.box.fMax    = target->getBoxCenter();
         query.box.fMax.z += size / 2.0f;  // To the center of the object


         //if we can see the player
         if (! sc->findLOS(query, &info))
         {
            GFXBitmap *bmp;
            if (cg.playerManager->getNumTeams() > 1 && myClientRep->team == target->getTeam())
               bmp = (! mbLowRes ? mFriendBMP : lrFriendBMP);
            else
               bmp = (! mbLowRes ? mFoeBMP : lrFoeBMP);

            sfc->drawBitmap2d(bmp, &Point2I(targetCenter.x - bmp->getWidth() / 2, targetCenter.y - bmp->getHeight()));
            isVis = true;
         }
      }
      else
         isVis = target->getTeam() == myClientRep->team;


      if(isVis)
      {
         const Box3F       &box    = target->getBoundingBox();
         RealF tempWidth = max(box.fMax.x - box.fMin.x, box.fMax.y - box.fMin.y);
         RealF tempHeight = box.fMax.z - box.fMin.z;
         RealF tempTargetWidth = cam->transformProjectRadius(tempTargetPoint, tempWidth) / 2;
         RealF tempTargetHeight = cam->transformProjectRadius(tempTargetPoint, tempHeight);
         bool showDamage = (tempTargetWidth >= 1.0f);
               
         Point2I retUL, retLR;
      
         retUL.x = targetCenter.x - max((int)tempTargetWidth, 8);
         retUL.y = targetCenter.y - 4;
         retLR.x = targetCenter.x + max((int)tempTargetWidth, 8);
         retLR.y = targetCenter.y + max((int)tempTargetHeight, 12);
         
            //now see if the player is under the crosshairs
         if ((center.x >= retUL.x) && (center.x <= retLR.x) &&
             (center.y >= retUL.y) && (center.y <= retLR.y))
         {
            //see if this player is the closest so far
            RealF temp = abs(m_dist(playerPosition, target->getLinearPosition())); 
            if ((dist < 0) || (temp < dist))
            {
               dist = temp;
               
               closestTarget = target;
               closestTargetIsPlayer = isPlayer;
               closestTargetCenter = targetCenter;
               if(isPlayer)
                  closestPlayer = target;
               if (showDamage)
               {
                  targetDamage = target->getDamageLevel();
               }
               else
               {
                  targetDamage = -1.0f;
               }
            }
		   }
      }
   }
    
    //unlock the camera
   cam->unlock();

   if(closestTarget && !closestTargetIsPlayer)
   {
      // run a LOS to the target
      query.id          = cg.psc->getControlObject()->getId();
      query.type        = -1;
      query.mask        = SimTerrainObjectType   | StaticObjectType   | 
                          ItemObjectType         | VehicleObjectType  | 
                          SimInteriorObjectType  | MoveableObjectType;
      query.detail      = SimContainerQuery::DefaultDetail;
      query.box.fMin    = eyePos;        // Shoot from the eye
      query.box.fMax    = closestTarget->getBoxCenter();
      if (sc->findLOS(query, &info))
      {
         if(info.object != closestTarget)
         {      
            closestTarget = closestPlayer;
            closestTargetIsPlayer = true;
         }
      }
   }

   if (closestTarget)
   {
      if (closestTargetIsPlayer && cg.playerManager)
      {
         PlayerManager::ClientRep *playerInfo = cg.playerManager->findBaseRep(closestTarget->getOwnerClient());
   		if (playerInfo)
         {
            if (targetDamage < 1.0f)
            {
               return playerInfo->name;
            }
         }
      }
      else
      {
         GameBase::GameBaseData *dat = closestTarget->getDatPtr();
         GFXBitmap *bmp = dat->iconBmp[1];
         if(bmp)
            sfc->drawBitmap2d(bmp, &Point2I(closestTargetCenter.x - (bmp->getWidth() / 2), closestTargetCenter.y - bmp->getHeight()));

         const char *tname = "";
         if(closestTarget->sensorKey != -1 && cg.psc)
            tname = cg.psc->getTargetName(closestTarget->sensorKey);
         if(!tname[0] && dat->description)
            tname = dat->description;
         return tname;
      }
   }
   
   return NULL;
}   

void Crosshair::renderTargets(GFXSurface* sfc)
{
   //make sure the player exists...
   if(!cg.psc)
      return;
   if(!cg.psc->getControlObject())
      return;
	Point3F eyePos = cg.psc->getControlObject()->getEyeTransform().p;

   // Ok, we have the eyepoint.  Now what we need to do is get the datablock for
   //  the player's current weapon, then loop through all the clientreps, drawing
   //  the targets if they are active.  Direction to draw the target at is
   //  determined by querying the projectile data block.
   //
   ShapeBase* pControl = dynamic_cast<ShapeBase*>(cg.psc->getControlObject());
   AssertFatal(pControl, "Error, control object should always be a shapebase");
   ProjectileDataType primaryWeapon;
   ProjectileDataType secondaryWeapon;
   primaryWeapon.type   = -1;
   secondaryWeapon.type = -1;
   pControl->getWeaponAmmoType(primaryWeapon, secondaryWeapon);

   Point3F gravityVec;
   float gravity = 0.0f;
   if (pControl->getForce(SimMovementGravityForce, &gravityVec) == true)
      gravity = -(gravityVec.len());

   SimGui::TSControl *myParent = dynamic_cast<SimGui::TSControl*>(parent);
   if (! myParent) return;
   TS::Camera *cam        = myParent->getCamera();
   TS::PointArray *pArray = myParent->getRenderContext()->getPointArray();
   if (! cam) return;
   
   //lock the camera before checking player visibilities
   if (cam->isLocked()) return;
   cam->lock();

   // Get the block and player managers...
   AssertFatal(manager != NULL, "This is going to cause problems...");
   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);

   //-------------------------------------- Get the data blocks
   //
	Projectile::ProjectileData* ptrPrimary = NULL;
   if (primaryWeapon.type != -1)
	   ptrPrimary = static_cast<Projectile::ProjectileData *>
	   	          (dbm->lookupDataBlock(primaryWeapon.dataType, primaryWeapon.type));

   SimSet* targetSet = static_cast<SimSet*>(manager->findObject(TargetableSetId));
   AssertFatal(targetSet != NULL, "No TargetableSet?  Shouldn't happen");
   
   for (Vector<SimObject*>::iterator itr = targetSet->begin(); itr != targetSet->end(); itr++) {
      Point3F targetPoint;
      int     team;

      GameBase* pGBase = (GameBase*)(*itr);

      if (pGBase->getTarget(&targetPoint, &team) == false)
         continue;
      if (team != pControl->getTeam())
         continue;

      Point3F primaryVector;
      bool    primaryInRange = false;
      Point3F secondaryVector;
      bool    secondaryInRange = false;
      bool    showPrimary;
      bool    showSecondary = false;

      if (ptrPrimary != NULL) {
         showPrimary = ptrPrimary->getTargetVector(pControl->getEyeTransform().p,
                                                   targetPoint, gravity,
                                                   &primaryVector, &primaryInRange, false);
         showSecondary = ptrPrimary->getTargetVector(pControl->getEyeTransform().p,
                                                     targetPoint, gravity,
                                                     &secondaryVector, &secondaryInRange, true);
      } else {
         showPrimary    = true;
         primaryVector  = targetPoint - pControl->getEyeTransform().p;
         primaryVector.normalize();
         primaryInRange = false;
      }

      // Secondary is never visible if primary is not...
      if (showPrimary == false)
         continue;

      SimCollisionInfo   info;
      SimContainerQuery  query;
      query.id          = pControl->getId();
      query.type        = -1;
      query.mask        = SimTerrainObjectType   | StaticObjectType      | 
                          ItemObjectType         | VehicleObjectType | 
                          SimInteriorObjectType  | MoveableObjectType;
      query.detail      = SimContainerQuery::DefaultDetail;
      query.box.fMin    = pControl->getEyeTransform().p;        // Shoot from the eye
      query.box.fMax    = targetPoint;

      // Ok, so now we know we have to draw the thing, and where it's pointing.
      //  We need to get screen coordinates from that fact.
      Point3F tempTargetPoint = pControl->getEyeTransform().p + (primaryVector * 2.0f);
		TS::TransformedVertex tVert;
		TS::TransformedVertex tVertTarget;

		cam->transformProject(tempTargetPoint, &tVert);
      bool drawPrimaryPoly = true;
		if (!(tVert.fStatus & TS::TransformedVertex::Projected)) {
         drawPrimaryPoly = false;
      }

		TS::TransformedVertex tVertSecondary;
      bool drawSecondaryPoly = showSecondary;
      if (showSecondary) {
         tempTargetPoint = pControl->getEyeTransform().p + (secondaryVector * 2.0f);

		   cam->transformProject(tempTargetPoint, &tVertSecondary);
		   if (!(tVertSecondary.fStatus & TS::TransformedVertex::Projected)) {
            drawSecondaryPoly = false;
         }
      }

      //see if the coords are within the view plane
      Point2I screenCoords = globalToLocalCoord(position);
      Point2I primaryCenter, secondaryCenter;
      primaryCenter.x = (int)(tVert.fPoint.x + 0.5f);
      primaryCenter.y = (int)(tVert.fPoint.y + 0.5f);
      secondaryCenter.x = (int)(tVertSecondary.fPoint.x + 0.5f);
      secondaryCenter.y = (int)(tVertSecondary.fPoint.y + 0.5f);

      //draw the rectangle
      int triangleSize = (! mbLowRes ? 4 : 3);
      Point3F polyOne[3];
      int j;
      for (j = 0; j < 3; j++)
      {
         polyOne[j].z = 1.0f;
      }
      polyOne[0].x = primaryCenter.x;
      polyOne[0].y = primaryCenter.y;
      polyOne[1].x = primaryCenter.x - triangleSize;
      polyOne[1].y = primaryCenter.y - (2 * triangleSize);
      polyOne[2].x = primaryCenter.x + triangleSize;
      polyOne[2].y = primaryCenter.y - (2 * triangleSize);
      
      Point3F polyTwo[3];
      for (j = 0; j < 3; j++)
      {
         polyTwo[j].z = 1.0f;
      }
      polyTwo[0].x = secondaryCenter.x;
      polyTwo[0].y = secondaryCenter.y;
      polyTwo[1].x = secondaryCenter.x - triangleSize;
      polyTwo[1].y = secondaryCenter.y - (2 * triangleSize);
      polyTwo[2].x = secondaryCenter.x + triangleSize;
      polyTwo[2].y = secondaryCenter.y - (2 * triangleSize);
      
      int color;
      if (primaryInRange == true) color = BLUE_HILITE;
      else                        color = GREEN_78;

      pArray->reset();
      int first = pArray->addPoint(tempTargetPoint);
      pArray->addPoint(targetPoint);
      pArray->drawLine(first, first + 1, WHITE);

      sfc->setFillMode(GFX_FILL_CONSTANT);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setHazeSource(GFX_HAZE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      sfc->setFillColor(color);
      
      if (drawPrimaryPoly == true) {
         for (int i = 0; i < 3; i++)
         {
            sfc->addVertex(polyOne + i);
         }
         sfc->emitPoly();
      }

      if (drawSecondaryPoly == true) {
         if (secondaryInRange == true) color = BLUE_HILITE;
         else                          color = GREEN_78;
         sfc->setFillColor(color);
         for (int i = 0; i < 3; i++) {
            sfc->addVertex(polyTwo + i);
         }
         sfc->emitPoly();
      }

      GFXFont *font = (! mbLowRes ? textFont : lrTextFont);
      char buf[256];
      TMat3F muzzleMat;
      if (pControl->getMuzzleTransform(0, &muzzleMat) == false)
         muzzleMat.identity();
      Point3F finalMuzzle;
      m_mul(muzzleMat.p, pControl->getTransform(), &finalMuzzle);
      sprintf(buf, "%2.1f m", (finalMuzzle - targetPoint).len());
      int width = font->getStrWidth(buf) / 2;
      if (drawPrimaryPoly == true) {
         Point2I textPoint(primaryCenter);
         textPoint.y += (! mbLowRes ? 4 : 3);
         textPoint.x -= width;
         sfc->drawText_p(font, &textPoint, buf);
      }
      if (drawSecondaryPoly == true) {
         Point2I textPoint(secondaryCenter);
         textPoint.y += (! mbLowRes ? 4 : 3);
         textPoint.x -= width;
         sfc->drawText_p(font, &textPoint, buf);
      }
   }

   cam->unlock();
}

};
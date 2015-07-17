#include <ts.h>
#include <baseshadowRenderImage.h>
#include <simContainer.h>
#include <m_collision.h>
#include <simterrain.h>
#include <grdcollision.h>
#include <g_surfac.h>
#include <gwDeviceManager.h>
#include <simGame.h>
#include <d_caps.h>

#define OnGroundTol 3
#define ShadowFeelerLength 100
#define ShadowItrFeeler 4

int BaseShadowRenderImage::shadowNum = -1;
float BaseShadowRenderImage::shadowLiftLo = 0.1f;
float BaseShadowRenderImage::shadowLiftHi;
float BaseShadowRenderImage::shadowLiftStep = 0.03f;

int BaseShadowRenderImage::maxProjectionPolys = 100;
int BaseShadowRenderImage::maxTerrainPolys    =  50;
int BaseShadowRenderImage::maxItrPolys        =  50;

//------------------------------------------------------------
BaseShadowRenderImage::BaseShadowRenderImage()
{
   castShadow=false;
   drawShape = true;
   animateShape = true;
   alphaLevel = 0.5f;
   swingDown  = 0.0f;
   shadowDetailScale = 1.0f;
   root = 0;
   currentDetail = -1;
   shadowOwnDetail = false;
   forcedDetailLevel = false;
   lightDirection.set( 1.0f, 1.0f, -1.0f);
   lastLight.set();
   projectionMask = SimInteriorObjectType | SimTerrainObjectType;
   shadowSettings.cacheProjection = false;
   shadowSettings.recacheProjection = false;
   shadowSettings.shadowDetail = -1;
   shadowSettings.hiShadowDetail = 0;
   shadowSettings.blurMethod = 3;
   shadowSettings.bmpDim = 64;
   shadowSettings.updateDelta = prevShadowUpdateDelta = 0;
   shadowSettings.projectTerrainOnly = false;
   shadowSettings.useFloor = false;
   shadowSettings.liftFloor = 1.0f;
   prevBmpDim = -1; // force new bmp
   shape = 0;
   itype = Translucent;
}

BaseShadowRenderImage::~BaseShadowRenderImage()
{
   shadow.clearResources(deviceManager.getGFXDeviceManager());
}

//------------------------------------------------------------

void BaseShadowRenderImage::render(TSRenderContext &rc)      
{
   // don't get bit by matrix flag bug -- set translation flag just in case
   transform.flags |= TMat3F::Matrix_HasTranslation;

   rc.getCamera()->pushTransform(transform);

   if (!forcedDetailLevel)
      currentDetail = shape->selectDetail(rc);

   if (currentDetail == -1)
   {
      rc.getCamera()->popTransform();
      return;
   }

   if (animateShape)
      shape->animate();

   if (drawShape)
      shape->render(rc);

   rc.getCamera()->popTransform();

   if (castShadow && !IsEqual(shadowDetailScale,0.0f))
      drawShadow(rc);
}

//------------------------------------------------------------

void BaseShadowRenderImage::drawShadow(TSRenderContext & rc)
{
   if (!castShadow || shadowSettings.shadowDetail==-1 || alphaLevel == 0.0f)
      return;
      
   if (shadowNum > 0)
   {
      shadowLiftHi = shadowLiftLo + ((float)shadowNum) * shadowLiftStep;
      shadowNum *= -1;
   }

   shadow.shadowLift = shadowLiftHi + ((float)shadowNum) * shadowLiftStep;
   
   shadowNum++;

   if (shadowSettings.cacheProjection)
      shadow.renderCache(rc);
   else if (!shadow.projectionList.empty())
      shadow.render(rc);
}

//------------------------------------------------------------

void BaseShadowRenderImage::setShadowDetails(TSRenderContext & rc)
{
   rc;

   // if the shadow is almost parallel to camera, lower detail
   float angleMult;
   if (shadow.maxPlaneDot < 0.5f)
      angleMult = 2.0f * shadow.maxPlaneDot;
   else
      angleMult = 1.0f;
   float adjSize2 = projSize * angleMult;
   float adjSize  = adjSize2 * shadowDetailScale;

   // choose blur method and bmp dim
   if (adjSize>=160)
   {
      shadowSettings.bmpDim = 64;
      shadowSettings.blurMethod = 23;
   }
   else if (adjSize >= 27)
   {
      shadowSettings.bmpDim = 64;
      shadowSettings.blurMethod = 3;
   }
   // can draw 16x16 only if actually small or cos angle near zero,
   // but not just because shadow details are down...
   else if (adjSize2 >= 8)
   {
      shadowSettings.bmpDim = 32;
      shadowSettings.blurMethod = 2;
   }
   // do a little blur if size is at least 8 pixels
   else if (projSize >= 8)
   {
      shadowSettings.bmpDim = 16;
      shadowSettings.blurMethod = 2;
   }
   else
   {
      shadowSettings.bmpDim = 16;
      shadowSettings.blurMethod = 0;
   }

   // choose update rate -- go from 0 to 0.1 between 50 and 25 pixel size
   // use real size rather than adjusted size here...
   if (projSize>50)
      shadowSettings.updateDelta = 0.0f;
   else if (projSize<25)
      shadowSettings.updateDelta = 0.1f;
   else
      shadowSettings.updateDelta = 0.1f - 0.1f * (projSize - 25.0f) / 25.0f;

//   shadowSettings.blurMethod = Console->getIntVariable("$Shadow::blur",3);
//   shadowSettings.updateDelta = Console->getFloatVariable("$Shadow::Rate",0.01f);
//   shadowSettings.bmpDim = Console->getIntVariable("$Shadow::bmpDim",64);
}

//------------------------------------------------------------
// first pass at detailing -- not all info available yet
// can only do early out
//------------------------------------------------------------
void BaseShadowRenderImage::setShadowDetailsA(TSRenderContext & rc)
{
   rc;

   float adjSize = projSize * shadowDetailScale;

   if (adjSize < 4)
      shadowSettings.shadowDetail = -1;
}
//------------------------------------------------------------

void BaseShadowRenderImage::preRenderShadow(TSRenderContext & rc, float curTime)
{
   if (!castShadow)
      return;
      
	// do visibility check for shadow
   Point3F center;
   m_mul(shape->getShape().fCenter,transform,&center);
	SphereF shadowSphere = SphereF(center,shape->getShape().fRadius);
	int shadowVis = rc.getCamera()->testVisibility(shadowSphere);
	if ( shadowVis==TS::ClipNoneVis )
   {
      shadowSettings.shadowDetail = -1;
      return;
   }

   // shape detail to use for shadows
   if (!shadowOwnDetail)
   {
      projSize = rc.getCamera()->transformProjectRadius( center, shape->getShape().fRadius );
      float adjustedSize = shadowDetailScale * projSize;
                           
      shadowSettings.shadowDetail = shape->getShape().selectDetail( adjustedSize );

      if (shadowSettings.shadowDetail==-1)
         return;
   }

   // first pass at shadow details
   setShadowDetailsA(rc);

   // set shadow details may veto shadow
   if (shadowSettings.shadowDetail==-1)
      return;

   // set light direction, but only if different than before
   Point3F * pLight = &lightDirection;
   Point3F tempDirection;
   if (swingDown != 0.0f)
   {
      tempDirection = (lightDirection * (1.0f - swingDown)) + (Point3F(0, 0, -1) * swingDown);
      tempDirection.normalize();
      pLight = &tempDirection;
   }
   if (m_dot(*pLight,lastLight) < 0.999)
   {
      shadow.setLight( *pLight, shape);
      lastLight = *pLight;
      nextShadowUpdateTime = -1;
   }

   // set position of shadow
   shadow.setPosition(transform.p);

   if (shadowSettings.cacheProjection)
   {
      if (shadowSettings.recacheProjection)
      {
         shadow.calcSourceWindow(shape,transform);
         getPolys();
         shadow.cachePolys();
         shadowSettings.recacheProjection = false;
      }
   }
   else
   {
      shadow.calcSourceWindow(shape,transform);
      getPolys();
      Point3F cc = rc.getCamera()->getTCW().p;
      Point3F camY = transform.p-cc;
      camY.normalize();
      shadow.getPlanes(cc,camY);
   }
   
   setShadowDetails(rc);
   
   // set shadow details may veto shadow
   if (shadowSettings.shadowDetail==-1)
      return;

   if (shadowSettings.shadowDetail < shadowSettings.hiShadowDetail)
      shadowSettings.shadowDetail = shadowSettings.hiShadowDetail;

   // adjust next update time...
   nextShadowUpdateTime += shadowSettings.updateDelta - prevShadowUpdateDelta;
   prevShadowUpdateDelta = shadowSettings.updateDelta;

   // adjust bmp dim...
   int newBmpDim = shadowSettings.bmpDim;
   if (newBmpDim != prevBmpDim)
   {
      prevBmpDim = newBmpDim;
      shadow.setBitmapSize(deviceManager.getGFXDeviceManager(),newBmpDim,rc.getSurface());
      nextShadowUpdateTime = -1;
   }

   shadow.setAlphaLevel(alphaLevel);

   // create the shadow bitmap if needed
   if (curTime > nextShadowUpdateTime)
   {
      if (shadowSettings.useFloor)
      {
         SimContainerQuery query;
         Box3F & box = query.box;
         box.fMin    = box.fMax = center;
         box.fMin.z += shape->getShape().fRadius;
         box.fMax.z -= shape->getShape().fRadius * 1.5f;
         query.id = -1;
         query.type = -1;
         query.mask = shadowSettings.projectTerrainOnly ? SimTerrainObjectType : projectionMask;
         SimCollisionInfo collision;
         if (root->findLOS(query,&collision))
         {
            Point3F n,p;
            m_mul(collision.surfaces[0].position,collision.surfaces.tWorld,&p);
            m_mul(collision.surfaces[0].normal,(RMat3F&)collision.surfaces.tWorld,&n);
            p -= transform.p;
            Point3F lift = n;
            lift *= shadowSettings.liftFloor;
            p += lift;
            shadow.setFloor(p,n);
         }
         else
            shadow.clearFloor();
      }
      else
         shadow.clearFloor();

      shape->setDetailLevel(shadowSettings.shadowDetail);
      shape->animate();
      GFXPalette * pal = SimGame::get()->getWorld(SimGame::CLIENT)->getPalette();
      AssertFatal(pal, "invalid palette");
      // getShadowBitmap assumes calcSourceWindow already called...
      // ...but that's ok because we called it above
      shadow.getShadowBitmap(shape,pal,transform,shadowSettings.blurMethod);
      nextShadowUpdateTime = curTime + shadowSettings.updateDelta;
   }

	AssertFatal(root,
		"shadowRenderImage::preRenderShadow:  cannot cast shadow before \'root\' container set");

   // this'll keep track of how many shadows are out there
   if (shadowNum < 0)
      shadowNum=1; // first preRender this render cycle
   else
      shadowNum++;
}

//------------------------------------------------------------

void BaseShadowRenderImage::getPolys()
{
   shadow.projectionList.clearPolys();

   float rad = shape->getShape().fRadius;
   
   // use this light because it reflects effect of swingDown...
   Point3F & light = lastLight;

   // structures for query
   SimContainerQuery query;

   // now build a really big box
   // NOTE: we could make box a little smaller by getting shapes
   // bounding box rather than using it's sphere, but it's going
   // to be a really big box anyway, so we don't bother...
   Point3F center,preC;
   m_mul(shape->getShape().fCenter,shape->fRootDeltaTransform,&preC);
   m_mul(preC,transform,&center);
   Box3F & shadowBox = query.box;
   shadowBox.fMin = center;
   shadowBox.fMin.x -= rad;
   shadowBox.fMin.y -= rad;
   shadowBox.fMin.z -= rad;
   shadowBox.fMax = center;
   shadowBox.fMax.x += rad;
   shadowBox.fMax.y += rad;
   shadowBox.fMax.z += rad;

   // expand box by projecting in direction of light
   if (light.x>0)
      shadowBox.fMax.x += light.x * ShadowFeelerLength;
   else
      shadowBox.fMin.x += light.x * ShadowFeelerLength;
   if (light.y>0)
      shadowBox.fMax.y += light.y * ShadowFeelerLength;
   else
      shadowBox.fMin.y += light.y * ShadowFeelerLength;
   if (light.z>0)
      shadowBox.fMax.z += light.z * ShadowFeelerLength;
   else
      shadowBox.fMin.z += light.z * ShadowFeelerLength;

   query.id = -1;
   query.type = -1;
   query.mask = shadowSettings.projectTerrainOnly ? SimTerrainObjectType : projectionMask;

//   terrOnly = true;

   SimContainerList cList;
   if (root->findIntersections(query,&cList))
   {
      int i;
      for (i=0;i<cList.size();i++)
      {
         SimContainer * cnt = cList[i];
         if (cnt->getType() & SimTerrainObjectType)
            getTerrPolys(static_cast<SimTerrain*>(cnt));
         else
            getUnknownPolys(cnt);
      }
   }
}

//------------------------------------------------------------

void BaseShadowRenderImage::getTerrPolys( SimTerrain * terr)
{
   float rad = shadow.getShadowRadius();
	GridFile *terrFile = terr->getGridFile();
	const TMat3F & terrInv = terr->getInvTransform();

	// get the light direction in terrain space
	Point3F lightInTerr;
	m_mul(lastLight,(RMat3F&)terr->getInvTransform(),&lightInTerr);
	if (IsZero(lightInTerr.z,0.01f))
		return; // light parallel to x-y plane

	// center of shape in world coords
	Point3F shapeInWorld,preC;
   m_mul(shape->getShape().fCenter,shape->fRootDeltaTransform,&preC);
   m_mul(preC,transform,&shapeInWorld);

	// feet of shape in world coords
	Point3F feetInTerr;
   m_mul(transform.p,terrInv,&feetInTerr);

   // first task is to find plane on ground where light hits
   // this plane will be used to decide how big to make box
   // to grab terrain polys from
   Point3F n,p;
   float dot;
   bool gotIt = false;

   // first, are we standing on the terrain...if so, use this plane
   CollisionSurface cs;
   if (terrFile->getSurfaceInfo(feetInTerr,&cs))
   {
      if (IsZero(feetInTerr.z-cs.position.z,rad))
      {
         n = cs.normal;
         p = cs.position;
         dot = m_dot(n,lightInTerr);
         if (dot < -0.2f)
            gotIt = true;
      }
   }

   // now project in direction of light down from top of shape
   Point3F ta, a = shapeInWorld;
   a.z += rad;
   m_mul(a,terrInv,&ta);

   Point3F tb = lightInTerr;
   tb *= ShadowFeelerLength;
   tb += ta;
   GridCollision coll(terrFile,NULL);
   if (coll.collide(ta,tb) && terrFile->getSurfaceInfo(coll.surface.position,&cs))
   {
      float thisDot = m_dot(lightInTerr,coll.surface.normal);
      if (!gotIt || (thisDot > dot && thisDot < -0.2f))
      {
         n = coll.surface.normal;
         p = coll.surface.position;
         dot = thisDot;
         gotIt = true;
      }
   }
   
   if (!gotIt)
      return; // no terrain polys

	// shape center in terrain space
	Point3F shapeInTerr;
	m_mul(shapeInWorld,terrInv,&shapeInTerr);

   // we now have light, shape center, and a plane all in terrain space
   // build a box around projection of shape center onto plane
   Point3F shapeOnGround;
   float k = 1.0f / dot;
   float t = m_dot(n,p-shapeInTerr) * k;
   shapeOnGround  = lightInTerr;
   shapeOnGround *= t;
   shapeOnGround += shapeInTerr;

   // make the box
   Box2F shadowBox;
   shadowBox.fMin = shadowBox.fMax = shapeOnGround;
   float tx = -rad * n.x * k;
   float ty = -rad * n.y * k;
   float tz = -rad * n.z * k;
   shadowBox.fMin.x -= rad + tx*lightInTerr.x + 
                       fabs(ty*lightInTerr.x) + 
                       fabs(tz*lightInTerr.x);
   shadowBox.fMax.x += rad - tx*lightInTerr.x + 
                       fabs(ty*lightInTerr.x) + 
                       fabs(tz*lightInTerr.x);
   
   shadowBox.fMin.y -= rad + ty*lightInTerr.y + 
                       fabs(tx*lightInTerr.y) + 
                       fabs(tz*lightInTerr.y);
   shadowBox.fMax.y += rad - ty*lightInTerr.y + 
                       fabs(tx*lightInTerr.y) + 
                       fabs(tz*lightInTerr.y);
                       
   int maxPolys = min(shadow.projectionList.size() + BaseShadowRenderImage::maxTerrainPolys,
                      BaseShadowRenderImage::maxProjectionPolys);
   
	// now get polys from terrain to project shadow onto
	terrFile->getPolys(shadowBox,terr->getTransform(),shadow.projectionList,maxPolys);
}

//------------------------------------------------------------

//------------------------------------------------------------

void
BaseShadowRenderImage::getUnknownPolys(SimContainer*)
{
}

//------------------------------------------------------------

float BaseShadowRenderImage::findCameraDistance(TSCamera * camera)
{
   const TMat3F & twc = camera->getTWC();
   // don't need x & z (<- sigh), so skip the extra work...
   return twc.p.y + transform.p.x * twc.m[0][1] +
                    transform.p.y * twc.m[1][1] +
                    transform.p.z * twc.m[2][1];
}

void BaseShadowRenderImage::setSortValue(TSCamera * camera)
{
	if (itype != SimRenderImage::Translucent)
	{
		sortValue = 0;
		return;
	}
   
   float cd = findCameraDistance(camera);
   float nd = camera->getNearDist();
   sortValue = (cd<=nd) ? 0.99f : nd/cd;
}

void
BaseShadowRenderImage::setImageType()
{
   itype = (castShadow && !IsEqual(shadowDetailScale,0.0f) && alphaLevel != 0.0f) ?
           Translucent : Normal;
}

//------------------------------------------------------------


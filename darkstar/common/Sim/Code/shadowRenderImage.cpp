#include <ts.h>
#include <shadowRenderImage.h>
#include <simContainer.h>
#include <m_collision.h>
#include <simterrain.h>
#include <grdcollision.h>
#include <siminterior.h>
#include <simInteriorShape.h>
#include <g_surfac.h>
#include <gwDeviceManager.h>
#include <simGame.h>
#include <d_caps.h>

#define ShadowItrFeeler 4

void ShadowRenderImage::getItrPolys( SimInterior * itr )
{
	// center of shape in world coords
	Point3F shapeInWorld,preC;
	m_mul(shape->getShape().fCenter,shape->fRootDeltaTransform,&preC);
	m_mul(preC,transform,&shapeInWorld);
	float rad = shape->getShape().fRadius;

   // light to world
   TMat3F LW;
   (RMat3F&) LW = shadow.getTLW();
   LW.p = shapeInWorld;

   // world to itr
   const TMat3F & toItr = itr->getInvTransform();

   // light to interior   
   TMat3F LI;
   m_mul(LW,toItr,&LI);

   // shadow box...
   Box3F box;
   float srad = shadow.getShadowRadius();
   box.fMin.set(-srad,rad*0.5f,-srad);
   box.fMax.set( srad, rad + ShadowItrFeeler*rad, srad);

   const TMat3F & itrToWorld = itr->getTransform();

   int maxPolys = min(shadow.projectionList.size() + ShadowRenderImage::maxItrPolys,
                      ShadowRenderImage::maxProjectionPolys);

	itr->getInstance()->getPolys(box,LI,itrToWorld,shadow.projectionList,maxPolys);
}

void
ShadowRenderImage::getUnknownPolys(SimContainer*cnt)
{
   if (SimInterior* itr = dynamic_cast<SimInterior*>(cnt))
      getItrPolys(itr);
   else if (SimInteriorShape* itrShape = dynamic_cast<SimInteriorShape*>(cnt))
      getItrPolys(itrShape);
}

//------------------------------------------------------------

void ShadowRenderImage::getItrPolys( SimInteriorShape * itr )
{
	// center of shape in world coords
	Point3F shapeInWorld,preC;
	m_mul(shape->getShape().fCenter,shape->fRootDeltaTransform,&preC);
	m_mul(preC,transform,&shapeInWorld);
	float rad = shape->getShape().fRadius;

   // light to world
   TMat3F LW;
   (RMat3F&)LW = shadow.getTLW();
   LW.p = shapeInWorld;

   // world to itr
   const TMat3F & toItr = itr->getInvTransform();

   // light to interior   
   TMat3F LI;
   m_mul(LW,toItr,&LI);

   // shadow box...
   Box3F box;
   float srad = shadow.getShadowRadius();
   box.fMin.set(-srad,rad*0.5f,-srad);
   box.fMax.set( srad, rad + ShadowItrFeeler*rad, srad);

   const TMat3F & itrToWorld = itr->getTransform();

   int maxPolys = min(shadow.projectionList.size() + ShadowRenderImage::maxItrPolys,
                      ShadowRenderImage::maxProjectionPolys);

	itr->getInstance()->getPolys(box,LI,itrToWorld,shadow.projectionList,maxPolys);
}


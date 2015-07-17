//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <ts.h>
#include "tribesShadowRenderImage.h"
#include "interiorShape.h"


TribesShadowRenderImage::TribesShadowRenderImage()
{
}

void
TribesShadowRenderImage::getUnknownPolys(SimContainer* io_pContainer)
{
   InteriorShape* pItrShape = dynamic_cast<InteriorShape*>(io_pContainer);
   if (pItrShape)
      getItrShapePolys(pItrShape);
}


void TribesShadowRenderImage::getItrShapePolys( InteriorShape * itr )
{
	if (!itr->getInstance())
		return;

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

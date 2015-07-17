//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <grdFile.h>
#include <grdCollision.h>
#include <itrInstance.h>
#include <m_coll.h>

#include "shapeCollision.h"
#include "FearDcl.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

ShapeCollisionImage::ShapeCollisionImage()
{
	type = ShapeCollisionImageType;
	priority = ShapeCollisionImageType;
	moveable = true;
	collisionLevel=CollideFaces;
	collisionDetail=-1;
	shapeInst=0;
}


//----------------------------------------------------------------------------

bool ShapeCollisionImage::collide(SimCollisionImage* image, CollisionSurfaceList *list )
{
	AssertFatal(collisionDetail!=-1,
		"ShapeCollisionImage::collide: collisionDetail not set or set improperly.");
	AssertFatal(shapeInst,
		"ShapeCollisionImage::collide:  shape not set or set improperly.");

	TMat3F mat;
	switch (image->getType()) {
		case Sphere: {
			buildImageTransform(image,list,&mat);
			SimCollisionSphereImage* si = static_cast<SimCollisionSphereImage*>(image);
			return shapeInst->collideSphere
				(collisionDetail,si->center,si->radius,&mat,list,
					collisionLevel==CollideFaces);
		}
		case Box: {
			buildImageTransform(image,list,&mat);
			SimCollisionBoxImage* bi = static_cast<SimCollisionBoxImage*>(image);
			return checkBox(bi,&mat,list);
		}
		case Tube: {
			buildImageTransform(image,list,&mat);
			SimCollisionTubeImage* ti = static_cast<SimCollisionTubeImage*>(image);
			return shapeInst->collideTube
				(collisionDetail,ti->start,ti->end,ti->radius,&mat,list,
					collisionLevel==CollideFaces);
		}
		case Line: {
         // Collides line with TS3 shape
			// Lines always collide with the highest detail level.
			buildImageTransform(image,list,&mat);
			SimCollisionLineImage* li = static_cast<SimCollisionLineImage*>(image);
			return shapeInst->collideLine(0,li->start,li->end,&mat,list,
				collisionLevel==CollideFaces);
		}
		//
		case Terrain: {
			SimCollisionTerrImage* ti = static_cast<SimCollisionTerrImage*>(image);
			return checkTerrain(ti,list);
		}
		case Interior: {
			SimCollisionItrImage* ii = static_cast<SimCollisionItrImage*>(image);
			return checkInterior(ii,list);
		}
		case ShapeCollisionImageType: {
			ShapeCollisionImage* si = static_cast<ShapeCollisionImage*>(image);
			return checkShape(si,list);
		}
		default:
			AssertFatal(0,"ShapeCollisionImage:collide: Unrecognized type");
			break;
	}
	return false;
}


//----------------------------------------------------------------------------

void ShapeCollisionImage::buildImageTransform(SimCollisionImage* image,
	CollisionSurfaceList* list,TMat3F* mat)
{
	// Transform that goes from image space into the local space.
	//
	TMat3F &rootDelta = shapeInst->fRootDeltaTransform;
	if (rootDelta.flags & (TMat3F::Matrix_HasRotation |
		TMat3F::Matrix_HasTranslation | TMat3F::Matrix_HasScale))
	{
		// Since this shape has an animation transform, we need
		// to adjust the collision surface transforms.
		TMat3F tmp = list->tLocal;
		m_mul(rootDelta,tmp,&list->tLocal);
		m_mul(rootDelta,transform,&list->tWorld);
	}
	TMat3F wta = list->tWorld;
	wta.inverse();
	m_mul(image->transform,wta,mat);
}	


//----------------------------------------------------------------------------

bool ShapeCollisionImage::checkBox(SimCollisionBoxImage * bi,
		TMat3F * trans, CollisionSurfaceList * list)
{
	// Shape-box collision routine wants box centered at 
	// origin and wants radii not min/max
	Point3F radii;
	Point3F & min = bi->box.fMin, & max = bi->box.fMax;
	radii.x = max.x-min.x;
	radii.y = max.y-min.y;
	radii.z = max.z-min.z;
	radii *= 0.5f;

	// Shift transform so box center at origin
	Point3F tmpPoint;
	tmpPoint.x = min.x + radii.x;
	tmpPoint.y = min.y + radii.y;
	tmpPoint.z = min.z + radii.z;
	trans->preTranslate(tmpPoint);

	return shapeInst->collideBox(collisionDetail,radii,trans,list,false);
}


//----------------------------------------------------------------------------

bool ShapeCollisionImage::checkTerrain(SimCollisionTerrImage * image,
	CollisionSurfaceList * list)
{
	// Bounding sphere against terrain, going to do this
	// in terrain space
	TMat3F mat;
	TMat3F tmp = image->transform;
	tmp.inverse();
	m_mul(transform,tmp,&mat);
	
	//
	Point3F center;
	m_mul(sphere.center,mat,&center);
	list->increment();
	CollisionSurface& info = list->last();
	if (image->gridFile->getSurfaceInfo(center,&info)) {
		float dist = m_dot(center,info.normal) -
			m_dot(info.position,info.normal);
		if (fabs(dist) < sphere.radius) {
			info.distance = dist;
			// Pos & normal back to player space
			list->tLocal = mat;
			list->tLocal.inverse();
			list->tWorld = image->transform;
			return true;
		}
	}
	list->clear();
	return false;
}


//----------------------------------------------------------------------------

bool ShapeCollisionImage::checkInterior(SimCollisionItrImage * image,
		CollisionSurfaceList * list)
{
	// Bounding box against interior, going to do this
	// in interior space
	TMat3F mat;
	TMat3F tmp = image->transform;
	tmp.inverse();
	m_mul(transform,tmp,&mat);
	
	//
	ITRCollision & itrColl = image->collision;
	itrColl.geometry = image->instance->getGeometry();
	itrColl.materialList = image->instance->getMaterialList();
	itrColl.transform = &mat;
	itrColl.collisionList = list;

	if (!itrColl.collide(bbox))
		return false;

	// Pos & normal back to player space
	// Material, surface & part still refer to the interior
	list->tLocal = mat;
	list->tLocal.inverse();
	list->tWorld = image->transform;
	return true;
}


//----------------------------------------------------------------------------

bool ShapeCollisionImage::checkShape(ShapeCollisionImage * image,
	CollisionSurfaceList * list)
{
	// Sphere against polys in other shape
	m_mul(image->shapeInst->fRootDeltaTransform,
		image->transform,&list->tWorld);
	TMat3F wti = list->tWorld;
	wti.inverse();
	TMat3F mat;
	m_mul(transform,wti,&mat);

	if (!image->shapeInst->collideSphere(image->collisionDetail,
			sphere.center,sphere.radius,&mat,list,
			image->collisionLevel == CollideFaces))
		return false;

	list->tLocal = mat;
	list->tLocal.inverse();
	return true;
}


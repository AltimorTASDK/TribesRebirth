//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <grdFile.h>
#include <grdCollision.h>
#include <itrInstance.h>
#include <m_coll.h>

#include "playerCollision.h"
#include "FearDcl.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

PlayerCollisionImage::PlayerCollisionImage()
{
	type = PlayerCollisionImageType;
	priority = PlayerCollisionImageType;
	moveable = true;
	collisionLevel=CollideFaces;
	collisionDetail=-1;
	shapeInst=0;
}


//----------------------------------------------------------------------------

bool PlayerCollisionImage::collide(SimCollisionImage* image, CollisionSurfaceList *list )
{
	AssertFatal(collisionDetail!=-1,
		"PlayerCollisionImage::collide: collisionDetail not set or set improperly.");
	AssertFatal(shapeInst,
		"PlayerCollisionImage::collide:  shape not set or set improperly.");

	// Test collision of image argument against this image.
	// Collision surfaces are returned in this image space.

	// Build the transform which goes from image space to the
	// space in which the testing will take place.
	TMat3F mat;
	list->stepVector.set(0,0,-1);

	// Do the tests...
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
         Point3F a,b;
         RealF hit;

         SimCollisionLineImage *line = static_cast<SimCollisionLineImage *>(image);
         
         Parent::buildImageTransform(image, &mat);
         m_mul(line->start, mat, &a);
         m_mul(line->end,   mat, &b);

         if(m_lineAABox(a, b, crouchBox.fMin, crouchBox.fMax, hit))
         {
            CollisionSurface surface;

				surface.time = hit;
            surface.position.x = line->start.x + (hit * (line->end.x - line->start.x));
            surface.position.y = line->start.y + (hit * (line->end.y - line->start.y));
            surface.position.z = line->start.z + (hit * (line->end.z - line->start.z));
            surface.distance = m_distf(surface.position, line->start);
            surface.normal.set(0, 0, 1);  // incomplete and wrong
            surface.material = surface.surface = 0;
				
				list->tWorld.identity();
				list->push_back(surface);
            
            return (true);
         }
         return (false);

#if 0
         // Collides line with bounding box (axis-aligned) -- easier to hit
         // than line/TS3, more accurate than line/Sphere
         float   rTime = 0.0f;
         Point3F objStart, objEnd;
         Parent::buildImageTransform(image, &mat);

         SimCollisionLineImage *line = static_cast<SimCollisionLineImage *>(image);

         m_mul(line->start, mat, &objStart);
         m_mul(line->end,   mat, &objEnd);

         return (m_lineAABox(objStart, objEnd,
            bbox.fMin, bbox.fMax, rTime) && rTime > 0.0f);
#endif
#if 0
         // Collides line with sphere -- temporary "fix" to make it a little 
         //easier to hit others 
         SimCollisionSphereImage sphere;
         
         sphere.center    = this->sphere.center;
         sphere.radius    = this->sphere.radius;
         sphere.transform = transform;

         SimCollisionLineImage *line = 
            static_cast<SimCollisionLineImage *>(image);

         return (line->collide(&sphere, list));
#endif
#if 0
         // Collides line with TS3 shape -- very difficult to hit
			buildImageTransform(image,list,&mat);
			SimCollisionLineImage* li = static_cast<SimCollisionLineImage*>(image);
			return shapeInst->collideLine
				(collisionDetail,li->start,li->end,&mat,list,
				collisionLevel==CollideFaces);
#endif
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
		case PlayerCollisionImageType: {
			PlayerCollisionImage* pi = static_cast<PlayerCollisionImage*>(image);
			return checkPlayer(pi,list);
		}
		default:
			AssertFatal(0,"PlayerCollisionImage:collide: Unrecognized type");
			break;
	}
	return false;
}


//----------------------------------------------------------------------------

void PlayerCollisionImage::buildImageTransform(SimCollisionImage* image,
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

bool PlayerCollisionImage::checkBox(SimCollisionBoxImage * bi,
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

bool PlayerCollisionImage::checkTerrain(SimCollisionTerrImage * image,
	CollisionSurfaceList * list)
{
	// Bounding sphere against terrain, going to do this
	// in terrain space.  Ignores player rotation
	TMat3F mat,nmat;
	TMat3F tmp = image->transform;
	tmp.inverse();
	nmat.identity();
	nmat.p = transform.p;
	m_mul(nmat,tmp,&mat);
	
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

bool PlayerCollisionImage::checkInterior(SimCollisionItrImage * image,
		CollisionSurfaceList * list)
{
	// Bounding box against interior, going to do this
	// in interior space.  Ignores player rotation.
	TMat3F mat,nmat;
	TMat3F tmp = image->transform;
	tmp.inverse();
	nmat.identity();
	nmat.p = transform.p;
	m_mul(nmat,tmp,&mat);
	
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

bool PlayerCollisionImage::checkShape(ShapeCollisionImage * image,
	CollisionSurfaceList * list)
{
#if 0
	TMat3F mat,tmp = image->transform;
	tmp.inverse();
	m_mul(transform,tmp,&mat);

	// Shape-box collision routine wants box centered at 
	// origin and wants radii not min/max
	Point3F radii;
	Point3F & min = bbox.fMin, & max = bbox.fMax;
	radii.x = max.x-min.x;
	radii.y = max.y-min.y;
	radii.z = max.z-min.z;
	radii *= 0.5f;

	// Shift transform so box center at origin
	Point3F tmpPoint;
	tmpPoint.x = min.x + radii.x;
	tmpPoint.y = min.y + radii.y;
	tmpPoint.z = min.z + radii.z;
	mat.preTranslate(tmpPoint);

	return image->shapeInst->collideBox(collisionDetail,radii,&mat,list,false);
#else
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
#endif
}


//----------------------------------------------------------------------------

bool PlayerCollisionImage::checkPlayer(PlayerCollisionImage* image,
	CollisionSurfaceList* list)
{
	// Bounding sphere to bounding sphere for now.
	TMat3F mat;
	Parent::buildImageTransform(image,&mat);

	Point3F center2;
	m_mul(image->sphere.center,mat,&center2);
	float dist = m_distf(sphere.center,center2);
	if (dist < sphere.radius + image->sphere.radius) {
		CollisionSurface surface;
		surface.time = 0.0f;
		surface.position = sphere.center;
		surface.normal = sphere.center - center2;
		surface.normal *= 1.0 / dist;
		surface.distance = dist;
		surface.material = surface.surface = 0;
		list->push_back(surface);
		return true;
	}
	return false;
}	

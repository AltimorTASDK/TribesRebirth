
#include <ts.h>
#include "simCollisionImages.h"
#include "grdFile.h"
#include "grdCollision.h"
#include "itrInstance.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionTerrImage::SimCollisionTerrImage()
{
	type = Terrain;
	priority = Terrain;
	moveable = false;
	// Hole collision only applies to the line intersections
	editor = false;
}

bool SimCollisionTerrImage::collide(SimCollisionImage* image,CollisionSurfaceList* cList)
{
	if (!gridFile)
		return false;

	// Transform image argument into local space.
	// Collision surfaces are returned in this image space.
	TMat3F mat;
	buildImageTransform(image,&mat);

	switch (image->getType()) {
		case Sphere: {
			SimCollisionSphereImage* sphere = static_cast<SimCollisionSphereImage*>(image);
			Point3F center;
			m_mul(sphere->center,mat,&center);
			cList->increment();
			CollisionSurface& info = cList->last();
			if (gridFile->getSurfaceInfo(center,&info)) {
				float dist = m_dot(center,info.normal) -
                         m_dot(info.position,info.normal);
                        // terrain planes are 2-sided...
                        dist = fabs(dist);
				if (dist < sphere->radius) {
					info.distance = dist;
					return true;
				}
			}
			cList->clear();
			break;
		}
		case Line: {
			SimCollisionLineImage* line = static_cast<SimCollisionLineImage*>(image);
			GridCollision collision(gridFile,&mat);
			if (collision.collide(line->start,line->end,false,editor)) {
				cList->push_back(collision.surface);
				return true;
			}
			break;
		}
		case Tube: {
			SimCollisionTubeImage* tube = static_cast<SimCollisionTubeImage*>(image);
			GridCollision collision(gridFile,&mat);
			if (collision.collide(tube->start,tube->end,tube->radius)) {
				cList->push_back(collision.surface);
				return true;
          }
			break;
		}
		
		case Box: {
         // BJW missing implementation

         break;
      }
      
		default:
			AssertFatal(0,"SimTerrain:CollisionImage:collide: Unrecognized type");
			break;
	}
	return false;
}	

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionItrImage::SimCollisionItrImage()
{
	type = Interior;
	priority = Interior;
	moveable = false;
}


bool SimCollisionItrImage::collide(SimCollisionImage* image,CollisionSurfaceList* cList)
{
	// Transform image argument into local space.
	// Collision surfaces are returned in this image space.
	TMat3F mat;
	buildImageTransform(image,&mat);
	collision.geometry = instance->getHighestGeometry();
	collision.materialList = instance->getMaterialList();

	switch (image->getType()) {
		case Sphere: {
			SimCollisionSphereImage* sphere = static_cast<SimCollisionSphereImage*>(image);
			collision.transform = &mat;
			collision.collisionList = cList;
			if (collision.collide(sphere->center,sphere->radius))
				return true;
			break;
		}
		case Line: {
			SimCollisionLineImage* line = static_cast<SimCollisionLineImage*>(image);
			collision.transform = &mat;
			collision.collisionList = cList;
			if (collision.collide(line->start,line->end))
				return true;
			break;
		}
		case Tube: {
			SimCollisionTubeImage* tube = static_cast<SimCollisionTubeImage*>(image);
			collision.transform = &mat;
			collision.collisionList = cList;
			if (collision.collide(tube->radius,tube->start,tube->end))
				return true;
			break;
		}
		
		case Box: {
         SimCollisionBoxImage* box = static_cast<SimCollisionBoxImage*>(image);
			collision.transform = &mat;
			collision.collisionList = cList;
			if (collision.collide(box->box))
				return true;
			break;
      }
		
		case Interior:
			// Should probably do a bounding box check.
			break;
      
		default:
			AssertFatal(0,"SimTerrain:CollisionImage:collide: Unrecognized type");
			break;
	}
	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionTS3Image::SimCollisionTS3Image()
{
	type = TSShape;
	priority = TSShape;
	moveable = true;
	collisionLevel=CollideFaces;
	collisionDetail=-1;
	shapeInst=0;
}

bool SimCollisionTS3Image::collide(SimCollisionImage* image, CollisionSurfaceList *list )
{
	AssertFatal(collisionDetail!=-1,
		"SimCollisionTS3Image::collide: collisionDetail not set or set improperly.");
	AssertFatal(shapeInst,
		"SimCollisionTS3Image::collide:  shape not set or set improperly.");

	// Test collision of image argument against this image.
	// Collision surfaces are returned in this image space.
	// Account for fRootDeltaTransform here
	TMat3F & rootDelta = shapeInst->fRootDeltaTransform, mat;
	if (rootDelta.flags &
			(TMat3F::Matrix_HasRotation|TMat3F::Matrix_HasTranslation|TMat3F::Matrix_HasScale))
	{
		TMat3F middleMat;
		buildImageTransform(image,&middleMat);
		TMat3F rootInv = rootDelta;
		rootInv.inverse();
		m_mul(middleMat,rootInv,&mat);

		// supply matrix to return to world and local coordinates
		middleMat = list->tWorld;
		m_mul(rootDelta,middleMat,&list->tWorld);
		middleMat = list->tLocal;
		m_mul(rootDelta,middleMat,&list->tLocal);
	}
	else
		buildImageTransform(image,&mat);

	switch (image->getType()) {
		case Sphere:
		{
			SimCollisionSphereImage* si = static_cast<SimCollisionSphereImage*>(image);
			return checkSphere(si,&mat,list);
		}
		case Box:
		{
			SimCollisionBoxImage* bi = static_cast<SimCollisionBoxImage*>(image);
			return checkBox(bi,&mat,list);
		}
		case Tube:
		{
			SimCollisionTubeImage* ti = static_cast<SimCollisionTubeImage*>(image);
			return checkTube(ti,&mat,list);
		}
		case Line:
		{
			SimCollisionLineImage* li = static_cast<SimCollisionLineImage*>(image);
			return checkLine(li,&mat,list);
		}
		case Terrain:
		{
			SimCollisionTerrImage* ti = static_cast<SimCollisionTerrImage*>(image);
			return checkTerrain(ti,&mat,list);
		}
		case Interior:
		{
			SimCollisionItrImage* ii = static_cast<SimCollisionItrImage*>(image);
			return checkInterior(ii,&mat,list);
		}
		case TSShape:
		{
			SimCollisionTS3Image* si = static_cast<SimCollisionTS3Image*>(image);
			return checkOtherShape(si,&mat,list);
		}
		default:
			AssertFatal(0,"SimCollisionTS3Image:collide: Unrecognized type");
			break;
	}
	return false;
}

bool SimCollisionTS3Image::checkSphere(SimCollisionSphereImage * si,
                                               TMat3F * trans,
                                               CollisionSurfaceList * list)
{
	return shapeInst->collideSphere(collisionDetail,si->center,si->radius,trans,list,
	                                collisionLevel==CollideFaces);
}

bool SimCollisionTS3Image::checkBox(SimCollisionBoxImage * bi,
                                            TMat3F * trans,
                                            CollisionSurfaceList * list)
{
	// note:  no face level check

	// shape-box collision routine wants box centered at origin and wants radii not min/max
	Point3F radii;
	Point3F & min = bi->box.fMin, & max = bi->box.fMax;
	radii.x = max.x-min.x;
	radii.y = max.y-min.y;
	radii.z = max.z-min.z;
	radii *= 0.5f;

	// shift transform so box center at origin
	Point3F tmpPoint;
	tmpPoint.x = min.x + radii.x;
	tmpPoint.y = min.y + radii.y;
	tmpPoint.z = min.z + radii.z;
	trans->preTranslate(tmpPoint);

	return shapeInst->collideBox(collisionDetail,radii,trans,list,false);
}



bool SimCollisionTS3Image::checkTube(SimCollisionTubeImage * ti,
                                             TMat3F * trans,
                                             CollisionSurfaceList * list)
{
	return shapeInst->collideTube(collisionDetail,ti->start,ti->end,ti->radius,trans,list,
	                              collisionLevel==CollideFaces);
}



bool SimCollisionTS3Image::checkLine(SimCollisionLineImage * li,
                                             TMat3F * trans,
                                             CollisionSurfaceList * list)
{
	return shapeInst->collideLine(collisionDetail,li->start,li->end,trans,list,
	                              collisionLevel==CollideFaces);
}



bool SimCollisionTS3Image::checkTerrain(SimCollisionTerrImage * ti,
                                                TMat3F * trans,
                                                CollisionSurfaceList * list)
{
	// note:  no face level check

	TMat3F invTrans = *trans;
	invTrans.inverse();

	Point3F pos;
	m_mul(shapeInst->getShape().fCenter,invTrans,&pos);

	list->increment();
	CollisionSurface & info = list->last();;
	ti->gridFile->getSurfaceInfo(pos,&info); // pos: Point3F -> Point2F

	// put info.position into shape space
	Point3F tmpP = info.position;
	m_mul(tmpP,*trans,&info.position); 

	float height = bbox.fMin.z-info.position.z;
	if (height>=0.0f)
	{
		list->decrement();
		return false;
	}

	info.distance = -height; // distance of bbox bottom from ground
	// info.material,surface,part correspond to terrain...should this be so?
	
	return true;
}


bool SimCollisionTS3Image::checkInterior(SimCollisionItrImage * ii,
                                                 TMat3F * trans,
                                                 CollisionSurfaceList * list)
{
	// note:  no face level check

 	TMat3F invTrans = *trans;
	invTrans.inverse();

	ITRCollision & itrColl = ii->collision;
	itrColl.geometry = ii->instance->getGeometry();
	itrColl.materialList = ii->instance->getMaterialList();
	itrColl.transform = &invTrans;
	itrColl.collisionList = list;

	if (!itrColl.collide(bbox))
		return false;

	// currently leave collision info in itr space rather than shape space
	int i;
	for (i=0;i<list->size();i++)
	{
		CollisionSurface & cs = (*list)[i];
		Point3F pos = cs.position;
		m_mul(pos,*trans,&cs.position);
		Point3F n = cs.normal;
		m_mul(n,(RMat3F&) *trans,&cs.normal);
		// material, surface, part refers to itr...should this be so?
	}
	return true;
}



bool SimCollisionTS3Image::checkOtherShape(SimCollisionTS3Image * si,
                                                   TMat3F * trans,
                                                   CollisionSurfaceList * list)
{
	// note:  no face level check

	TSShapeInstance *otherSI = si->shapeInst;

	// account for other shapes fRootDeltaTransform
	TMat3F trans2;
	m_mul(otherSI->fRootDeltaTransform,*trans,&trans2);

	// first do quick sphere check
	Point3F otherCenter;
	m_mul(otherSI->getShape().fCenter,trans2,&otherCenter);
	otherCenter -= shapeInst->getShape().fCenter;
	float d2 = m_dot(otherCenter,otherCenter);
	float otherRad = otherSI->getShape().fRadius*0.5f;
	float thisRad  = shapeInst->getShape().fRadius*0.5f;
	if (d2> otherRad*otherRad + thisRad*thisRad)
		return false;


	return shapeInst->collideShape(collisionDetail,si->collisionDetail,*otherSI,&trans2,list,false);
}


//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <simCollision.h>
#include <m_coll.h>


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionInfo::SimCollisionInfo()
{
   time = 0.0;
	object = NULL;
   image = NULL;
   imageIndex = 0;
}

void SimCollisionInfo::transform(TMat3F &mat)
{
	surfaces.transform(mat);
}


//----------------------------------------------------------------------------

void SimCollisionInfoList::transform(TMat3F &mat)
{
	for (iterator itr = begin(); itr != end(); itr++)
		(*itr).transform(mat);
}


SimCollisionInfoList::~SimCollisionInfoList()
{
	for (iterator itr = begin(); itr != end(); itr++)
		(*itr).~SimCollisionInfo();
}


void SimCollisionInfoList::clear ( void )
{
	for (iterator itr = begin(); itr != end(); itr++)
		(*itr).~SimCollisionInfo();
      
   Base::clear();
}




//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionImage::SimCollisionImage()
{
	trigger = false;
	moveable = false;
}

SimCollisionImage::~SimCollisionImage()
{
}	

void SimCollisionImage::buildImageTransform(SimCollisionImage* image,TMat3F* mat)
{
	// Build transform that goes from image argument space into
	// this image's space.
	TMat3F mm = transform;
	mm.inverse();
	m_mul(image->transform,mm,mat);
}


//----------------------------------------------------------------------------

bool SimCollisionImage::test(SimCollisionImage* image1,SimCollisionImage* image2,
	CollisionSurfaceList* list)
{
	// Image1 is the one that is moving.
	if (image1->priority >= image2->priority) {
		// Collisions calculated in image1 space
		list->tWorld = image1->transform;
		list->tLocal.identity();
		return image1->collide(image2,list);
	}
	else {
		// Collisions calculated in image2 space
		list->tWorld = image2->transform;
		image1->buildImageTransform(image2,&list->tLocal);
		return image2->collide(image1,list);
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionSphereImage::SimCollisionSphereImage()
{
	type = Sphere;
	priority = Sphere;
}

bool SimCollisionSphereImage::collide(SimCollisionImage* image,
	CollisionSurfaceList* list)
{
   // Test collision of image argument against this image.
   // Collision surfaces are returned in this image space.
   TMat3F mat;
   buildImageTransform(image,&mat);

   switch (image->getType())
   {
      case Sphere:
      {
         SimCollisionSphereImage* ip = static_cast<SimCollisionSphereImage*>(image);
         Point3F center2;
         m_mul(ip->center,mat,&center2);
         Vector3F offset = center - center2;
         float distSq = m_dot(offset, offset);
         float radSq = radius + ip->radius;
         radSq *= radSq;
         if (distSq <= radSq)
         {
            CollisionSurface surface;
            surface.time = 0.0f;
            surface.position = center;
            surface.distance = m_sqrt(distSq);
            surface.normal = offset;
            if (surface.distance<0.001f)
               surface.normal.set(0,0,1);
            else
               surface.normal *= 1.0f / surface.distance;
            surface.material = surface.surface = 0;
            list->push_back(surface);
            return true;
         }
         break;
      }

      default:
         AssertFatal(0,"SimCollisionSphereImage:collide: Unrecognized type");
         break;
   }
   return false;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionBoxImage::SimCollisionBoxImage()
{
	type = Box;
	priority = Box;
}

bool SimCollisionBoxImage::collide(SimCollisionImage* image,
		CollisionSurfaceList *list )
{
	TMat3F mat;
	buildImageTransform(image,&mat);

	// Box is axis aligned in it's source space
	switch (image->getType()) {
		case Sphere: {
         SimCollisionSphereImage *sphere = static_cast<SimCollisionSphereImage*>(image);
         RealF hit;
         
         Point3F tcenter;
         m_mul(sphere->center,mat,&tcenter);
         Point3F boxCenter = box.fMin + box.fMax;
         boxCenter *= 0.5f;
         tcenter -= boxCenter;

         if(m_sphereAABox(tcenter, (sphere->radius * sphere->radius), box.fMin, box.fMax, hit))
         {
            CollisionSurface surface;
				surface.time = 0;   // not sure how to fill in any of these values
				//surface.position = ;
				//surface.distance = ;
				//surface.normal = ;
				surface.material = surface.surface = 0;
				list->push_back(surface);
            return true;
         }
         break;
      }
      
		case Box: {
         // BJW missing implementation

         break;
      }
      
		default:
			AssertFatal(0,"SimCollisionBoxImage:collide: Unrecognized type");
			break;
	}
	return false;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionLineImage::SimCollisionLineImage()
{
	type = Line;
	priority = Line;
}

bool SimCollisionLineImage::collide(SimCollisionImage* image,
		CollisionSurfaceList *list)
{
	TMat3F mat;
	buildImageTransform(image,&mat);

	switch (image->getType()) {
		case Sphere:
      {
			SimCollisionSphereImage *sphere = 
            static_cast<SimCollisionSphereImage*>(image);
         const float &radius = sphere->radius;
         Point3F center;
			m_mul(sphere->center, mat, &center);
         Point3F V = end - start;
         float vlenSq = m_dot(V,V);
         if (vlenSq < 0.01f)
            // lines interect points with probability zero
            break;
         V *= 1.0f/m_sqrt(vlenSq);
         Point3F EO = center;
         EO -= start;
         const float v = m_dot(EO, V);
         const float disc = radius*radius - (m_dot(EO, EO) - v*v);
         if (disc >= 0.0f) 
         {
            const float d = m_sqrt(disc);
				CollisionSurface surface;
				surface.time = 0.0f;
				surface.position = V;
            surface.position *= (v - d);
            surface.position += start;
				surface.distance = m_distf(surface.position, start);
				surface.normal = surface.position;
				surface.normal -= center;
				surface.material = surface.surface = 0;
				list->push_back(surface);
            return true;
         }                  
         break;
		}

		case Box: {
         TMat3F imat = mat;
         Point3F a,b;
         RealF hit;
         
         SimCollisionBoxImage *box = 
            static_cast<SimCollisionBoxImage*>(image);
         
         imat.inverse();
         
         m_mul(start, imat, &a);
         m_mul(end, imat, &b);

         if(m_lineAABox(a, b, box->box.fMin, box->box.fMax, hit))
         {
            CollisionSurface surface;
				surface.time = hit;
            surface.position.x = start.x + (hit * (end.x - start.x));
            surface.position.y = start.y + (hit * (end.y - start.y));
            surface.position.z = start.z + (hit * (end.z - start.z));
            surface.distance = m_distf(surface.position, start);
            surface.normal.set(0, 0, 0);  // incomplete and wrong
            surface.material = surface.surface = 0;
				list->push_back(surface);
            return true;
         }

         break;
      }
      
		default:
			AssertFatal(0,"SimCollisionLineImage:collide: Unrecognized type");
			break;
	}
	return false;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollisionTubeImage::SimCollisionTubeImage()
{
	type = Tube;
	priority = Tube;
}

bool SimCollisionTubeImage::collide(SimCollisionImage* image,
		CollisionSurfaceList* )
{
	switch (image->getType()) {
		case Sphere:
			break;
		case Box:
			break;
		case Tube:
			break;
		default:
			AssertFatal(0,"SimCollisionTubeImage:collide: Unrecognized type");
			break;
	}
	return false;
}


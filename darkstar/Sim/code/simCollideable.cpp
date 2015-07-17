//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <simCollideable.h>


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimCollideable::SimCollideable()
{
	boundingBox.fMin.set(-0.5f,-0.5f,-0.5f);
	boundingBox.fMax.set(+0.5f,+0.5f,+0.5f);
	transform.identity();
	collisionMask = -1;
   boundingBoxRotate = false;
}

SimCollideable::~SimCollideable()
{
}


//----------------------------------------------------------------------------

bool SimCollideable::getIntersectionList(Point3F& pos,SimContainerList* list)
{
	// Build bounding box of new position in world space
	SimContainerQuery query;
	query.id = getId();
	query.type = getType();
	query.mask = collisionMask;
	query.detail = SimContainerQuery::DefaultDetail;

	TMat3F mat = transform;
	mat.p = pos;
	mat.flags |= TMat3F::Matrix_HasTranslation;
	buildBoundingBox(mat,&query.box);

	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	root->findIntersections(query,list);
	return !list->empty();
}


//----------------------------------------------------------------------------

void SimCollideable::buildBoundingBox(const TMat3F& mat,Box3F* box)
{
	// Transform our local bounding box into a bounding box
	// axis aligned in world space.

   if (boundingBoxRotate)  
      m_mul(boundingBox, mat, box);
   else {
      // if an object's bound box is defined by a radius then it doesn't
      // change under rotation, hence we simply translate it
      *box = boundingBox;
      box->fMin += mat.p;
      box->fMax += mat.p;
   }
}


//----------------------------------------------------------------------------

SimCollisionImage* SimCollideable::getImage(SimObject* object)
{
   // Moving objects can have only one image
	SimCollisionImageQuery query;
	//
	// Query position
	// Query detail level
	//
	if (object->processQuery(&query))
		return query.image[0];
	return 0;
}


//----------------------------------------------------------------------------

bool SimCollideable::movePosition(TMat3F& newPos,SimCollisionInfoList* cList)
{
	// Bin search start to end.
	// Bin search includes container objects?

	// Return all container collision + bin search of
	// closest objects.
	// Flag for bin search on container objects?
	return setPosition(newPos,cList);
}


//----------------------------------------------------------------------------

bool SimCollideable::setPosition(TMat3F newPos,SimCollisionInfoList* cList,bool force)
{
	if (!force) 
   {
		// Make sure we have an image first, since we're moving we get only one
		SimCollisionImage* image1 = getImage(this);
		image1->transform = newPos;
		if (image1) 
      {
			// Get list of objects that intersect our bounding box
			SimContainerList iList;
			getIntersectionList(newPos.p,&iList);

			// Query & test each object's collision image
			cList->increment ();
			for (SimContainerList::iterator itr = iList.begin();
					itr != iList.end(); itr++) 
         {
            // Cycle through the object's images, testing against our image
            SimCollisionImageQuery query;
            if ((*itr)->processQuery(&query))
               for (int i = 0; i < query.count; i++)
               {
      				SimCollisionImage *image2 = query.image[i];
   					// Move alarms into a seperate list?
			         SimCollisionInfo & info = cList->last ();
   					info.surfaces.clear();
   					if (SimCollisionImage::test(image1,image2,&info.surfaces)) 
                  {
   						info.time = 1.0f;
   						info.object = *itr;
							info.image = image2;
                     info.imageIndex = i;
   						// cList->push_back(info);
                     cList->increment ();
                  }
					}
			}
         cList->decrement ();
		}
	}

	if (force || cList->empty()) {
		// Post notification to objects in alarm list.
		transform = newPos;
		Box3F box;
		buildBoundingBox(transform,&box);
		setBoundingBox(box);
		return true;
	}

	// Post event to both objects?
	return false;
}


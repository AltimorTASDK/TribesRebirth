//--------------------------------------------------------------------------- 

// For visibility, need update that only adds objects from
// parent containers that actually intersect the container.
// Ground?

//--------------------------------------------------------------------------- 

#include <netGhostManager.h>
#include "simContainer.h"
#include "g_surfac.h"
#include "simGame.h"
#include "simTerrain.h"
#include "gOGLSfc.h"
#include <ts.h>
//----------------------------------------------------------------------------
// SimContainer
//----------------------------------------------------------------------------

int  SimContainer::lastTag;

//----------------------------------------------------------------------------

SimContainer::SimContainer()
{
   type = SimContainerObjectType;
	container = 0;
	containerMask = 0;
	boundingBox.fMin.set(.0f,.0f,.0f);
	boundingBox.fMax.set(.0f,.0f,.0f);
	contextTag = -1;
	database = 0;
}

SimContainer::~SimContainer()
{
	AssertFatal(container == 0,
		"SimContainer::~SimContainer: Should remove from container");
	delete database;
}

float
SimContainer::getScopeVisibleDist() const
{
   SimContainer* ncThis = const_cast<SimContainer*>(this);

   if (ncThis->getContainer() != NULL) {
      return ncThis->getContainer()->getScopeVisibleDist();
   }

   return 1.0e6f;
}

//----------------------------------------------------------------------------

bool SimContainer::installDatabase(SimContainerDatabase* db)
{
	AssertFatal(!database || !db,
			"SimContainer::installDatabase: Already have a database");
	if (container) {
		SimContainer* ptr = container;
		ptr->removeObject(this);
		database = db;
		ptr->addObject(this);
	}
	else
		database = db;
	return true;
}

void SimContainer::uninstallDatabase()
{
   // We need to make sure that when the database starts stuffing the objects into
   //  the parent, that they don't come right back...
   //
   UInt32 temp = containerMask;
   containerMask = 0;

	SimContainer* parent = getContainer();
	if (database)
		database->removeAll(parent);

   delete database;
   database = NULL;
   
   containerMask = temp;
}

SimContainerDatabase* SimContainer::getDatabase()
{
   return database;
}

//----------------------------------------------------------------------------

void SimContainer::setBoundingBox(const Box3F& box)
{
	boundingBox = box;
	if (container)
		container->updateObject(this);
}


//----------------------------------------------------------------------------

bool SimContainer::isIntersecting(const SimContainerQuery& query)
{
	// Test the query mask agains the object.  Intersect only
	// objects request by the query.
	return query.id != id && query.mask & getType() && 
		isIntersecting(query.box);
}	

bool SimContainer::isContained(const SimContainerQuery& query)
{
	// Is the query box contained by this container?
	// Use the objects mask against the query.  The container
	// controls which objects it wants to contain.
	return query.id != id && getContainerMask() & query.type &&
		isContained(query.box);
}	

bool SimContainer::isIntersecting(const Box3F& bbox)
{
	return boundingBox.fMax.x >= bbox.fMin.x &&
		boundingBox.fMin.x <= bbox.fMax.x &&
		boundingBox.fMax.y >= bbox.fMin.y &&
		boundingBox.fMin.y <= bbox.fMax.y &&
		boundingBox.fMax.z >= bbox.fMin.z &&
		boundingBox.fMin.z <= bbox.fMax.z;
}	

bool SimContainer::isContained(const Box3F& box)
{
	// Is box contained by this container?
	return boundingBox.fMax.x >= box.fMax.x &&
		boundingBox.fMin.x <= box.fMin.x &&
		boundingBox.fMax.y >= box.fMax.y &&
		boundingBox.fMin.y <= box.fMin.y &&
		boundingBox.fMax.z >= box.fMax.z &&
		boundingBox.fMin.z <= box.fMin.z;
}

//--------------------------------------------------------------------------- 

bool SimContainerQuery::isContained(SimContainer* object) const
{
	// Is the container contained by this query box?
	if (id != object->getId() && mask & object->getType()) {
		const Box3F& boundingBox = object->getBoundingBox();
		return box.fMax.x >= boundingBox.fMax.x &&
			box.fMin.x <= boundingBox.fMin.x &&
			box.fMax.y >= boundingBox.fMax.y &&
			box.fMin.y <= boundingBox.fMin.y &&
			box.fMax.z >= boundingBox.fMax.z &&
			box.fMin.z <= boundingBox.fMin.z;
	}
	return false;
}


//----------------------------------------------------------------------------
// Container data base methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool SimContainer::addObject(SimContainer *object)
{
	AssertFatal(isOpen(),
		"SimContainer::addObject: Container is not open");
	AssertFatal(isContained(object),
		"SimContainer::addObject: Object is not contained");

	// Pull it out of it's last container
	if (object->getContainer())
		object->getContainer()->removeObject(object);

	// See object fits in a sub-container
	SimContainerQuery query;
	query.id = object->getId();
	query.type = object->getType();
	query.mask = object->getContainerMask();
	query.box = object->getBoundingBox();
	query.detail = SimContainerQuery::SystemDetail;

	if (SimContainer* con = database->findOpenContainer(query))
		return con->addObject(object);

	// Add to the internal database
	deleteNotify(object);
	object->container = this;
	database->add(object);
	enterContainer(object);

	// If the object is an open container, we may have
	// to move some our current contents into it.
	if (object->isOpen()) {
		SimContainerQuery query;
		query.id = object->getId();
		query.type = object->getType();
		query.mask = object->getContainerMask();
		query.box = object->getBoundingBox();
	   query.detail = SimContainerQuery::SystemDetail;

		SimContainerList list;
		if (database->findContained(query,&list)) {
			for (SimContainerList::iterator ptr = list.begin();
					ptr != list.end(); ptr++)
				// FindContained doesn't test the query mask
				if (query.mask & (*ptr)->getType())
					object->addObject(*ptr);
		}
	}
	return true;
}


//--------------------------------------------------------------------------- 

bool SimContainer::removeObject(SimContainer *object)
{
	AssertFatal(object->container == this,
		"SimContainer::removeObject: "
		"Object does not belong to this container");
	leaveContainer(object);
	if (database->remove(object)) {
		object->container = 0;
		clearNotify(object);
		return true;
	}
	return false;
}


//--------------------------------------------------------------------------- 

bool SimContainer::updateObject(SimContainer* object)
{
	// This method gets called when an object's bounding
	// box has changed.
	AssertFatal(object->container == this,
		"SimContainer::updateObject: "
		"Object does not belong to this container");
	if (isContained(object))
		database->update(object);
	else {
		// Back up untill we find the first parent
		// that will contain the object.
		SimContainer* con = object->container;
		while (con && !con->isContained(object))
			con = con->container;
		AssertFatal(con,"SimDefaultContainer::updateObject: "
			"No parent container");
		con->addObject(object);
	}
	return true;
}

//----------------------------------------------------------------------------
// The enter and leave methods are called on a container whenever a
// containable enter or leaves the container's "scope".  This is
// different that entering or leaving the database.  The containable
// is still in "scope" if it's belongs to a sub-container.

void SimContainer::enterContainer(SimContainer* obj)
{
	// Dervide class should call their parents first
	if (container)
		container->enterContainer(obj);
}

void SimContainer::leaveContainer(SimContainer* obj)
{
	// Derived classes should call their parent after they are done
	if (container)
		container->leaveContainer(obj);
}


//----------------------------------------------------------------------------

SimContainer* SimContainer::findOpenContainer(SimContainer* object)
{
	AssertFatal(isContained(object),
		"SimContainer::findOpenContainer: Object is not contained");
	if (isOpen()) {
		// Find the smallest open container that contains the object.
		SimContainerQuery query;
		query.id = object->getId();
		query.type = object->getType();
		query.mask = object->getContainerMask();
		query.box = object->getBoundingBox();
		query.detail = SimContainerQuery::SystemDetail;

		//
		SimContainer *con = this, *last;
		do {
			last = con;
			con = last->database->findOpenContainer(query);
		} while (con);
		return last;
	}
	return 0;
}


//--------------------------------------------------------------------------- 

bool SimContainer::findIntersections(SimContainer* object,
	SimContainerList* list,UInt32 detail)
{
	if (isOpen()) {
		// Find all the intersections in this container and
		// all sub-containers.
		SimContainerQuery query;
		query.id = object->getId();
		query.type = object->getType();
		query.mask = object->getContainerMask();
		query.box = object->getBoundingBox();
		query.detail = detail;
		return findIntersections(query,list);
	}
	return false;
}

bool SimContainer::findIntersections(const SimContainerQuery& query,
	SimContainerList* list)
{
	if (isOpen() && (containerMask & query.mask)) {
		// Find all the intersections in this container and
		// Query local data base then recurse into sub-containers
		int index = list->size();
		database->findIntersections(query,list);

		// Uses index instead of iterator because sub-containers may
		// change list size/address.
		int end = list->size();
		for (; index < end; index++) {
			if ((*list)[index]->isOpen()) {
				SimContainer* cnt = (*list)[index];
				if (query.id == cnt->getId() || !(query.mask & cnt->getType())) {
					(*list)[index] = (*list)[end - 1];
	            (*list)[end-1] = (*list)[list->size() - 1];
					list->decrement();
					index--;
					end--;
				}
				cnt->findIntersections(query,list);
			}
		}
		return !list->empty();
	}
	return false;
}


static bool clip(LineSeg3F *seg, PlaneF *p, int whichSide)
{
   int s1 = p->whichSide(seg->start);
   int s2 = p->whichSide(seg->end);

   if(s1 == whichSide && s2 == whichSide)
      return true;
   if(s1 != whichSide && s2 != whichSide)
      return false;
   if(s1 + s2 == whichSide)
      return true;
   Point3F xPoint;
   p->intersect(*seg, &xPoint);
   if(s1 == whichSide)
      seg->end = xPoint;
   else
      seg->start = xPoint;
   return true;
}

//--------------------------------------------------------------------------- 
// This definitly could be optimized, but this is at least a first
// pass at it.  Should probably think about moving this test into
// the local databases for more efficient local processing.
//
bool SimContainer::findLOS(const SimContainerQuery& query,SimCollisionInfo* info,
	SimCollisionImageQuery::Detail collisionDetail)
{
	if (!isOpen())
		return false;

	// Find the first object intersecting the line from query.box.fMin
	// to query.box.fMax.
	SimContainerQuery q = query;
	q.box.fMin.setMin(query.box.fMax);
	q.box.fMax.setMax(query.box.fMin);

	// Find all the objects within possible range.  This will get changed
	// to line query when that's supported.
	SimContainerList list;
	findIntersections(q,&list);

	// Query and test the images
	SimCollisionImageQuery imageQuery;
	imageQuery.detail = collisionDetail;

	SimCollisionLineImage image;
	image.transform.identity();
	image.start = query.box.fMin;
	image.end = query.box.fMax;

	float bestDistance = 1.0e30f;
	CollisionSurfaceList surfaces;
	info->object = 0;

	for (SimContainerList::iterator itr = list.begin(); itr != list.end(); itr++)
   {
		// clip the line of sight to the bounding box of the container
		LineSeg3F l(image.start, image.end);
      PlaneF p;
      const Box3F &box = (*itr)->getBoundingBox();

      p.setXY(box.fMin.z);
      if(!clip(&l, &p, 1))
         continue;
      p.setXY(box.fMax.z);
      if(!clip(&l, &p, -1))
         continue;
      p.setYZ(box.fMin.x);
      if(!clip(&l, &p, 1))
         continue;
      p.setYZ(box.fMax.x);
      if(!clip(&l, &p, -1))
         continue;
      p.setXZ(box.fMin.y);
      if(!clip(&l, &p, 1))
         continue;
      p.setXZ(box.fMax.y);
      if(!clip(&l, &p, -1))
         continue;

		if ((*itr)->processQuery(&imageQuery)) 
      {
			// Keep the best surface, closest point to the start of the line.
		   for (int i = 0; i < imageQuery.count; i++)
         {
				SimCollisionImage* im = imageQuery.image[i];
         	if( ( !im->trigger || ( collisionDetail == SimCollisionImageQuery::Editor ) ) && 
         			SimCollisionImage::test(&image,im,&surfaces)) {
   				Point3F point;
   				m_mul(surfaces[0].position,surfaces.tWorld,&point);
   				float dist = m_distf(query.box.fMin,point);
   				if (dist < bestDistance) {
   					bestDistance = dist;
   					info->object = *itr;
                  info->image = im;
                  info->imageIndex = i;
   					info->surfaces.clear();
   					info->surfaces.push_back(surfaces[0]);
                  info->surfaces.tWorld = surfaces.tWorld;
                  info->surfaces.tLocal = surfaces.tLocal;
   					image.end = point;
   				}
   			}
            surfaces.clear();
         }            
   	}
   }
	return info->object != 0;
}	


//--------------------------------------------------------------------------- 

bool SimContainer::findLOS(const SimContainerQuery& query,SimCollisionInfoList* infoList,
	SimCollisionImageQuery::Detail collisionDetail)
{
	if (!isOpen())
		return false;

	// Find the all objects intersecting the line from query.box.fMin
	// to query.box.fMax.
	SimContainerQuery q = query;
	q.box.fMin.setMin(query.box.fMax);
	q.box.fMax.setMax(query.box.fMin);

	// Find all the objects within possible range.  This will get changed
	// to line query when that's supported.
	SimContainerList list;
	findIntersections(q,&list);

	// Query and test the images
	SimCollisionImageQuery imageQuery;
	imageQuery.detail = collisionDetail;

	SimCollisionLineImage image;
	image.transform.identity();
	image.start = query.box.fMin;
	image.end = query.box.fMax;

	CollisionSurfaceList surfaces;
   
   infoList->clear();

	for (SimContainerList::iterator itr = list.begin(); itr != list.end(); itr++)
		if ((*itr)->processQuery(&imageQuery)) 
      {
			// Keep the best surface, closest point to the start of the line.
		   for (int i = 0; i < imageQuery.count; i++)
         {
         	if (SimCollisionImage::test(&image,imageQuery.image[i],&surfaces)) {
               infoList->increment();
               SimCollisionInfo & info = infoList->last();
					info.object = *itr;
               info.image = imageQuery.image[i];
               info.imageIndex = i;
					info.surfaces.clear();
					info.surfaces.push_back(surfaces[0]);
               info.surfaces.tWorld = surfaces.tWorld;
               info.surfaces.tLocal = surfaces.tLocal;
               // infoList->push_back(info);
   			}
            surfaces.clear();
         }            
   	}

	return infoList->size() != 0;
}	


//--------------------------------------------------------------------------- 

bool SimContainer::render(SimContainerRenderContext& rc)
{
	contextTag = rc.getTag();
	if (isOpen()) {
		// Depth first traversal based on camera pos.
		if (rc.currentState().containCamera) {
			SimContainer* cnt = database->findOpenContainer(rc);
			if (cnt && rc.getTag() != cnt->getTag())
				if (!cnt->render(rc))
					return false;
		}

		// Render ourself
		rc.render(this);

		// Push new state for the sub containers
		SimContainerRenderContext::State ss;
		ss = rc.currentState();
		ss.containCamera = false;
		rc.push(ss);

		// Render contents of our database
		database->render(rc);
		rc.pop();

		// If we contain the camera, we may restrict the
		// rendering of parent containers by returning false.
		if (rc.currentState().containCamera) {
		}
	}
	return true;
}


//----------------------------------------------------------------------------

bool SimContainer::scope(SimContainerScopeContext& sc)
{
	if (isOpen()) {
		contextTag = sc.tag;

		// Depth first traversal based on camera pos.
		if (sc.currentState().containCamera) {
			SimContainer* cnt = database->findOpenContainer(sc);
			if (cnt && sc.getTag() != cnt->getTag())
				if (!cnt->scope(sc))
					return false;
		}
		// Scope ourself
		if (this->isScopeable())
			sc.scopeObject(this);

		// Push new state for the sub containers
		SimContainerScopeContext::State ss;
		ss = sc.currentState();
		ss.containCamera = false;
		sc.push(ss);

		// Scope the contents of our database
		database->scope(sc);
		sc.pop();

		// If we contain the camera, we may restrict the
		// rendering of parent containers by returning false.
		if (sc.currentState().containCamera) {
		}
	}
	return true;
}


//----------------------------------------------------------------------------

void SimContainer::getAlphaBlend(TMat3F &objectPos, ColorF *alphaColor, float *alphaBlend)
{
   if(container)
      container->getAlphaBlend(objectPos, alphaColor, alphaBlend);
   else
   {
      *alphaBlend = 0;
      alphaColor->red = 0;
      alphaColor->green = 0;
      alphaColor->blue = 0;
   }
}

//----------------------------------------------------------------------------

float SimContainer::getHazeValue(TSRenderContext &rc, const Point3F &objPos)
{
	// Don't calulate anything by default, just call our parent
	// if we have one.
	if (container)
		return container->getHazeValue(rc,objPos);
	return 0.0f;
}

float SimContainer::getHazeValue(const Point3F &cameraPos, const Point3F &objPos)
{
   if (container)
   {
      return (container->getHazeValue(cameraPos, objPos));
   }
   return (0.0f);
}



//----------------------------------------------------------------------------
// Event/Query handlers
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void SimContainer::onDeleteNotify(SimObject *object)
{
	// Derived classes may install their own notifies, so
	// we may get called even though we are not open.
	if (isOpen()) {
		SimContainer* cnt = dynamic_cast<SimContainer*>(object);
		if (cnt && cnt->getContainer() == this)
			removeObject(cnt);
	}
   Parent::onDeleteNotify(object);
}


//----------------------------------------------------------------------------

void SimContainer::onRemove()
{
	SimContainer* parent = getContainer();
	if (parent)
		parent->removeObject(this);
	if (database)
		database->removeAll(parent);
   Parent::onRemove();
}


//----------------------------------------------------------------------------
// Container render context
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

SimContainerRenderContext::SimContainerRenderContext()
  : notRenderingContainers(false)
{
	type = -1;
	mask = -1;
	id = -1;
}

SimContainerRenderContext::~SimContainerRenderContext()
{
}	

void SimContainerRenderContext::lock(SimObject* vp,TSRenderContext* renderContext)
{
	tag = SimContainer::getNextTag();
	viewportObject = vp;

	// First container/root must contain the camera
	State ss;
	ss.containCamera = true;
	ss.renderContext = renderContext;
	state.clear();
	push(ss);

	translucentList.clear();
	overlayList.clear();

	// Camera pos in current (world) space.
	TMat3F mat = renderContext->getCamera()->getTWC();
	mat.inverse();
	box.fMin = box.fMax = mat.p;
}

void SimContainerRenderContext::unlock()
{
   Vector<SimRenderImage *>::iterator ptr;
	// *** These items are rendered in the default render state.
	TSRenderContext& tsrc = *currentState().renderContext;
	TSCamera * camera = tsrc.getCamera();
   GFXSurface *sfc = tsrc.getSurface();
   bool isOrtho = tsrc.getPointArray()->getOrtho();

   backgroundList.sort();

   // background, foreground and translucent lists are
   // all 3D sorted, so draw em that way.

   sfc->draw3DBegin();
   sfc->setClipPlanes(camera->getNearDist(),
                      camera->getFarDist());

   OpenGL::Surface* pOGLSfc;
   if ((pOGLSfc = dynamic_cast<OpenGL::Surface*>(sfc)) != NULL) {
      SimManager* pManager = viewportObject->getManager();
      AssertFatal(pManager != NULL, "No manager?");
      SimTerrain* pTerrain = (SimTerrain*)pManager->findObject(SimTerrainId);
      AssertFatal(pTerrain != NULL, "No terrain?");

      float begin = pTerrain->getHazeDistance();
      float end   = pTerrain->getVisibleDistance();

      pOGLSfc->setFogDists(begin, end);
   }

   for(ptr = backgroundList.begin(); ptr != backgroundList.end(); ptr++)
	{
		camera->pushVisibility((*ptr)->visibility);
		if (!isOrtho && (*ptr)->hazeValue>0.0001f)
		{
			sfc->setHazeSource(GFX_HAZE_CONSTANT);
			sfc->setConstantHaze((*ptr)->hazeValue);
			(*ptr)->hazeValue = -1.0f;
		}
		else
			sfc->setHazeSource(GFX_HAZE_NONE);

      (*ptr)->render(tsrc);
		camera->popVisibility();
	}
   
   for(ptr = foregroundList.begin(); ptr != foregroundList.end(); ptr++)
	{
		camera->pushVisibility((*ptr)->visibility);
		if (!isOrtho &&(*ptr)->hazeValue>0.0001f)
		{
			sfc->setHazeSource(GFX_HAZE_CONSTANT);
			sfc->setConstantHaze((*ptr)->hazeValue);
			(*ptr)->hazeValue = -1.0f;
		}
		else
			sfc->setHazeSource(GFX_HAZE_NONE);

      (*ptr)->render(tsrc);
		camera->popVisibility();
	}

   translucentList.sort();
   for(ptr = translucentList.begin(); ptr != translucentList.end(); ptr++)
	{
		camera->pushVisibility((*ptr)->visibility);
		if (!isOrtho && (*ptr)->hazeValue>0.0001f)
		{
			sfc->setHazeSource(GFX_HAZE_CONSTANT);
			sfc->setConstantHaze((*ptr)->hazeValue);
			(*ptr)->hazeValue = -1.0f;
		}
		else
			sfc->setHazeSource(GFX_HAZE_NONE);

      (*ptr)->render(tsrc);

		camera->popVisibility();
	}

   // overlay layer is 2D... draw it that way.

   sfc->draw3DEnd();
   sfc->setZTest(false);

   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   sfc->setShadeSource(GFX_SHADE_NONE);   

   for(ptr = overlayList.begin(); ptr != overlayList.end(); ptr++)
      (*ptr)->render(tsrc);

	pop();
}

void SimContainerRenderContext::render(SimObject* object)
{
	SimRenderQueryImage query;
	query.renderContext = currentState().renderContext;
	query.viewportObject = viewportObject;
	TSCamera * camera = query.renderContext->getCamera();
	int vis = camera->getVisibility();
   query.containerRenderQuery = !notRenderingContainers;

	if (object->processQuery(&query)) {
		// Container context setup for object.
		// Setup GFX.
                // put assert to test image over-run, but damage already done...
                AssertFatal(query.count<=SimRenderQueryImage::MaxImage,
                   "SimContainerRenderContext::render:  too many render images");
		for (int i = 0; i < query.count; i++) {
			SimRenderImage *image = query.image[i];
			image->visibility = vis;
			switch (image->itype) {
				case SimRenderImage::Background:
               backgroundList.push_back(image);
               break;
				case SimRenderImage::Normal:
               foregroundList.push_back(image);
					break;
				case SimRenderImage::Translucent:
					translucentList.push_back(image);
					break;
				case SimRenderImage::Overlay:
					overlayList.push_back(image);
					break;
			}
		}
	}
}


//----------------------------------------------------------------------------
// Scoping context
//----------------------------------------------------------------------------

SimContainerScopeContext::SimContainerScopeContext()
{
	type = -1;
	mask = -1;
	id = -1;
}


//----------------------------------------------------------------------------

void SimContainerScopeContext::lock(Net::GhostManager* gman,
	SimNetObject::CameraInfo* camInfo)
{
	tag = SimContainer::getNextTag();
	ghostManager = gman;

	State ss;
	ss.containCamera = true;
	ss.cameraInfo = camInfo;
	state.clear();
	push(ss);

	box.fMin = box.fMax = camInfo->pos;
}

void SimContainerScopeContext::unlock()
{
}

void SimContainerScopeContext::scopeObject(SimNetObject* obj)
{
	ghostManager->objectInScope(obj);
}


//----------------------------------------------------------------------------
// Default Open container
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

SimDefaultOpenContainer::SimDefaultOpenContainer(SimObjectId _id)
{
	if (_id)
		id = _id;

	Box3F box;
	box.fMin.set(-1.0E30f,-1.0E30f,-1.0E30f);
	box.fMax.set(+1.0E30f,+1.0E30f,+1.0E30f);
	setBoundingBox(box);

	containerMask = Int32(-1);
	installDatabase(&database);
}

SimDefaultOpenContainer::~SimDefaultOpenContainer()
{
	// The container base class will try to delete
	// the installed database, which in our case we
	// don't want.
	installDatabase(0);
}


//----------------------------------------------------------------------------
// Default container data base
//----------------------------------------------------------------------------

SimContainerDatabase::~SimContainerDatabase()
{
}

SimDefaultContainerDatabase::SimDefaultContainerDatabase()
{
}

SimDefaultContainerDatabase::~SimDefaultContainerDatabase()
{
}


//--------------------------------------------------------------------------- 

bool SimDefaultContainerDatabase::add(SimContainer* object)
{
	objectList.push_back(object);
	return true;
}

bool SimDefaultContainerDatabase::remove(SimContainer* object)
{
	SimContainerList::iterator itr = 
		find(objectList.begin(),objectList.end(),object);
	if (itr != objectList.end())
		objectList.erase(itr);
	return true;
}

void SimDefaultContainerDatabase::update(SimContainer* object)
{
	SimContainerQuery query;
	query.id = object->getId();
	query.type = object->getType();
	query.mask = object->getContainerMask();
	query.box = object->getBoundingBox();

	// May have to move the object into a sub-container
	SimContainer* cnt = findOpenContainer(query);
	if (cnt)
		cnt->addObject(object);
}

bool SimDefaultContainerDatabase::findContained(const SimContainerQuery& query,
	SimContainerList* containedList)
{
	// Find all objects contained in the query box
	for (SimContainerList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++)
		if (query.isContained(*itr))
			containedList->push_back(*itr);
	return !containedList->empty();
}

SimContainer* SimDefaultContainerDatabase::findOpenContainer(const SimContainerQuery& query)
{
	// Find the container that containes the query box.
	for (SimContainerList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++ )
		if ((*itr)->isOpen() && (*itr)->isContained(query))
			return *itr;
	return 0;
}

bool SimDefaultContainerDatabase::findIntersections(const SimContainerQuery& query,
	SimContainerList* intersectionList)
{
	// Find all objects that intersect the query box
	for (SimContainerList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++)
		if (((*itr)->isOpen() && (*itr)->isIntersecting(query.box)) ||
				(*itr)->isIntersecting(query))
			intersectionList->push_back(*itr);
	return !intersectionList->empty();
}

bool SimDefaultContainerDatabase::removeAll(SimContainer* parent)
{
	// Remove the objects and place then into our parent.
	if (parent)
	   while (objectList.size()) {
			SimContainer* object = objectList.first();
			// Haven't really looked to see why we should
			// get here with deleted objects in our list,
			// but seems like we do.
			if (object->isDeleted())
				object->getContainer()->removeObject(object);
			else
				parent->addObject(object);
	   }
	else
	   while (objectList.size()) {
			// This is little bizzare, we actually have to call the
			// removeObject on the container.
			SimContainer* object = objectList.first();
			object->getContainer()->removeObject(object);
	   }
   return true;
}      


//--------------------------------------------------------------------------- 

void SimDefaultContainerDatabase::render(SimContainerRenderContext& rc)
{
	// Render local objects.  This method is a little different
	// then the normal database functions in that it recursively
	// operates on nested open containers.
	//
   const int size = objectList.size();
	for (SimContainerList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++ ) {

		// Recurse into local containers we haven't hit already
		if ((*itr)->isOpen()) {
			if (rc.getTag() != (*itr)->getTag())
				(*itr)->render(rc);
		}
		else
			rc.render(*itr);
      
      AssertFatal(size == objectList.size(),
         "object changed containers during render cycle");
	}
}


//----------------------------------------------------------------------------
	
void SimDefaultContainerDatabase::scope(SimContainerScopeContext& sc)
{
	// Scope all the objects and sub-containers.
	// This method is a little different then the normal database 
	// functions in that it recursively operates on nested open
	// containers.
	//
	for (SimContainerList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++ ) {
		if ((*itr)->isOpen()) {
			if (sc.getTag() != (*itr)->getTag())
				(*itr)->scope(sc);
		}
		else
			if ((*itr)->isScopeable())
				sc.scopeObject(*itr);
	}
}	



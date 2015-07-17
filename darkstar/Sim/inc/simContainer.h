//--------------------------------------------------------------------------- 

// World axis aligned bounding box hierarchy.

// Containers provide the following:
//		Spatial searches for objects (bounding box)
//		Rendering of contained objects.
//		Scoping for network objects

// Custom containers:
//		Containers may provide their own spatial data-base.
//		Containers may render the objects they own and control
//		the traversal of other other containers.

//--------------------------------------------------------------------------- 

#ifndef _SIMCONTAINER_H_
#define _SIMCONTAINER_H_

#include <sim.h>
#include <tsorted.h>
#include <simRenderGrp.h>
#include <simNetObject.h>
#include <simCollision.h>
#include <ts_types.h>
#include <tRVector.h>

//--------------------------------------------------------------------------- 

class SimContainer;

typedef RVector<SimContainer*> SimContainerList;


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 
// Structure for container queries.

struct SimContainerQuery
{
	enum Constants {
		SystemDetail = -1,// Detail level used by internal searches
		DefaultDetail = 0,
	};

	SimObjectId id;		// Objects to exclude by id, or set to -1.
	UInt32 type;			// Query object type, used on some internal queries (set to -1)
	UInt32 mask;			// Objects to include by type mask
	UInt32 detail;			// Detail level used to control searching
	Box3F box;				// Box in world space

	bool isContained(SimContainer*) const;
};


//--------------------------------------------------------------------------- 
// Context used to control rendering through the container data base.

class SimContainerRenderContext: public SimContainerQuery {
public:
	struct State {
		// Internal states
		bool containCamera;

		// Public states
		TSRenderContext* renderContext;
		Int32 typeMask;		// Objects to draw
	};

private:
	typedef Vector<State> StateList;
	typedef SortableVectorPtr<SimRenderImage*> SimRenderImageList;

	int tag;
	StateList state;
   SimRenderImageList backgroundList;
   VectorPtr<SimRenderImage *> foregroundList;
	SimRenderImageList translucentList;
	SimRenderImageList overlayList;
	SimObject* viewportObject;

public:
	//
	SimContainerRenderContext();
	~SimContainerRenderContext();

   bool notRenderingContainers;  // The simGuiTSCtrl uses a SCRContext to render
                                 //  objects in the simRenderSet.  If this
                                 //  flags is true, this is what is happening,
                                 //  and render() sets a flag in the renderImage
                                 //  query

	int  getTag();
	void lock(SimObject*,TSRenderContext*);
	void unlock();
	void push(const State&);
	void pop();
	State& currentState();
	void render(SimObject* object);
	SimObject* getViewportObject() { return viewportObject; }
};


inline int SimContainerRenderContext::getTag()
{
	return tag;
}

inline void SimContainerRenderContext::push(const State& ss)
{
	state.push_back(ss);
}

inline void SimContainerRenderContext::pop()
{
	state.pop_back();
}

inline SimContainerRenderContext::State& SimContainerRenderContext::currentState()
{
	return state.last();
}	


//--------------------------------------------------------------------------- 
// Context used to control object scopeing

struct SimContainerScopeContext: public SimContainerQuery 
{
public:
	struct State {
		// Internal states
		bool containCamera;
		SimNetObject::CameraInfo* cameraInfo;
	};

private:
	friend SimContainer;
	typedef Vector<State> StateList;

	int tag;
	StateList state;
	Net::GhostManager* ghostManager;

public:
	SimContainerScopeContext();
	//
	int getTag();
	SimNetObject::CameraInfo* getInfo();
	void lock(Net::GhostManager* gman,SimNetObject::CameraInfo* camInfo);
	void unlock();
	void push(const State&);
	void pop();
	bool isScopeable(SimNetObject*);
	void scopeObject(SimNetObject* obj);
	State& currentState();
};

inline SimNetObject::CameraInfo* SimContainerScopeContext::getInfo()
{
   return state.last().cameraInfo;
}

inline int SimContainerScopeContext::getTag()
{
	return tag;
}

inline void SimContainerScopeContext::push(const State& ss)
{
	state.push_back(ss);
}

inline void SimContainerScopeContext::pop()
{
	state.pop_back();
}

inline SimContainerScopeContext::State& SimContainerScopeContext::currentState()
{
	return state.last();
}


//--------------------------------------------------------------------------- 
// ContainerDataBase
//--------------------------------------------------------------------------- 

// Abstract class used for container databases.
class SimContainerDatabase {
public:
	virtual ~SimContainerDatabase();
	virtual bool add(SimContainer*) = 0;
	virtual bool remove(SimContainer*) = 0;
	virtual void update(SimContainer*) = 0;
   virtual bool removeAll(SimContainer*) = 0;
	virtual bool findContained(const SimContainerQuery&,SimContainerList* list) = 0;
	virtual SimContainer* findOpenContainer(const SimContainerQuery&) = 0;
	virtual bool findIntersections(const SimContainerQuery&,SimContainerList* list) = 0;
	virtual void render(SimContainerRenderContext& rq) = 0;
	virtual void scope(SimContainerScopeContext& camInfo) = 0;
};



//--------------------------------------------------------------------------- 
// Containers
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

class DLLAPI SimContainer: public SimNetObject
{
	friend SimContainer;
	typedef SimNetObject Parent;

	// Containable data
	SimContainer *container;		// Parent container
	int containerData;				// Space for parents to store data.
	Box3F boundingBox;				// Box in world space

	// Container data
	static int lastTag;
	SimContainerDatabase* database;
  protected:
	int contextTag;

protected:
	Int32 containerMask;				// Used mainly by containers.

	bool installDatabase(SimContainerDatabase*);
   void uninstallDatabase();
   SimContainerDatabase* getDatabase();
	virtual void enterContainer(SimContainer*);
	virtual void leaveContainer(SimContainer*);

	void onDeleteNotify(SimObject *obj);
	void onRemove();

public:
	//
	SimContainer();
	~SimContainer();

	// Basic object methods
	bool isOpen();
	void setContainerData(int data);
	int getContainerData();
	SimContainer* getContainer();
	UInt32 getContainerMask();

	void setBoundingBox(const Box3F& box);
	const Box3F& getBoundingBox();
	Point3F getBoxCenter();

	// Support methods
	bool isIntersecting(SimContainer* obj);
	bool isContained(SimContainer* obj);
	bool isIntersecting(const Box3F& bbox);
	bool isContained(const Box3F& bbox);
	bool isIntersecting(const SimContainerQuery& query);
	bool isContained(const SimContainerQuery& query);

	// Data base access methods
	// The following methods are only valid if the container
	// has a database.
	virtual bool addObject(SimContainer*);
	virtual bool removeObject(SimContainer *);
	virtual bool updateObject(SimContainer*);
	virtual SimContainer* findOpenContainer(SimContainer*);
	virtual bool findIntersections(SimContainer*,SimContainerList* list,
		UInt32 detail = SimContainerQuery::DefaultDetail);
	virtual bool findIntersections(const SimContainerQuery&,SimContainerList* list);
	virtual bool findLOS(const SimContainerQuery&,SimCollisionInfo* info,
		SimCollisionImageQuery::Detail d = SimCollisionImageQuery::Normal);
   virtual bool findLOS(const SimContainerQuery&,SimCollisionInfoList* infoList,
		SimCollisionImageQuery::Detail d = SimCollisionImageQuery::Normal);

	// Scoping & Rendering using the data base
	virtual bool render(SimContainerRenderContext& rq);
	virtual bool scope(SimContainerScopeContext& camInfo);

	// Utility functions used by objects when rendering themselves
	virtual float getHazeValue(TSRenderContext &, const Point3F & objectPos);
   virtual float getHazeValue(const Point3F &, const Point3F &);
   virtual void getAlphaBlend(TMat3F &objectPos, ColorF *alphaColor, float *alphaBlend);

   virtual float getScopeVisibleDist() const;

	// Misc
	int getTag();
	static int getNextTag();
};


//--------------------------------------------------------------------------- 

inline bool SimContainer::isOpen()
{
	// Open means the container may actually contain
	// something as is not just a leaf.
	return database;
}

inline void SimContainer::setContainerData(int data)
{
	containerData = data;
}

inline int SimContainer::getContainerData()
{
	return containerData;
}

inline const Box3F& SimContainer::getBoundingBox()
{
	return boundingBox;
}

inline Point3F SimContainer::getBoxCenter()
{
	Point3F p = boundingBox.fMin;
	p += boundingBox.fMax;
	p *= 0.5f;
	return p;
}

inline SimContainer* SimContainer::getContainer()
{
	return container;
}

inline UInt32 SimContainer::getContainerMask()
{
	return containerMask;
}

inline bool SimContainer::isIntersecting(SimContainer* obj)
{
	return isIntersecting(obj->getBoundingBox());
}

inline bool SimContainer::isContained(SimContainer* obj)
{
	// Does this object contain obj.
	return isContained(obj->getBoundingBox());
}

inline int SimContainer::getTag()
{
	return contextTag;
}

inline int SimContainer::getNextTag()
{
	return lastTag++;
}


//--------------------------------------------------------------------------- 
// Default database container
// Provides a simple object list data base.  All queries are
// performed in linear time.
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

class SimDefaultContainerDatabase: public SimContainerDatabase
{
protected:
	SimContainerList objectList;

	bool add(SimContainer*);
	bool remove(SimContainer*);
	void update(SimContainer*);
   bool removeAll(SimContainer*);
	bool findContained(const SimContainerQuery&,SimContainerList* list);
	SimContainer* findOpenContainer(const SimContainerQuery&);
	bool findIntersections(const SimContainerQuery&,SimContainerList* list);
	void render(SimContainerRenderContext& rq);
	void scope(SimContainerScopeContext& camInfo);

public:
	SimDefaultContainerDatabase();
	~SimDefaultContainerDatabase();
};

//--------------------------------------------------------------------------- 
// Implements a container using the default database
//
class SimDefaultOpenContainer: public SimContainer
{
protected:
	SimDefaultContainerDatabase database;

public:
	SimDefaultOpenContainer(SimObjectId = 0);
	~SimDefaultOpenContainer();
};


#endif

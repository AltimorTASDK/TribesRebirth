//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

#ifndef _SIMDEBRIS_H_
#define _SIMDEBRIS_H_

#include <Sim.h>
#include <SimMovement.h>
#include <SimTagDictionary.h>
#include <SimDebrisRecord.h> // this can be included by debris data files
#include <ts.h>

class debrisRenderImage : public SimRenderImage
{
   bool  useLighting;
   float lightCoefficient;
   
public:
	TMat3F transform;
	TSShapeInstance * shape;
	bool animated;
	int lastDetail;
	void render(TSRenderContext &rc);

   void setLighting(float lc) { lightCoefficient = lc; useLighting=true; }
   void clearLighting() { useLighting=false; }
   bool lightingOn() { return useLighting; }
   float getLighting() { return lightCoefficient; }

	debrisRenderImage() { shape=0; animated=false; lastDetail=-1; useLighting=false; }
};

class SimDebrisCloud;
class SimPartDebris;
class SimTimerEvent;

class SimDebris : public SimMovement
{
	friend SimDebrisCloud;
   friend SimPartDebris;

	typedef SimMovement Parent;
	enum { netInit = 0x01, netUpdate = 0x02 };
   
   // enum for collision detail -- defined in debris record (info)
   enum { SphereCollisions=0, BoxCollisions=1, DetailedCollisions=2 };

	int debrisID;
	int debrisIndex;

   int shapeTag;      // if + on addnotify use this shape tag, otherwise use one in info
   char *shapeName;
   float debrisMass;  // if + on addnotify use this mass, otherwise use mass in info
   int animationSequence; // if + on addNotify, use this sequence, ow use value in info
   float animRestartTime; // if + on addNotify, use this for restart time, ow use value in info
   float duration;        // if + on addNotify, use this for restart time, ow use value in info
   int spawnedExplosionId; // if + on addNotify, yada yada...
   float elastic;
   float explodeProb;
   float sinkDepth;
   float spawnImpulse;
   bool  explodeOnRest;
   
   // use this address for shape's detail scale
   float * shapeDetailScale;

	const SimDebrisRecord * info;

	debrisRenderImage image;
	SimCollisionSphereImage collisionImage;

	Point3F radii; // radii of box used for collisions
	               // note:  collision image still sphere, box part taken care
						//        of in bounce method called from processCollision

	bool ghostMe;

	// keep track of time
	float reanimateTime;       // restart animation at this time
	float timeout;             // timeout at this time
	float debrisTime;          // when this piece of debris was made
   SimTime lastUpdateTime;    // for interpolation on render query
   bool firstUpdate;          // true till done with first timer update
   int timerCounter;

	// don't spawn anything after this amount of time has elapsed
	float spawnTimeout;

   // damage
   float damage;

	bool expire;               // when true, expire/explode debris
	bool inCloud;              // part of debrisCloud or not

	int gridPoly; // index into gridPolyStore -- for faster terrain collision checking

	// keep track of bounces
	virtual bool processCollision(SimMovementInfo* info);
	bool bounce(TMat3F & objToWorld, Point3F & normal, float k);
   bool bounceSphere(Point3F & normal);
	bool bounce(SimMovementInfo* info); // called by processCollision
   
   // make sound on bounce?
   int bounceSound;
   float bounceSoundVelThresh;
   float bounceSoundRefractoryPeriod;
   float nextBounceSoundTime;

   // never try to kill debris 2 times in a row...
   bool hitThisTime;
   bool hitLastTime;

   // used for controling jitters
   Point3F avgLV;
   Point3F avgAV;
   float potentialEnergyDeficit;
   bool underSurfaceLastTime, underSurfaceThisTime;

	// event handlers
	bool onSimTimerEvent(const SimTimerEvent * event);
   virtual bool onSimMessageEvent(const SimMessageEvent *);

	// query handlers
	bool onSimRenderQueryImage(SimRenderQueryImage * query);
	bool onSimCollisionImageQuery(SimCollisionImageQuery * query);
   
   // over-ride SimMovement method
   bool getImageList(const Point3F & pos, ImageList * list);

	// initialization
	void finishAddToManager();
	virtual void initShape();

public:
   SimDebris(const SimDebris &from);
	SimDebris(int id = -1, int index = -1, float spawnTimeout = 0, bool ghost=false);
	void setShape(int _replaceShape, int _replaceMass = -1,
	                      int _replaceSequence = -1, float _replaceAnimRestart = -1);
	void setShape(const char *_replaceShapeName, int _replaceMass = -1,
	                      int _replaceSequence = -1, float _replaceAnimRestart = -1);
   void setDuration(float _duration);
   void setExplosion(int _explosionId, float si = -1.0f); 
   void setCollisionMask(int _newMask);
   void setExplodeProbability(float _p) { explodeProb = _p; }
   void setElasticity(float _e) { elastic = _e; }
   void setSinkDepth(float _depth) { sinkDepth = _depth; }
   void setShapeDetailScale(float*fptr) { shapeDetailScale = fptr; }
   void setBounceSound(int _bounceSound, float _velThresh, float _timeThresh);
   void setLighting(float coeff) { image.setLighting(coeff); }
   
   void clearLighting() { image.clearLighting(); }

   TSShapeInstance * getShape() { return image.shape; }
   int getDebrisId() { return debrisID; }
   SimTime getLastUpdateTime() { return lastUpdateTime; }
   int getCollisionMask() { return collisionMask; }

	~SimDebris();

	static void createOnClients(SimDebris *, SimManager *, float scopeDistance = 3000.0f);

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
	bool onAdd();
	void onRemove();

	// Persistent IO
	DECLARE_PERSISTENT(SimDebris);

	DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
	void unpackUpdate(Net::GhostManager *gm, BitStream *stream);
};

inline void SimDebris::setExplosion(int _explosionId, float si)
{
   spawnedExplosionId = _explosionId;
   spawnImpulse = si;

   // if this guy was detailed out, bring him back
   if (expire)
   {
      spawnTimeout = fabs(spawnTimeout);
      expire = false;
   }   
}

class SimDebrisTable : public SimObject
{
public:
	struct Entry
	{
		SimDebrisRecord entry;
		int operator<(const Entry & e) const { return e.entry.mass > entry.mass; }
	};

	struct ShortListEntry
	{
		int index;
		Int32 mass;
		float p;
	};
	typedef Vector<ShortListEntry> ShortList;

private:
	static SimDebrisTable * tableObject;

	SortableVector<Entry> table;

	bool load(const char * fileName);

	const SimDebrisRecord* lookup(Int32 id);
	const SimDebrisRecord* get(int index);
	void   add(const SimDebrisRecord & item);
	float  getShortList(float mass, Int32 typeMask, ShortList & sl);

public:
	SimDebrisTable();
	~SimDebrisTable();

	static bool load(int tableTagID,SimManager * mgr);
	static bool load(const char * fileName,SimManager * mgr);

	static const SimDebrisRecord* lookup(Int32 id, SimManager * mgr);
	static const SimDebrisRecord* get(int id, SimManager * mgr);
	static void   add(const SimDebrisRecord & item, SimManager *);
	static float  getShortList(float mass, Int32 typeMask, ShortList & sl, SimManager *);
	static bool  loaded(SimManager * mgr) { return find(mgr); }
	static SimDebrisTable* find(SimManager *mgr);

   void onPreLoad(SimPreLoadManager *splm);

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
   bool processArguments(int, const char**);
	
   DECLARE_PERSISTENT(SimDebrisTable);
};

class SimPartDebrisCloud;

class SimDebrisCloud : public SimContainer
{
   friend SimPartDebrisCloud;
	typedef SimContainer Parent;

	//
	float mass;
	Int32 debrisMask;
	Point3F pos;
	float radius;
	Point3F vel;
	float outwardImpulse;
	Point3F axis;
	Point3F axes[3];
	float angleRange; // M_PI for a circle, M_PI/2 for a hemisphere, etc...
	Point3F incomingImpulse;
	float spawnTimeout;
	int soundId; // if negative, no sound
	bool dataReceived; // initially false if cloud sent over the net (o.w., true)
   
   //
   bool useLighting;
   float lightingCoefficient;

	// debris cloud container to put our debris into if we are spawn
	SimDebrisCloud * spawnedFrom;
   
   // a list of objects this debris cloud intersects (used by debris)
   SimContainerList intersectionList;
   
	bool onSimTimerEvent(const SimTimerEvent *);
   bool onSimMessageEvent(const SimMessageEvent *);

	void finishAddToManager();

public:
	static float detailScale; // 0-1:  0=no debris; 1=full debris ... effects mass
	static void getAxes(const Point3F & axis, Point3F * axes, float & aRange);
	static void toss(SimDebris * debris, 
	                         const Point3F & pos, float radius,
	                         const Point3F & vel, float outwardImpulse,
	                         const Point3F * axes, float angleRange,
	                         const Point3F & incomingImpulse, SimManager * mgr);

	static void createOnClients(SimDebrisCloud *, SimManager *, float scopeDist = 3000.0f);

   SimDebrisCloud(const SimDebrisCloud &cloud);
	SimDebrisCloud(float mass, Int32 typeMask,
	                          const Point3F &pos, float radius,
	                          const Point3F &vel, float outwardImpulse,
	                          const Point3F & axis, float angleRange, 
	                          const Point3F & incomingImpulse, float spawnTO);
	SimDebrisCloud();
	~SimDebrisCloud();
	void setSound(int _soundId) { soundId = _soundId; }
   void setLighting(float coeff) { useLighting = true; lightingCoefficient=coeff; }

   VectorPtr<SimDebris*> & getDebrisList();
   SimContainerList & getIntersections()
   {
     // only valid in onSimTimerEvent
     AssertFatal(database.inTimer,"SimDebrisCloud::getIntersections: not allowed here.");
     return intersectionList;
   }
   void addCollisionMask(int _addMask) { database.unionCollisionMask |= _addMask; }

	void setSpawnedFrom(SimDebrisCloud *);

	bool processEvent(const SimEvent*);
	bool onAdd();

	// Persistent IO
	DECLARE_PERSISTENT(SimDebrisCloud);

	DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
	void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

	// container methods for debris cloud

	protected:
		class Database: public SimContainerDatabase {
		public:
			VectorPtr<SimDebris*> debrisList;
         bool inTimer;  // true when in timer update
         int unionCollisionMask; // the union of the collision masks of contents

			// Internal Data base methods
			bool add(SimContainer*);
			bool remove(SimContainer*);
			void update(SimContainer*);
			void update(SimContainer *object, SimContainer *cnt);
	   	bool removeAll(SimContainer*);
			bool findContained(const SimContainerQuery&,SimContainerList* list);
			SimContainer* findOpenContainer(const SimContainerQuery&);
			bool findIntersections(const SimContainerQuery&,SimContainerList* list);
			void render(SimContainerRenderContext& rq);
			void scope(SimContainerScopeContext& sc);
         Database() { inTimer = false; unionCollisionMask = 0; }
		} database;

	public:
		// Data base access methods
		bool updateObject(SimContainer*);
};

inline VectorPtr<SimDebris*> & SimDebrisCloud::getDebrisList()
{
	return database.debrisList;
}

extern void freeGridPolyStore();

#endif
//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _myGuy_H_
#define _myGuy_H_

#include <simResource.h>
#include <simAction.h>
#include <simRenderGrp.h>
#include <ts_shapeInst.h>
#include <ts_shadow.h>
#include "mover.h"

#include <SimTerrain.h>
// #include <SimInterior.h>

#define maxThreads 100

#define  CAST_ SHADOWS  false


#define COREAPI
//--------------------------------------------------------------------------- 

class myGuyRenderImage: public SimRenderImage
{
	int minDetail;
	int maxDetail;
	int currentDetail;
public:
	myGuyRenderImage() 
		{ minDetail=-1;currentDetail=0; maxDetail=1000; castShadowTerr=false; castShadowItr=false; }
	void setTerrain( const TMat3F & shapeToWorld, const TMat3F & worldToShape );
	// void setItr( const TMat3F & shapeToWorld, const TMat3F & worldToShape );
	TMat3F transform;
	TMat3F invTransform;
	TSShapeInstance* shape;
	TS::Shadow shadow;
	SimTerrain *terr;
	// SimInterior *itr;
	Point3F lightDirection;
	float shadowLift;
	bool castShadowTerr;
	bool castShadowItr;
	// total number of detail levels
   int numDetailLevels() { return shape->getShape().fDetails.size(); }
	// current detail levels
	int getDetailLevel() { return currentDetail; }
	// force a range, but determines level during each render
   void setDetailLevel(int mind,int maxd)
   	{ minDetail=mind; maxDetail=maxd; }
   // force a single value
	void setDetailLevel(int newDetailLevel)
		{ minDetail=newDetailLevel; maxDetail=newDetailLevel; }
	int numAnimSeq()
		{ return shape->getShape().fSequences.size(); }
	static bool renderBounds;
   void renderBoundingBox(TSRenderContext& rc);
	void render(TSRenderContext &rc);
};

class myGuy: public SimObject
{
   typedef SimObject ObjParent;
   
protected:
	// the shape goes here
	myGuyRenderImage image;
   Resource<SimActionMap> eventMap;

	// variables for animation
	TS::ShapeInstance::Thread *animThreads[maxThreads];
	bool animating[maxThreads];
	int numThreads;
	int curThread;
	float animSpeed[maxThreads];
	bool visible;
	bool stickToGround;
	bool killRootTransform;

	// map between joystick actions and "postAction" actions
//   SimActionEventMap eventMap;

	moverHelper mover;

	// ghosting variables
   char *ghost_name;		// object name to pass on to ghost
	char *shape_name;		// our resource to load
   int managerId;			// manager we came from
   int groupId;			// group to add to when added to manager
   int ghostGroupId;		// group ghost should add itself to when added

	// scale stuff
	bool isScaled;
	Point3F scaleVector;

	void update(SimTime time);
//   void setEventMap();
   void init();
	void loadResources ();

public:
	myGuy(const char* shapename, const char *ghost_name = 0,
              int manager_id = -1, int ghost_group_id = 0,
              int group_id = 0 );
	myGuy();
	~myGuy();
	void setPos(Point3F pos);
	void setPosVel(Point3F pos);
	void setRot(EulerF pos);
	void setRotVel(EulerF pos);
   const Point3F *getPos() { return &mover.getPos(); }
   const EulerF *getRot() { return (EulerF *)&mover.getRot(); }

	int numDetailLevels();
	int getDetailLevel();
	void setDetailLevel(int nl);
	void setDetailLevel(int min,int max);
	float getSphere() { return image.shape->getShape().fRadius; }
	const Point3F &getCenter() { return image.shape->getShape().fCenter; }
	const TSShape & getShape() { return image.shape->getShape(); }
	TSShapeInstance & getShapeInst() { return *(image.shape); }

	// animation stuff
	void pause() { animating[curThread]=false; }
	void play() { animating[curThread]=true; }
	bool isPlaying() { return animating[curThread]; }
	void stepThread(float increment, int tNum=-1);
	void step(float increment);
	void setKillRootTransform(bool nval) { killRootTransform=nval; }
	void setStickToGround(bool nval) { stickToGround=nval; }
	bool getStickToGround() { return stickToGround; }
	void setAnimSpeed(float ns) { if (numThreads) animSpeed[curThread]=ns; }
	float getAnimSpeed() { if (numThreads) return animSpeed[curThread]; return 0.0f; }
	float getAnimDur()
	{
		if (animThreads[curThread]->getState()==TSShapeInstance::Thread::InTransition)
			return animThreads[curThread]->getTransition().fDuration;
		else
			return animThreads[curThread]->getSequence().fDuration;
	}
	float getAnimDur(int seq) { return image.shape->getSequence(seq).fDuration; }
	float getAnimPos() { return animThreads[curThread]->getPosition(); }
	void  setAnimPos(float np,bool animateRoot=false)
	{
		animThreads[curThread]->SetPosition(np);
		if (animateRoot)
			image.shape->animateRoot();
	}
	void setVisible(bool v) { visible=v; }
	void invalidateSList() { animThreads[curThread]->invalidateSubscriberList(); }

	Vector<TSShape::Transition> const & getTransitionList() 
		{ return (image.shape->getShape().fTransitions); }
	void setTransition(int transNum) { animThreads[curThread]->SetTransition(
		&(getTransitionList()[transNum]) ); }
	void transitionTo(int seq) { animThreads[curThread]->TransitionToSequence(seq); }

	int newThread(int seqNum=0);
	void selectThread(int threadNum);
	int getNumThreads() { return numThreads; }
	int getCurThreadIndex() { return curThread;}
	int getState() { return animThreads[curThread]->getState(); }
	TSShapeInstance::Thread & getThread() { return *animThreads[curThread]; }

	int numAnimSeq() { return image.numAnimSeq(); }
	void setAnimSeq(int seqNum,float startTime=0.0f);
	int getAnimSeq();
	const char * getSeqName(int seqNum)
	  { return image.shape->getShape().fNames[image.shape->getShape().fSequences[seqNum].fName]; }

	void setScale(Point3F & v)
	{
		isScaled=true;
		scaleVector=v;
	}

	bool COREAPI processEvent(const SimEvent*);
	bool COREAPI processQuery(SimQuery*);

   bool shape() { return(image.shape); }   // used for load error check
   bool onAdd();

	// Persistent IO
	DECLARE_PERSISTENT(myGuy);
	Error COREAPI read( StreamIO &, int version, int );
	Error COREAPI write( StreamIO &, int version, int );
};

inline void myGuy::setPos(Point3F pos)
{
	mover.setPos(pos);
}

inline void myGuy::setPosVel(Point3F vel)
{
	mover.setPosVel(vel);
}

inline void myGuy::setRot(EulerF rot)
{
	mover.setRot(rot);
}

inline void myGuy::setRotVel(EulerF rot)
{
	mover.setRotVel(rot);
}

#endif

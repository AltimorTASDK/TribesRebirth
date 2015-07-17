//-----------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#ifndef _SIMFIRE_H_
#define _SIMFIRE_H_

#include <sim.h>
#include <simFireRecord.h>
#include <simContainer.h>
#include <fxRenderImage.h>
#include <m_random.h>
#include <soundFx.h>
#include "simLightGrp.h"

//------------------------------------------------------------------------------
class SimTimerEvent;

class SimFire : public SimContainer
{
	typedef SimContainer Parent;

   private:   
      // shape tags
      int         shapeTag;
		int         translucentShapeTag;
		int         soundID;
      SFX_HANDLE  hSound;

      SimObject   *onObj;       // fire will follow this object

      Point3F     pos;          // if onObj is set, pos is an offset from
                                // the onObj's position, 
                                // else, pos is global location

      Box3F       bbox;         // bounding box of explosion in shape space
                                // used to construct world space box for
                                // container system
  
      bool        renderedYet;  // starts false, true after 1st render
      bool        inHardware;
      bool        faceCam;
      bool        followCam;
      bool        updateBox;

      float       fireOut;      // time to put fire out

      // smoke information
      bool        producesSmoke;
      int         smokeShapeTag;
      int         smokeTransShapeTag;
      float       smokeToSmoke; // inter-smoke-interval
      float       smokeOut;
      Point3F     smokeOffset;
      float       fireHeight;
      Point3F     smokeVel;
      bool        useLight;
      float       lightRange;
      ColorF      lightColor;
   
   private:
      // animation data
      fxRenderImage            image;
      Resource<TSShape>        hShape;
      TSLight glow;      

		float   prevAnimTime;

		float   nextSmokeTime;

   private:
		void getShape(int shapeTag,TSShapeInstance**,TSShapeInstance::Thread**);
      void getPosition(Point3F & pos);
		void updateBBox(const Point3F & pos);

		void finishAddToManager();
      bool onSimTimerEvent(const SimTimerEvent *);
      bool onSimLightQuery ( SimLightQuery *);
  
   public:
      static bool DynamicLighting;
   
      SimFire( int _shapeTag = -1, int _tshapeTag = -1, float fireOut = 5.0f );
		SimFire( const SimFireRecord *, float fireOut );
		SimFire( SimFire& );

		void setFireDuration(float _time);
		void setSound(int _id);
		void setPosition(const Point3F & _pos);
		void setSmoke(int _shapeTag = -1, int _tshapeTag = -1, 
		                      float _ISI = 0.5f, float _smokeOut = 5.0f);
		void setSmokePosVel(const Point3F & _offset, const Point3F &_vel);
		void setSmokeVel(const Point3F &_vel);

		void setObject(SimObject *obj, const Point3F & _offset);
			// (optional) object to attach to with offset.

		void setFaceCam() { faceCam = true; }
		void setFollowCam() { followCam = true; }
      
      void setLight(bool nv) { useLight = nv; }

		static void createOnClients(SimFire *, SimManager *);
			// create version of fire on all clients (should be called on server)
			// fires don't ghost, since they can run fine w/o ghostUpdate events

   	bool processEvent(const SimEvent *);
   	bool processQuery(SimQuery *);
		bool onAdd();
		void onRemove();
		void onDeleteNotify(SimObject* obj);

		// Persistent IO
		DECLARE_PERSISTENT(SimFire);

		DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
		void unpackUpdate(Net::GhostManager *gm, BitStream *stream);
};



class SimFireTable : public SimObject
{
	Vector<SimFireRecord> table;

	bool load(const char * fileName);

	const SimFireRecord* lookup(Int32 id);
	void  add(const SimFireRecord & item);

public:
	SimFireTable();
	~SimFireTable();

	static bool load(int tableTagID,SimManager * mgr);
	static bool load(const char * fileName,SimManager * mgr);

	static const SimFireRecord* lookup(Int32 id, SimManager * mgr);
	static const SimFireRecord* get(int id, SimManager * mgr);
	static void  add(const SimFireRecord & item, SimManager *);
	static bool  loaded(SimManager * mgr) { return find(mgr); }
	static SimFireTable* find(SimManager*);

   void onPreLoad(SimPreLoadManager *splm);

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
   
   bool processArguments(int, const char**);

   DECLARE_PERSISTENT(SimFireTable);
};

#endif



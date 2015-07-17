//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMEXPLOSION_H_
#define _SIMEXPLOSION_H_

//Includes
#include <m_random.h>
#include <sim.h>
#include <simContainer.h>
#include <simLightGrp.h>

#include "simExplosionTable.h"
#include "fxRenderImage.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimExplosionCloud;
class SimTimerEvent;

class DLLAPI SimExplosion : public SimContainer
{
	friend SimExplosionCloud;
	typedef SimContainer Parent;

   protected:   
		const SimExplosionRecord * info;

      // type and positioning data
      int         explosionID;  // these two are used to look up explosion
		int         index;        // in SimExplosionTable -- exactly one of
		                          // them must be != -1

      SimObject   *onObj;       // explosion will follow this object
      Point3F     pos;          // if onObj is set, pos is an offset from
                                // the onObj's position, 
                                // else, pos is global location
      Point3F     lightPos;

		bool        soundOn;

		Box3F			bbox;			  // bounding box of explosion in shape space
										  // used to construct world space box for
										  // container system
		bool        updateBox;    // update bbox in containers when set
		                          // neeed because we can't do it during render query
  
		bool        backface;     // should we "abort" explosion if axis faces other way
		bool        renderedYet;  // starts false, true after 1st render

		Point3F     yaxis;        // axis used for backfacing and (if !faceCam) orienting shape
		bool        shiftPos;     // if true, shift pos of explosion on yaxis
		                          // to move away from surface, set axis to surface normal

      float       detonateTime; // time to detonation -- initially from
                                // time of creation, but after addNotify
										  // in manager time

		float       expireTime;   // time to delete oneself

      bool        detonating;

      ColorF      m_dynamicLightColor0;
      ColorF      m_dynamicLightColor1;
      ColorF      m_dynamicLightColor2;
      float       m_radFactor0;
      float       m_radFactor1;
      float       m_radFactor2;
      float       m_dynamicLightRange;
      float       m_lightTime0;
      float       m_lightTime1;

   protected:
      // animation data
      fxRenderImage  image;
      Resource<TSShape>        hShape;

		float   prevAnimTime;

   	//
   	TS::Light m_light;
        bool m_disableLight;

   protected:
		virtual void getShape(int shapeTag, TSShapeInstance**,TSShapeInstance::Thread**);
   private:
      void detonate();
      void getPosition(Point3F & pos);
      void getLightPosition(Point3F & pos);
		virtual void updateBBox(const Point3F & pos);

      bool onSimTimerEvent(const SimTimerEvent *);

		bool finishAddToManager(); // happens either during onSimAddNotify or
		                                   // unpackUpdate

      bool onSimRenderQueryImage(SimRenderQueryImage* query);
	   bool onSimLightQuery ( SimLightQuery * query );

   public:
		static bool SoftwareTranslucency;
      static bool DynamicLighting;
      static int defaultFxObjectType;

      SimExplosion(const SimExplosion &explosion);
      SimExplosion( int _id = -1, int _index = -1);

		static void createOnClients(SimExplosion *, SimManager *, float scopeDistance = 3000.0f);
			// create version of explosion on all clients (should be called on server)
			// explosions don't ghost, since they can run fine w/o ghostUpdate events

		void setPosition(const Point3F & _pos);
		void setObject(SimObject *obj,const Point3F & _offset);
			// (optional) object to attach to with offset.
		void setAxis(const Point3F & _axis, bool _backface=false);
			// note:  axis has 2 purposes.  1: explosions are shifted away from a wall by setting
			//        the axis to the wall normal.  2:  explosions are backfaced using axis (if
			//        axis doesn't face camera, explosion not set off).
		void setDetonationTime(float _time);
		void setSound(bool _sOn);
                void disableLight(bool _lOff) { m_disableLight = _lOff; }

		// note:  this method no longer works as advertised -- duration no longer accessible
		//        till render...should be removed at some point
		float getDuration() { return info->timeScale; }

   	bool processEvent(const SimEvent *);
   	bool processQuery(SimQuery *);
		bool onAdd();
		void onRemove();
		void onDeleteNotify(SimObject* obj);

		// Persistent IO
		DECLARE_PERSISTENT(SimExplosion);
		
		DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
		void unpackUpdate(Net::GhostManager *gm, BitStream *stream);
};


//--------------------------------------------------------------------------- 
// SimExplosionCloud
//--------------------------------------------------------------------------- 
//
class SimExplosionCloud : public SimContainer
{
	typedef SimContainer Parent;

	float megatons;
	Int32 typeMask;

	int form;
	bool topOnly;

	bool hasTransform;
	TMat3F transform;

	Point3F center; // for circle, sphere
	float radius;
	Box3F box;      // for box

	int soundId;

	bool dataReceived; // initially false if cloud sent over the net (o.w., true)

	bool removeFromRenderSet; // set after all explosions created

	float detonationMax; // in seconds
	float proportionNow; // proportion of explosions to detonate now

	bool onSimTimerEvent(const SimTimerEvent *);
        bool onSimMessageEvent(const SimMessageEvent *);
	bool onSimRenderQueryImage(SimRenderQueryImage *);
	void finishAddToManager();
	void lightFuse(SimExplosion * explosion, Point3F & cameraPos, bool igniteNow);
	void makeSound();

public:
	enum { Circle, Sphere, Box, Screen };

	static float detailScale; // 0-1:  0=no explosions; 1=full explosions ... effects megatons

	static void createOnClients(SimExplosionCloud *, SimManager *);

	// this constructor is for circle, sphere, and dome (sphere with topOnly)
	SimExplosionCloud(float _megatons, Int32 _typeMask,
	                             int _form, bool _topOnly,
	                             const Point3F & _center, float _radius,
                                const TMat3F * _transform=0);

	// this constructor is for box only
	SimExplosionCloud(float _megatons, Int32 _typeMask, 
	                             int _form, bool _topOnly, 
	                             const Box3F & _box, const TMat3F * _transform=0);

	// this constructor copies just enough to be sent over the net
	SimExplosionCloud(SimExplosionCloud&);

	void setDetonation(float _detonationMax, float _proportionNow);
	void setSound(int _soundId);

	SimExplosionCloud() { dataReceived = false; }

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
	bool onAdd();

	// Persistent IO
	DECLARE_PERSISTENT(SimExplosionCloud);

	DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
	void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

	// container methods for explosion cloud

	protected:
		class Database: public SimContainerDatabase {
		public:
			Vector<SimExplosion*> explosionList;

			// Internal Data base methods
			bool add(SimContainer*);
			bool remove(SimContainer*);
			void update(SimContainer*);
			bool findContained(const SimContainerQuery&,SimContainerList* list);
			SimContainer* findOpenContainer(const SimContainerQuery&);
			bool findIntersections(const SimContainerQuery&,SimContainerList* list);
			void render(SimContainerRenderContext& rq);
			void scope(SimContainerScopeContext& sc);
	   	bool removeAll(SimContainer*);
		} database;

	public:
		// Data base access methods
		bool updateObject(SimContainer*);
		SimContainer* findContainer(SimContainer*);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMEXPLOSION_H_


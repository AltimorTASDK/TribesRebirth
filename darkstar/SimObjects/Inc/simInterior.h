//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#ifndef _SIMINTERIOR_H_
#define _SIMINTERIOR_H_

#include <m_box.h>
#include <sim.h>
#include <inspect.h>
#include <itrmetrics.h>
#include <itrgeometry.h>
#include <itrlighting.h>
#include <itrinstance.h>
#include <itrrender.h>
#include <itrcollision.h>
#include <simContainer.h>
#include <simCollision.h>
#include <simCollisionImages.h>

#include <move.h>

#include <simNetObject.h>
#include <simevdcl.h>

#include <tMHashMap.h>

//----------------------------------------------------------------------------

#define MAX_ROT (M_2PI/10.0f)

class SimInterior;
class MissionLighting;
class SimTimerEvent;

//------------------------------------------------------------------------------
// For container database support

class SimInterior: public SimContainer
{
	typedef SimContainer Parent;
   friend class InteriorCollideable;
   friend class MissionLighting;
      
   static const int sm_fileVersion;

  protected:
   enum NetMaskFlags {
      FileNameMask      = 1 << 0,
      TransformMask     = 1 << 1,
      InteriorStateMask = 1 << 2,
      InitMask          = 1 << 31
   };

	class RenderImage: public SimRenderImage {
	  public:
      SimInterior* pSimInterior;    // We need to send link messages to the interior
      
		ITRRender    rend;
		ITRInstance* instance;
		TMat3F       transform;
		TMat3F       inverseTransform;

      void render( TSRenderContext &rc );
	};
   friend class RenderImage;

   
   //-------------------------------------- Persistent Data members
   //
   const char*    m_pFilename;

   //-------------------------------------- Transient Data members
   //
   UInt32               m_currentState;
   UInt32               m_currentLightState;
   
   RenderImage          renderImage;
   SimCollisionItrImage collisionImage;

   // For state-lights
   Vector<int> animatingLights;

	//
	bool setFilename(const char* files);
   void unloadResources();

	void  updateBoundingBox();

   bool onAdd();
   void inspectRead(Inspect*);
   void inspectWrite(Inspect*);

   bool onSimTimerEvent(const SimTimerEvent* event);
   bool onSimObjectTransformEvent(const SimObjectTransformEvent *);

	bool onSimRenderQueryImage(SimRenderQueryImage* query);
	bool onSimCollisionImageQuery(SimCollisionImageQuery* query);
   bool onSimObjectTransformQuery(SimObjectTransformQuery *query);
   bool missionLitName();

  public:
	SimInterior();
	~SimInterior();

	bool loadShape(const char *fileName);
   bool processArguments(int argc, const char **argv);

   void getBaseFilename( String & str );
   const char * getFilename() { return( m_pFilename ); }
	const TMat3F& getTransform()    const { return renderImage.transform; }
	const TMat3F& getInvTransform() const { return renderImage.inverseTransform; }
	ITRInstance*  getInstance()     const { return renderImage.instance; }

   int  getLightId(const char* in_pLightName);
   bool animateLight(const int   in_lightId,
                     const Int32 in_loopCount);
   bool deanimateLight(const int     in_lightId);
   bool resetLight(const int     in_lightId);
   
	void set(const TMat3F& mat, bool force = false);
	void set(const EulerF&,const Point3F&);
   bool setState(const UInt32 in_state);
   bool setLightState(const UInt32 in_lightState);

	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);

   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void  unpackUpdate(Net::GhostManager *gm, BitStream *stream);
   static void initPersistFields();
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
   int version();
	DECLARE_PERSISTENT(SimInterior);
};


#endif

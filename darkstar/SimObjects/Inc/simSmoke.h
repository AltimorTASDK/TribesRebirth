//-----------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#ifndef _SIMSMOKE_H_
#define _SIMSMOKE_H_

#include <sim.h>
#include <simContainer.h>
#include <fxRenderImage.h>
#include <m_random.h>

class SimTimerEvent;
//------------------------------------------------------------------------------

class SimSmoke : public SimContainer
{
	typedef SimContainer Parent;

   private:   
      // shape tags
      int         shapeTag;
		int         translucentShapeTag;

      Point3F     pos;
		Point3F     vel;
		Box3F			bbox;			  // bounding box of smoke shape in shape space
										  // used to construct world space box for
										  // container system
  
		bool        renderedYet;  // starts false, true after 1st render
		bool        faceCam;
		bool        followCam;

      float       smokeOut;     // time for smoke to disappear
		float       smokeDuration;// how long for smoke to last
   
   private:
      // animation data
      fxRenderImage            image;
      Resource<TSShape>        hShape;

		float   prevAnimTime;

   private:
		void getShape(int shapeTag,TSShapeInstance**,TSShapeInstance::Thread**);
		void updateBBox(const Point3F & pos);

      bool onSimTimerEvent(const SimTimerEvent *);

   public:
      SimSmoke( int _shapeTag = -1, int _tshapeTag = -1, float _smokeOut = 5.0f );

		void setPosition(const Point3F & _pos);
		void setVelocity(const Point3F & _vel);

		void setFaceCam() { faceCam = true; }
		void setFollowCam() { followCam = true; }

		void setDuration(float _time);

   	bool processEvent(const SimEvent *);
   	bool processQuery(SimQuery *);
		bool onAdd();
		void onRemove();
};


#endif
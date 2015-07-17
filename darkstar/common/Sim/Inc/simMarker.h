//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMMARKER_H_
#define _SIMMARKER_H_

#include <sim.h>
#include <inspect.h>
#include <simContainer.h>
#include <simCollideable.h>
#include <m_collision.h>
#include "commonEditor.strings.h"
#include <editor.strings.h>
#include "esfEvDcl.h"
#include <ts.h>

class SimMarker : public SimCollideable
{
   protected:
      typedef SimCollideable Parent;   	
   
      SimRenderDefaultImage    image;
		Resource<TSShape>        hShape;
		TSShapeInstance         *inst;

		SimCollisionSphereImage  collisionImage;
      
      enum NetMaskBits
      {
         Moved = 1,
      };
   
		// default shape for the object
      virtual Int32 getDefaultShape() { return IDCOMEDIT_SHP_2M_PYRM; }

		// Query Handlers
      virtual bool onSimObjectTransformQuery(SimObjectTransformQuery *query);
		virtual bool onSimCollisionImageQuery(SimCollisionImageQuery* query);
		virtual bool onSimRenderQueryImage(SimRenderQueryImage* query);
		virtual bool onSimImageTransformQuery(SimImageTransformQuery* query);
		
		// Event Handlers
      virtual bool onSimEditEvent(const SimEditEvent *event);
      virtual bool onSimObjectTransformEvent(const SimObjectTransformEvent *event);

      // Ghost Handling
      void ghostsOn();
      void ghostsOff();

      // Shape Stuff
      virtual void shapeOn();
      void shapeOff();

   public:
      SimMarker();
      ~SimMarker() { AssertFatal(inst == NULL, "shape never deleted"); }

		bool processEvent(const SimEvent *);
		bool processQuery(SimQuery *);
		bool onAdd();
		void onRemove();
      void onPreLoad(SimPreLoadManager *splm);
		void inspectRead(Inspect* inspect);
		void inspectWrite(Inspect* inspect);
      
      DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
      void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

		// Persistent IO
		DECLARE_PERSISTENT(SimMarker);
      static void initPersistFields();
		Error read(StreamIO &, int version, int);
		Error write(StreamIO &, int version, int);
};   

#endif //_SIMMARKER_H_
					 		
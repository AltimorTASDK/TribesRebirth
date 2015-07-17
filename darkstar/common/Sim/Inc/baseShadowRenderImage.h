#ifndef _H_BASESHADOWRENDERIMAGE
#define _H_BASESHADOWRENDERIMAGE

#include <sim.h>
#include <ts_shadow.h>
#include "simRenderGrp.h"

class SimContainer;
class SimTerrain;

// note:  must set transform (and itype) during SimRenderQuery,
// and "shape" must be set too (probably when first initializing
// image.

// if shadows are to be cast, castShadow must be set to true,
// and the root container must be set.  You should also set
// lightDirection (in world coords) -- a default direction is
// initially set, however.

// you can force a given detail level for the shape or shadow
// with setShapeDetail/setShadowDetail, and let the detail level
// run freely again by calling releaseShapeDetail/releaseShadowDetail


class DLLAPI BaseShadowRenderImage : public SimRenderImage
{
   public:
      struct ShadowDetailSettings
      {
         int shadowDetail;
         int hiShadowDetail;
         int blurMethod;
         int bmpDim;
         float updateDelta;
         bool projectTerrainOnly;
         bool useFloor;
         float liftFloor;
         bool cacheProjection;
         bool recacheProjection;
      } shadowSettings;

   protected:
      int prevBmpDim;
      float prevShadowUpdateDelta;
      static float shadowLiftHi; // set on fly depending on # of shadows out there

		int currentDetail;
		bool shadowOwnDetail; // does the shadow have its own detail level
		bool forcedDetailLevel; // if true, image doesn't set detail level
      Point3F lastLight; // don't recompute light if it hasn't changed...
      float nextShadowUpdateTime;
      float projSize;

		void getPolys(); // get polys to project onto
		void getTerrPolys(SimTerrain *);

      virtual void getUnknownPolys(SimContainer*);

      float findCameraDistance(TSCamera * camera);

   public:
		BaseShadowRenderImage();
      ~BaseShadowRenderImage();

      TMat3F transform;
      UInt32 projectionMask;
      TSShapeInstance *shape;

		bool castShadow; // yeah or nay
		bool drawShape; // ... draw shape?
		bool animateShape;
		TS::Shadow shadow;
		Point3F lightDirection; // before swing-down
      static int shadowNum;
      static float shadowLiftLo;
      static float shadowLiftStep;
      static int maxTerrainPolys;
      static int maxItrPolys;
      static int maxProjectionPolys;
		float alphaLevel; // translucency
      float swingDown;
		SimContainer * root; // must set to cast shadows

      // controls shape detail level used 
      float shadowDetailScale;

      void setImageType();
      void setSortValue(TSCamera* camera);
		void setShapeDetail(int nv) 
      { 
         currentDetail=nv; forcedDetailLevel=true; 
         shape->setDetailLevel(currentDetail);
      }
		void releaseShapeDetail() { forcedDetailLevel=false; }
		int  getDetailLevel() { return currentDetail; }

		void setShadowDetail(int nv)
      {
         shadowSettings.shadowDetail=nv;
         shadowOwnDetail=true;
      }
		void releaseShadowDetail() { shadowOwnDetail=false; }

      // calls drawShadow
		void render(TSRenderContext &);

      // call during render query
      void preRenderShadow(TSRenderContext & rc, float curTime);
      virtual void setShadowDetails(TSRenderContext & rc);
      virtual void setShadowDetailsA(TSRenderContext & rc);

      void drawShadow(TSRenderContext &);
};


#endif
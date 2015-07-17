//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRRENDER_H_
#define _ITRRENDER_H_

#include <ml.h>
#include <ts.h>
#include <tvector.h>
#include <itrbit.h>
#include <itrgeometry.h>

//----------------------------------------------------------------------------

class ITRInstance;
class ITRLighting;

//----------------------------------------------------------------------------

class ITRRender
{
  public:
   static UInt32 PrefInteriorTextureDetail;

  protected:
	static ITRBitVector surfaceSet;
	static ITRBitVector planeSet;
	static ITRBitVector pointSet;
	ITRInstance *instance;
	ITRGeometry *geometry;				// Obtained from instance
	ITRLighting* lighting;				// Obtained from instance
	TSMaterialList* materialList;	// Obtained from instance

	TSRenderContext* renderContext;
	TSCamera* camera;						// Obtained from RC
	TSPointArray* pointArray;			// Obtained from RC
	GFXSurface* gfxSurface;				// Obtained from RC

	struct PointM {
		float distance;					// Distance from camera
		int index;							// PointArray index
	};
	Vector<PointM> pointMap;
	Point3F cameraPos;					// Camera in object space
	float textureScale;					// Texture to world scale
	int outsideBits;						// Outside visibility

public:
   struct VertexMapEntry {
      int      localIndex[2];
      UInt32   copiedFrameKey;
      float    distance;

      Point3F  projectedPoint;
      int      status;
      Point4F  transformedPoint;
      float    paDist;
   };

   struct VertexRenderMap {
      VertexMapEntry* pVertexMap;
      int             numEntries;
      int             myKey;
   };

  private:
	//
	void leafVisible(ITRGeometry::BSPLeafWrap& leaf);
	int findLeaf(const Point3F& p,int nodeIndex = 0);
	void processPVS(ITRGeometry::BSPLeafWrap& leaf);
	void processPlanes(const Point3F& cp);
	void processSurfaces();
	void processPoints();
	void renderSurfaces();
	void renderSurfacesOGL(const bool in_renderPass);
	void renderSurfacesOGLSP();

  public:
	void render(TSRenderContext&         rc,
	            ITRInstance*             instance);
   int getOutsideVisibility() {
      return outsideBits;
   }

   // For separate vis queries (SimInterior)
	int getOutsideVisibility(const Point3F& in_cameraPos, ITRInstance*);
};

#endif

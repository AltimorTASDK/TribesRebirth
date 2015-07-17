//
// boxRender.cpp
//

// Renders a bright colored box, useful for debugging bounding box stuff

#include <sim.h>
#include "g_surfac.h"
#include "simRenderGrp.h"
#include "boxRender.h"
#include <ts.h>

//-----------------------------------------------------------------------

static void drawSquare(const Point3F &p0, const Point3F &p1, const Point3F &p2,
   const Point3F &p3, TS::PointArray *pa)
{
   TS::VertexIndexPair v[4];
   v[0].fVertexIndex = pa->addPoint(p0);
   v[1].fVertexIndex = pa->addPoint(p1);
   v[2].fVertexIndex = pa->addPoint(p2);
   v[3].fVertexIndex = pa->addPoint(p3);
   pa->drawPoly(4, v, 0);
}
 
//-----------------------------------------------------------------------

static void drawCube(const Point3F &P0, const Point3F &P1, 
   TSRenderContext &rc)
{
   TS::PointArray *pointArray = rc.getPointArray();

   const Point3F &a = P0;
   const Point3F  b = Point3F(P0.x, P0.y, P1.z);
   const Point3F  c = Point3F(P1.x, P0.y, P1.z);
   const Point3F  d = Point3F(P1.x, P0.y, P0.z);
   const Point3F  e = Point3F(P0.x, P1.y, P0.z);
   const Point3F  f = Point3F(P0.x, P1.y, P1.z);
   const Point3F &g = P1;
   const Point3F  h = Point3F(P1.x, P1.y, P0.z);

   rc.getSurface()->setFillColor(253);
   drawSquare(a, b, c, d, pointArray);     // Front
   drawSquare(e, h, g, f, pointArray);     // Rear
   rc.getSurface()->setFillColor(252);
   drawSquare(a, e, f, b, pointArray);     // Left
   drawSquare(d, c, g, h, pointArray);     // right
   rc.getSurface()->setFillColor(251);
   drawSquare(b, f, g, c, pointArray);     // Top
   drawSquare(a, d, h, e, pointArray);     // Bottom
}

//-----------------------------------------------------------------------

void BoxRenderImage::render(TSRenderContext &rc)
{
   GFXSurface *gfxSurface = rc.getSurface();
   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
	gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setTransparency(FALSE);
   TS::PointArray *pointArray = rc.getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(false);
	pointArray->setVisibility(TS::ClipMask);

   
   rc.getCamera()->pushTransform(transform);
   drawCube(box.fMin, box.fMax, rc);
	rc.getCamera()->popTransform();
}

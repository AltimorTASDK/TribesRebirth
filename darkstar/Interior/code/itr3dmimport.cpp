//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <stdio.h>
#include <ml.h>
#include <tpoly.h>
#include <tvector.h>
#include "itrbit.h"
#include "itr3dmimport.h"


//----------------------------------------------------------------------------

namespace ITR3DMImport
{
   enum Constants {
      MaxPolyVertex = 100,
   };
   enum BoxMapping {
      MapYZ,
      MapXZ,
      MapXY,
   };
   struct PolyVertex: public TPolyVertex {
      Point2F texture;
		bool colinear;
      void set(PolyVertex& start,PolyVertex& end,float scale);
   };
   class Poly: public TPoly<PolyVertex>
   {
   public:
      typedef TPoly<PolyVertex>::VertexList VertexList;
      Point2I textureSize;
      Point2I textureOffset;
      UInt8   textureScaleShift;
      bool    applyAmbient;
      BoxMapping boxMapping;
		UInt32 volumeMask;
      TPlaneF plane;
      int material;
      //
      void rotate();
      bool isPointPtr(Point3F* ptr);
   };
   class PolyList: public Vector<Poly*> {
   public:
      ~PolyList();
   };

   struct Colinear {
      float time;
      Point3F point;
      static int _USERENTRY compare(const void* a,const void* b);
   };
   typedef Vector<Colinear> ColinearList;

   //
   Point3F snap(const Point3F& p);
   bool isLine(const Point3F&,const Point3F&,const Point3F&);
   Point2F power2Size(const Point2F& ts);
   void boxMap(Poly* poly);
   void normalizeTexture(Poly* poly);
   void splitX(Poly* poly,PolyList* polyList);
   void splitY(Poly* poly,PolyList* polyList);
   bool findColinear(const Point3F&,const Point3F&,Poly*,PolyList&,ColinearList*);
   void insertVertices(PolyList& polyList);
   void import(const char* file,PolyList* polyList);
   void exportToGeometry(PolyList& polyList, ITRGeometry* geometry,Vector<UInt32>* volumeMasks);
   void sortByMaterial(PolyList& polyList);

   inline float fmax(float a, float b) { return (a > b)? a: b; }

   bool lowDetailInterior = false;
   int maxMipLevel    = 0;
   int splitDist      = 256;
   int MaxTextureCoor = 256;
   int MaxTextureMask = MaxTextureCoor - 1;
};


//----------------------------------------------------------------------------

float ITR3DMImport::scale = 1.0f;
float ITR3DMImport::distancePrecision = 0.01f;
float ITR3DMImport::distancePrecisionSQ = ((0.01f)*(0.01f));
float ITR3DMImport::normalPrecision = 0.0001f;
float ITR3DMImport::snapPrecision = 0.0f;
float ITR3DMImport::textureScale = 0.001f;


//--------------------------------------
//  Just a quick function for distance so we don't get the
// sqrt() hit from m_dist...
float
getDistSQ(Point3F &in_p1, Point3F &in_p2)
{
   return ((in_p1.x - in_p2.x) * (in_p1.x - in_p2.x) +
           (in_p1.y - in_p2.y) * (in_p1.y - in_p2.y) +
           (in_p1.z - in_p2.z) * (in_p1.z - in_p2.z));
}


//----------------------------------------------------------------------------

void
ITR3DMImport::setMaxMipLevel(const int in_maxMipLevel)
{
   maxMipLevel = in_maxMipLevel;
   splitDist = 256 << maxMipLevel;
}

//----------------------------------------------------------------------------

ITR3DMImport::PolyList::~PolyList()
{
   for (iterator itr = begin(); itr != end(); itr++)
      delete *itr;
}


//----------------------------------------------------------------------------
void ITR3DMImport::PolyVertex::set(ITR3DMImport::PolyVertex& start,
   ITR3DMImport::PolyVertex& end,float scale)
 {
	// Method used by the poly splitting.
   TPolyVertex::set(start,end,scale);
   texture.x = start.texture.x + (end.texture.x - start.texture.x) * scale;
   texture.y = start.texture.y + (end.texture.y - start.texture.y) * scale;
	colinear = false;
}


//----------------------------------------------------------------------------
// Rotate the poly so that the first three points are not colinear.
//
void ITR3DMImport::Poly::rotate()
{
   // Find the best 3 points.
   if (vertexList.size() < 4)
      return;
   float bestAngle = 1.0f;
   float bestVertex = 0;
   for (int v1 = 0; v1 < vertexList.size(); v1++) {
      int v2 = v1 + 1;
      if (v2 > vertexList.size() - 1)
         v2 = 0;
      int v3 = v2 + 1;
      if (v3 > vertexList.size() - 1)
         v3 = 0;
      Point3F vec1 = vertexList[v1].point;
      Point3F vec2 = vertexList[v3].point;
      vec1 -= vertexList[v2].point;
      vec2 -= vertexList[v2].point;

		float div = vec1.len() * vec2.len();
		if (div) {
	      float angle = fabs(m_dot(vec1,vec2)) / div;
	      if (angle < bestAngle) {
	         bestAngle = angle;
	         bestVertex = v1;
	      }
		}
   }

   // Rotate the best vertex to the start.
   while (bestVertex--) {
      PolyVertex t = vertexList[0];
      memmove(&vertexList[0],&vertexList[1],
         (vertexList.size() - 1) * sizeof(PolyVertex));
      vertexList.last() = t;
   }
}


//----------------------------------------------------------------------------

bool ITR3DMImport::Poly::isPointPtr(Point3F* ptr)
{
   // Returns true if the pointer points to one of poly's points.
   for (int i = 0; i < vertexList.size(); i++)
      if (ptr == &vertexList[i].point)
         return true;
   return false;
}


//----------------------------------------------------------------------------

Point3F ITR3DMImport::snap(const Point3F& p)
{
   if (snapPrecision != 0.0f) {
      Point3F sv;
      float snap2 = snapPrecision / 2;
      sv.x = p.x + snap2 - float(fmod(p.x + snap2,snapPrecision));
      sv.y = p.y + snap2 - float(fmod(p.y + snap2,snapPrecision));
      sv.z = p.z + snap2 - float(fmod(p.z + snap2,snapPrecision));
      return sv;
   }
   return p;
}  


//----------------------------------------------------------------------------

bool ITR3DMImport::isLine(const Point3F& k,const Point3F& j,const Point3F& l)
{
   Point3F kj = k,lj = l,pv;
   kj -= j;
   lj -= j;
   m_cross( kj, lj, &pv );
   return isEqual(pv.len(),.0f,TPlaneF::NormalPrecision);
}


//----------------------------------------------------------------------------

Point2F ITR3DMImport::power2Size(const Point2F& ts)
{
   // Find smallest 2^n size that bounds givin size.
   static int table[] =  { 1, 2, 4, 8, 16, 32, 64, 128, 256 };
   int x = 0, y = 0;
   for (; x < sizeof(table) * sizeof(*table); x++)
      if (ts.x <= table[x])
         break;
   for (; y < sizeof(table) * sizeof(*table); y++)
      if (ts.y <= table[y])
         break;
   return Point2F(table[x],table[y]);
};


//----------------------------------------------------------------------------

void ITR3DMImport::boxMap(Poly* poly)
{
   Point2F tMin(+1.0E20f,+1.0E20f);
   Point2F tMax(-1.0E20f,-1.0E20f);

   // Select box mapping
   if (fabs(poly->plane.x) > fabs(poly->plane.y))
      poly->boxMapping =
         (fabs(poly->plane.x) > fabs(poly->plane.z))? MapYZ: MapXY;
   else
      poly->boxMapping =
         (fabs(poly->plane.y) > fabs(poly->plane.z))? MapXZ: MapXY;

   // Map vertices to box face.
   for (int i = 0; i < poly->vertexList.size(); i++) {
      Point3F& tp = poly->vertexList[i].point;
      Point2F& tv = poly->vertexList[i].texture;
      switch(poly->boxMapping) {
         case MapYZ:
            tv.set(tp.y,-tp.z);
            break;
         case MapXY:
            tv.set(tp.x,-tp.y);
            break;
         case MapXZ:
            tv.set(tp.x,-tp.z);
            break;
      }
      tv *= textureScale;
      tMin.setMin(tv);
      tMax.setMax(tv);
   }

   // Min/Max must be whole numbers
   tMin.x = floor(tMin.x); tMin.y = floor(tMin.y);
   tMax.x = ceil(tMax.x); tMax.y = ceil(tMax.y);
   poly->textureSize.x = int(tMax.x - tMin.x);
   poly->textureSize.y = int(tMax.y - tMin.y);

   // 
   poly->textureOffset.x = int(tMin.x) & MaxTextureMask;
   poly->textureOffset.y = int(tMin.y) & MaxTextureMask;

   // Translate coor. so they fit on the texture.
   for (int v = 0; v < poly->vertexList.size(); v++)
      poly->vertexList[v].texture -= tMin;
}


//----------------------------------------------------------------------------
// Reset the texture coordinates after a split.
//
void ITR3DMImport::normalizeTexture(Poly* poly)
{
   Point2F tMin(+1.0E20f,+1.0E20f);
   Point2F tMax(-1.0E20f,-1.0E20f);

   for (int i = 0; i < poly->vertexList.size(); i++) {
      Point2F& tv = poly->vertexList[i].texture;
      tv.x += poly->textureOffset.x;
      tv.y += poly->textureOffset.y;
      tMin.setMin(tv);
      tMax.setMax(tv);
   }

   tMin.x = floor(tMin.x); tMin.y = floor(tMin.y);
   tMax.x = ceil(tMax.x);  tMax.y = ceil(tMax.y);

   // Mask onto power of 2 bitmap size.
   Point2I offset;
   offset.x = int(tMin.x) & MaxTextureMask;
   offset.y = int(tMin.y) & MaxTextureMask;

   // Adjust offset to multiple of 8 to avoid
   // mipmap jumping.
   poly->textureOffset.x = offset.x & ~0x7;
   poly->textureOffset.y = offset.y & ~0x7;
   tMin.x -= float(offset.x & 0x7);
   tMin.y -= float(offset.y & 0x7);

   poly->textureSize.x = int(tMax.x - tMin.x);
   poly->textureSize.y = int(tMax.y - tMin.y);

   for (int v = 0; v < poly->vertexList.size(); v++)
      poly->vertexList[v].texture -= tMin;
}


//----------------------------------------------------------------------------
// Split polygon whose texture coordinates are larger than 256

void ITR3DMImport::splitX(Poly* poly,PolyList* polyList)
{
   Vector<Point3F> points;
   Point3F iPoint;

   int v1 = poly->vertexList.size() - 1;
   for (int v2 = 0; v2 < poly->vertexList.size(); v2++) {
      Poly::Vertex* p1 = &poly->vertexList[v1];
      Poly::Vertex* p2 = &poly->vertexList[v2];

      if (p1->texture.x <= float(splitDist - 1) && p2->texture.x > float(splitDist - 1) ||
          p2->texture.x <= float(splitDist - 1) && p1->texture.x > float(splitDist - 1)) {
         Point3F vec;
         vec  = p2->point;
         vec -= p1->point;
         vec *= (float(splitDist - 1) - p1->texture.x) / (p2->texture.x - p1->texture.x);
         vec += p1->point;
         points.push_back(vec);
      }
      if (p1->texture.x < float(splitDist - 1))
         iPoint = p1->point;
      v1 = v2;
   }

   // Build plane and split the poly
   if (points.size() > 1) {
      Point3F vec = points[0];
      vec -= points[1];
      Point3F normal;
      m_cross(vec,poly->plane,&normal);
      TPlaneF plane(points[0],normal);
      if (plane.whichSide(iPoint) != TPlaneF::Inside)
         plane.neg();

      Poly tmp;
      Poly* npoly = new Poly;
      poly->split(plane,&tmp,npoly);
      poly->vertexList    = tmp.vertexList;

      npoly->plane = poly->plane;
      npoly->textureOffset = poly->textureOffset;
      npoly->material = poly->material;
      npoly->volumeMask = poly->volumeMask;
      npoly->textureScaleShift = poly->textureScaleShift;
      npoly->applyAmbient = poly->applyAmbient;
      polyList->push_back(npoly);

		// Normalize after the previous information copy, 
		// normalize alters some of the poly's fields.
      normalizeTexture(poly);
      normalizeTexture(npoly);
   }
}
   

//----------------------------------------------------------------------------
// Split polygon whose texture coordinates are larger than 256

void ITR3DMImport::splitY(Poly* poly,PolyList* polyList)
{
   Vector<Point3F> points;
   Point3F iPoint;

   int v1 = poly->vertexList.size() - 1;
   for (int v2 = 0; v2 < poly->vertexList.size(); v2++) {
      Poly::Vertex* p1 = &poly->vertexList[v1];
      Poly::Vertex* p2 = &poly->vertexList[v2];

      if (p1->texture.y <= float(splitDist - 1) && p2->texture.y > float(splitDist - 1) ||
          p2->texture.y <= float(splitDist - 1) && p1->texture.y > float(splitDist - 1)) {
         Point3F vec = p2->point;
         vec -= p1->point;
         vec *= (float(splitDist - 1) - p1->texture.y) / (p2->texture.y - p1->texture.y);
         vec += p1->point;
         points.push_back(vec);
      }
      if (p1->texture.y < float(splitDist - 1))
         iPoint = p1->point;
      v1 = v2;
   }

   // Build plane and split the poly
   if (points.size() > 1) {
      Point3F vec = points[0];
      vec -= points[1];
      Point3F normal;
      m_cross(vec,poly->plane,&normal);
      TPlaneF plane(points[0],normal);
      if (plane.whichSide(iPoint) != TPlaneF::Inside)
         plane.neg();

      Poly tmp;
      Poly* npoly = new Poly;
      poly->split(plane,&tmp,npoly);
      poly->vertexList = tmp.vertexList;

      npoly->plane = poly->plane;
      npoly->textureOffset = poly->textureOffset;
      npoly->material = poly->material;
      npoly->volumeMask = poly->volumeMask;
      npoly->textureScaleShift = poly->textureScaleShift;
      npoly->applyAmbient = poly->applyAmbient;
      polyList->push_back(npoly);

		// Normalize after the previous information copy, 
		// normalize alters some of the poly's fields.
      normalizeTexture(poly);
      normalizeTexture(npoly);
   }
}

   
//----------------------------------------------------------------------------

int _USERENTRY ITR3DMImport::Colinear::compare(const void* a,const void* b)
{
   const Colinear& ca = *reinterpret_cast<const Colinear*>(a);
   const Colinear& cb = *reinterpret_cast<const Colinear*>(b);
   return (ca.time > cb.time)? 1: (ca.time < cb.time)? -1: 0;
}

bool ITR3DMImport::findColinear(const Point3F& v1,const Point3F& v2,
   Poly* spoly,PolyList& polyList,ColinearList* colinearList)
{
   colinearList->clear();

   Point3F vec1 = v2; vec1 -= v1;
   float len = vec1.len();
   if (!len || len < distancePrecision)
      return false;
   float timeScale = 1.0f / len;
   vec1 *= timeScale;

	float maxDistance = len - distancePrecision;
	float dist2Precision = distancePrecision * distancePrecision;

   for (PolyList::iterator ptr = polyList.begin();
         ptr != polyList.end(); ptr++) {
      Poly* poly = *ptr;
      if (poly == spoly)
         continue;
      for (Poly::VertexList::iterator itr = poly->vertexList.begin(); 
            itr != poly->vertexList.end(); itr++) {
			if ((*itr).colinear)
				// Skip points that were inserted, we should end up testing
				// the original point.
				continue;
			// Is it within the bounds of the line seg?
         Point3F vec2,&v3 = (*itr).point;
         vec2.x = v3.x - v1.x;
         vec2.y = v3.y - v1.y;
         vec2.z = v3.z - v1.z;
         float d = m_dot(vec1,vec2);
         if (d > distancePrecision && d < maxDistance) {
				// Now lets see if it's on the line
				Point3F dp;
				dp.x = v3.x - (v1.x + vec1.x * d);
				dp.y = v3.y - (v1.y + vec1.y * d);
				dp.z = v3.z - (v1.z + vec1.z * d);
				float dist2 = dp.x * dp.x + dp.y * dp.y + dp.z * dp.z;
            if (dist2 < dist2Precision) {
					// Sanity check here, make sure the point is
					// on the plane.
					if (spoly->plane.whichSide((*itr).point) == TPlaneF::OnPlane) {
	               colinearList->increment();
	               colinearList->last().time = d * timeScale;
	               colinearList->last().point = (*itr).point;
					}
            }
         }
      }
   }

   // Sort by increasing time value.
   float precision = distancePrecision / len;
   if (colinearList->size()) {
      qsort(colinearList->address(),colinearList->size(),sizeof(Colinear),
         Colinear::compare);
      // Remove duplicates
      for (int i = 1; i < colinearList->size(); i++) {
         float delta = (*colinearList)[i].time - (*colinearList)[i-1].time;
         if (fabs(delta) < precision)
            colinearList->erase(i--);
      }
      return true;
   }
   return false;
}

void ITR3DMImport::insertVertices(PolyList& polyList)
{
   ColinearList colinearList;
   for (int p = 0; p < polyList.size(); p++) {
      Poly* poly = polyList[p];

      // Loop through all the edges.
      int v1 = poly->vertexList.size() - 1;
      for (int v2 = 0; v2 < poly->vertexList.size(); v2++) {
         // For each edge, find colinear points.
         Point3F &p1 = poly->vertexList[v1].point;
         Point3F &p2 = poly->vertexList[v2].point;
         if (findColinear(p1,p2,poly,polyList,&colinearList)) {
            Point2F tv1 = poly->vertexList[v1].texture;
            Point2F tv2 = poly->vertexList[v2].texture;
            tv2 -= tv1;
            // Insert new vertices in order.
            for (ColinearList::iterator itr = colinearList.begin();
                  itr != colinearList.end(); itr++) {
               poly->vertexList.insert(v2);
               PolyVertex& vertex = poly->vertexList[v2++];
               float time = (*itr).time;
               vertex.texture.x = tv1.x + (tv2.x * time);
               vertex.texture.y = tv1.y + (tv2.y * time);
               vertex.point = (*itr).point;
					vertex.colinear = true;
            }
         }
         v1 = v2;
      }

      // Make sure first three vertices are not colinear.
      poly->rotate();

		//
      for (int i = 0; i < poly->vertexList.size(); i++) {
			Point3F& p = poly->vertexList[i].point;
			float distance = poly->plane.distance(p);
			if (distance > distancePrecision) {
				printf("Doh!");
			}
      }
   }
}


//----------------------------------------------------------------------------
// ** Note:  The vertex and plane normals values are swapped
// around to align with our normal viewing axis.
//
void ITR3DMImport::import(const char* file,PolyList* polyList)
{
   int version = 0;
   
   FILE* fp = fopen(file,"r");
   char buff[256];
   AssertFatal(fp != 0,"ITR3DMImport::import: Error openning file");

   // Load the vertex list - or the version number...
   do
      fgets(buff,256,fp);
   while( strcmp(buff,"VERTICES\n") && strcmp( buff, "VERSION\n" ) );
   
   // check if we got verion
   if( !strcmp( buff, "VERSION\n" ) )
   {
      fscanf( fp, "%d\n", &version );   
      
      // get to the vertices
      do
         fgets(buff,256,fp);
      while( strcmp(buff,"VERTICES\n") );
   }
   
   int vertexCount;
   fscanf(fp,"%d\n",&vertexCount);

   Vector<Point3F> pointList;
   pointList.setSize(vertexCount);
   for (int i = 0; i < vertexCount; i++) {
      Point3F pp;
      // Convert to our coordinate system.
      fscanf(fp,"%f %f %f\n",&pp.x,&pp.z,&pp.y);
      pp *= scale;
      pointList[i] = snap(pp);
   }

   // Load the faces
   do
      fgets(buff,256,fp);
   while (strcmp(buff,"POLYGONS\n"));
   int polyCount;
   fscanf(fp,"%d\n",&polyCount);
	int degeneratePoly = 0;

   polyList->reserve(polyCount);
   for (int i = 0; i < polyCount; i++) {
      Poly* poly = new Poly;
      fgets(buff,256,fp);
      poly->material = atoi(buff);

      int count;
      fscanf(fp,"%d\n",&count);
      if (count < 3) {
			degeneratePoly++;
         delete poly;
         continue;
      }
      poly->vertexList.setSize(count);

      // DMM - Scan in texture size and offset.  (New in Zed);
      fscanf(fp, "%d %d\n", &poly->textureSize.x, &poly->textureSize.y);
      fscanf(fp, "%d %d\n", &poly->textureOffset.x, &poly->textureOffset.y);
      
      // grab the texture scaling factor
      int textureScaleShift;
      fscanf(fp, "%d\n", &textureScaleShift );
      poly->textureScaleShift = textureScaleShift;

      // check if there is a ambient flag ( any version will have this... )
      if( version )
      {
         int tmp;
         fscanf( fp, "%d\n", &tmp );
         poly->applyAmbient = bool( tmp );
      }
      
		// Vertices
      for (int i = 0; i < count; i++) {
         int index;
         fscanf(fp,"%d",&index);
         poly->vertexList[i].point = pointList[index];
         poly->vertexList[i].colinear = false;
      }
      fscanf(fp,"\n",buff);

		// Texture coor.
      for (int i = 0; i < count; i++) {
         Point2F& texture = poly->vertexList[i].texture;
         fscanf(fp, "%f %f", &texture.x,&texture.y);
      }
      fscanf(fp,"\n",buff);

		// Volume mask
      fscanf(fp,"%lu\n",&poly->volumeMask);

#if 1
      // Read in the plane exported by zed
		fscanf(fp,"%f %f %f %f\n",
			&poly->plane.x, &poly->plane.z, &poly->plane.y, &poly->plane.d);
		poly->plane.d *= scale;

		// Use the plane class to make sure the polygon
		// is not degenerate.
		TPlaneF tmp;
      Point3F* vl[MaxPolyVertex];
      AssertFatal(poly->vertexList.size() <= MaxPolyVertex,
         "ITR3DMImport::import: Poly exceeded max vertex size");
      for (int v = 0; v < poly->vertexList.size(); v++)
         vl[v] = &poly->vertexList[v].point;
      if (!tmp.set(poly->vertexList.size(),vl)) {
			degeneratePoly++;
         delete poly;
         continue;
      }
#else
		// Build the plane using the vertices
      Point3F* vl[MaxPolyVertex];
      AssertFatal(poly->vertexList.size() <= MaxPolyVertex,
         "ITR3DMImport::import: Poly exceeded max vertex size");
      for (int v = 0; v < poly->vertexList.size(); v++)
         vl[v] = &poly->vertexList[v].point;
      if (!poly->plane.set(poly->vertexList.size(),vl)) {
			degeneratePoly++;
         delete poly;
         continue;
      }
#endif

      // If we have bad texture coordinates, let's try re-boxmapping
		// first.
      if (poly->textureSize.x == 0 || poly->textureSize.y == 0) {
         printf("   Degenerate poly textures (%d)\n",i);
         boxMap(poly);
      }
      normalizeTexture(poly);
      if (poly->textureSize.x == 0 || poly->textureSize.y == 0) {
			degeneratePoly++;
         delete poly;
         continue;
      }

		//
      polyList->push_back(poly);
   }

	if (degeneratePoly)
	   printf("\n   *****: %d Degenerate polys dropped\n   ",degeneratePoly);
   fclose(fp);
}  


//----------------------------------------------------------------------------

void ITR3DMImport::exportToGeometry(PolyList&       polyList,
                          ITRGeometry*    geometry,
	                       Vector<UInt32>* volumeMasks)
{
   AssertFatal(polyList.size() < ITRGeometry::MaxIndex,
      "ITR3DMImport:export: Too many polygons");
   geometry->surfaceList.reserve(polyList.size());

   for (int i = 0; i < polyList.size(); i++) {
      Poly* poly = polyList[i];
      ITRBitVector bv;

      // Build surface
      geometry->surfaceList.push_back(ITRGeometry::Surface());
      ITRGeometry::Surface& surface = geometry->surfaceList.last();
      surface.type = ITRGeometry::Surface::Material;
      surface.material = (poly->material == -1)?
      	ITRGeometry::Surface::NullMaterial: poly->material;
      surface.textureOffset.x = poly->textureOffset.x;
      surface.textureOffset.y = poly->textureOffset.y;
      surface.textureScaleShift = ( poly->textureScaleShift & 
         ( ( 1 << ITRGeometry::Surface::textureScaleBits  ) - 1 ) );
      surface.applyAmbient = poly->applyAmbient;
      		
      // Default is _not_ visible to the outside...
      surface.visibleToOutside  = 0;
		
		volumeMasks->push_back(poly->volumeMask);

      // Size is 1->256 stored in UInt8
		Point2I pSize;
		pSize.x = poly->textureSize.x >> maxMipLevel;
		pSize.y = poly->textureSize.y >> maxMipLevel;
      surface.textureSize.x = (pSize.x == 0)? 1: pSize.x - 1;
      surface.textureSize.y = (pSize.y == 0)? 1: pSize.y - 1;

      // Build poly
		AssertFatal(poly->vertexList.size() < 256,
				"ITR3DMImport::export: Vertex count on poly too large")
      surface.vertexCount = poly->vertexList.size();
      surface.vertexIndex = geometry->vertexList.size();
      geometry->vertexList.setSize(geometry->vertexList.size() + surface.vertexCount);
      ITRGeometry::Vertex* vp = &geometry->vertexList[surface.vertexIndex];

      for (int i = 0; i < surface.vertexCount; i++) {
         vp[i].pointIndex = 
            geometry->point3List.add(poly->vertexList[i].point);
         AssertFatal(geometry->point3List.size() < ITRGeometry::MaxIndex,
            "ITR3DMImport:export: Too many poly vertices");

         // Texture coordinates normalized to texture size
         Point2F tx = poly->vertexList[i].texture;
         tx.x *= (1.0 / poly->textureSize.x);
         tx.y *= (1.0 / poly->textureSize.y);

         // Make sure it's not out of bounds (Zed bug?).
         bool  clamp = false;
         if (tx.x < 0.0f)
            tx.x = 0.0f, clamp = true;
         if (tx.x > 1.0f)
            tx.x = 1.0f, clamp = true;
         if (tx.y < 0.0f)
            tx.y = 0.0f, clamp = true;
         if (tx.y > 1.0f)
            tx.y = 1.0f, clamp = true;
         if (clamp)
            printf("   Warning: Texture coordinate clamped\n");

         //
         vp[i].textureIndex = geometry->point2List.add(tx);
         AssertFatal(geometry->point2List.size() < ITRGeometry::MaxIndex,
            "ITR3DMImport:export: Too many texture vertices");
         bv.set(vp[i].pointIndex);
      }

      // Add plane to list.
      ITRVector<TPlaneF>::iterator itr = 
         ::find(geometry->planeList.begin(),geometry->planeList.end(),
            poly->plane);
      if (itr == geometry->planeList.end()) {
         // Try too match inverted plane.
         poly->plane.neg();
         itr = ::find(geometry->planeList.begin(),
            geometry->planeList.end(),poly->plane);
         if (itr == geometry->planeList.end()) {
            // No inverted either, so add original plane
            // to the list.
            surface.planeIndex = geometry->planeList.size();
            surface.planeFront = true;
            poly->plane.neg();
            geometry->planeList.push_back(poly->plane);
         }
         else {
            surface.planeIndex = itr - geometry->planeList.begin();
            surface.planeFront = false;
         }
      }
      else {
         surface.planeIndex = itr - geometry->planeList.begin();
         surface.planeFront = true;
      }

      // Build bitvec of points used by the surface
      surface.pointIndex = geometry->bitList.size();
		int pcount = bv.compress(&geometry->bitList);
		AssertFatal(pcount < 256,
				"ITR3DMImport::export: Point bitvector too large");
      surface.pointCount = pcount;
   }
}


int _USERENTRY
materialCmp(const void* in_p1, const void* in_p2)
{
   ITR3DMImport::Poly** ppPoly1 = (ITR3DMImport::Poly**)in_p1;
   ITR3DMImport::Poly** ppPoly2 = (ITR3DMImport::Poly**)in_p2;

   return ((*ppPoly1)->material - (*ppPoly2)->material);
}


void
ITR3DMImport::sortByMaterial(PolyList& polyList)
{
   qsort(polyList.address(), polyList.size(), sizeof(PolyList::value_type),
         materialCmp);
}

//----------------------------------------------------------------------------

bool ITR3DMImport::import(const char* file, ITRGeometry* geometry,
	Vector<UInt32>* volumeMasks)
{
   TPlaneF::DistancePrecision = distancePrecision;
   TPlaneF::NormalPrecision   = normalPrecision;
   char buff[256];

   //
   printf("Thred.3DM Import\n");

   PolyList polyList;
   printf("   Importing...");
   import(file,&polyList);

#if 0
	// This is now down by Zed.
   // Texture mapping
   printf("Texturing...");
   for (int i = 0; i < polyList.size(); i++)
      boxMap(polyList[i]);
#endif

   // Split polys whose textures are larger than 256x256
   printf("Splitting...");
   for (int i = 0; i < polyList.size(); i++) {
      Poly* poly = polyList[i];
      if (poly->textureSize.x > splitDist)
         splitX(poly,&polyList);
      if (poly->textureSize.y > splitDist)
         splitY(poly,&polyList);
   }

   printf("Material Sorting...");
   sortByMaterial(polyList);

   if (lowDetailInterior == false) {
      // Insert colinear vertices into polygons
      printf("SharedVertices...");
      insertVertices(polyList);
   } else {
      printf("LowDetail (Shared Vertices not inserted)...");
      geometry->setFlag(ITRGeometry::LowDetailInterior);
   }

   //
   printf("Export...");
   exportToGeometry(polyList, geometry, volumeMasks);
   geometry->highestMipLevel = maxMipLevel;
   printf("\n");

   //
   printf("   Vertices: %d\n", geometry->point3List.size());
   printf("   Surfaces: %d\n", geometry->surfaceList.size());
   printf("   Planes: %d\n",   geometry->planeList.size());
   return true;
}

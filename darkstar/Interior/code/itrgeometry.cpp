//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#include "itrgeometry.h"


static const int FileVersion = 7;


//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(ITRGeometry);

ITRGeometry::ITRGeometry()
 : flags(0)
{
}

ITRGeometry::~ITRGeometry()
{
}

//----------------------------------------------------------------------------

int ITRGeometry::findLeaf(const Point3F& p,int nodeIndex)
{
	if (nodeIndex < 0)
		return -(nodeIndex+1);
	//
	BSPNode& node = nodeList[nodeIndex];
	if (planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
		return findLeaf(p,node.front);
	return findLeaf(p,node.back);
}	


//----------------------------------------------------------------------------

int ITRGeometry::externalLeaf(const Point3F& p)
{
	int index = 0;
	index |= (p.x < box.fMin.x)?  1: (p.x > box.fMax.x)?  2: 0;
	index |= (p.y < box.fMin.y)?  4: (p.y > box.fMax.y)?  8: 0;
	index |= (p.z < box.fMin.z)? 16: (p.z > box.fMax.z)? 32: 0;
	return index;
}


//----------------------------------------------------------------------------

int ITRGeometry::getOutsideBits(BSPLeafWrap& rLeafWrap)
{
	// The first 6 leafs represent visible faces of
	// the bounding box.
	if (rLeafWrap.getPVSCount() == 0)
		return 0;
	return bitList[rLeafWrap.getPVSIndex()] & OutsideMask;
}	


//----------------------------------------------------------------------------

template <class T>
inline void readSize(StreamIO& s,T& array)
{
	Int32 size;
	s.read(&size);
	array.setSize(size);
}

template <class T>
inline void readArray(StreamIO& s,T& array)
{
	s.read(array.size() * sizeof(T::value_type),
		(void*)array.address());
}	

template <class T>
inline void writeSize(StreamIO& s,T& array)
{
	s.write(Int32(array.size()));
}

template <class T>
inline void writeArray(StreamIO& s,T& array)
{
	s.write(array.size() * sizeof(T::value_type),
			(void*)array.address());
}


//----------------------------------------------------------------------------

Persistent::Base::Error ITRGeometry::read(StreamIO &s,int version,int)
{
	AssertFatal(version == FileVersion, "ITRGeometry::read: Incorrect file version");

	// Header, or in memory data first.
	s.read(&buildId);
	s.read(&textureScale);
	s.read(sizeof(Box3F),(void*)&box);
	readSize(s,surfaceList);
	readSize(s,nodeList);
	readSize(s,solidLeafList);
	readSize(s,emptyLeafList);
	readSize(s,bitList);
	readSize(s,vertexList);
	readSize(s,point3List);
	readSize(s,point2List);
	readSize(s,planeList);

	// Memory mapped portion
	readArray(s,surfaceList);
	readArray(s,nodeList);
	readArray(s,solidLeafList);
	readArray(s,emptyLeafList);
	readArray(s,bitList);
	readArray(s,vertexList);
	readArray(s,point3List);
	readArray(s,point2List);
	readArray(s,planeList);

   s.read(&highestMipLevel);
   s.read(&flags);

   // We need to determine which vertices are on the faces of the bounding box...
   //
   static const float s_jitterVal = 0.0125f;

   // DMMNOTE: TEMP
   for (int i = 0; i < point3List.size(); i++) {
      if (point3List[i].x == box.fMin.x)
         point3List[i].x -= s_jitterVal;
      if (point3List[i].y == box.fMin.y)
         point3List[i].y -= s_jitterVal;
      if (point3List[i].z == box.fMin.z)
         point3List[i].z -= s_jitterVal;

      if (point3List[i].x == box.fMax.x)
         point3List[i].x += s_jitterVal;
      if (point3List[i].y == box.fMax.y)
         point3List[i].y += s_jitterVal;
      if (point3List[i].z == box.fMax.z)
         point3List[i].z += s_jitterVal;
   }

	return (s.getStatus() == STRM_OK)? Ok : ReadError;
}

Persistent::Base::Error ITRGeometry::write(StreamIO &s,int version,int)
{
	AssertFatal(version == FileVersion,
               "ITRGeometry::write: Incorrect file version");

	// For better paging, sort:
	//		points by location (x:y)
	// 	surfaces by location
	//		polys by sorted surface order
	//		vertex by sorted poly order
	//		nodes by tree hierarchy?
	//		leafs by parent node order, maybe location?
	//		planeList by sorted surface order
	// 	Bitlist by sorted leaf node order
	//		Index list by sorted poly order?

	// Header, or in memory data first.
	s.write(buildId);
	s.write(textureScale);
	s.write(sizeof(Box3F),(void*)&box);
	writeSize(s,surfaceList);
	writeSize(s,nodeList);
	writeSize(s,solidLeafList);
	writeSize(s,emptyLeafList);
	writeSize(s,bitList);
	writeSize(s,vertexList);
	writeSize(s,point3List);
	writeSize(s,point2List);
	writeSize(s,planeList);

	// Memory mapped portion
	writeArray(s,surfaceList);
	writeArray(s,nodeList);
	writeArray(s,solidLeafList);
	writeArray(s,emptyLeafList);
	writeArray(s,bitList);
	writeArray(s,vertexList);
	writeArray(s,point3List);
	writeArray(s,point2List);
	writeArray(s,planeList);

   s.write(highestMipLevel);
   s.write(flags);

	return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

int ITRGeometry::version()
{
	return FileVersion;
}	


//------------------------------------------------------------------------------
// NAME 
//    Point2F 
//    ITRGeometry::getClosestTextureCoord(int            surfaceIndex,
//                                        const Point3F& worldPoint)
// DESCRIPTION 
//    returns texture coordinate of worldPoint.  Assumes worldpoint lies on the
//   plane of the surface...
//
//------------------------------------------------------------------------------
Point2F ITRGeometry::getTextureCoord(int surfaceIndex, const Point3F& worldPoint)
{
   Point3F wsv1, wsv2;
   Point2F tsv1, tsv2;
   float scale1, scale2;

   Surface& surface        = surfaceList[surfaceIndex];
   ITRGeometry::Vertex* vp = &vertexList[surface.vertexIndex];
   Point3F  world_base = point3List[vp[1].pointIndex];
   float    wlen1, wlen2;

   wsv1  = point3List[vp[0].pointIndex];
   wsv2  = point3List[vp[2].pointIndex];
   wsv1 -= world_base;
   wsv2 -= world_base;
   wlen2 = wsv2.len();

      // normalize by hand to get the length.  
   wsv2 /= wlen2;

   float nonOrthoFactor = m_dot(wsv1, wsv2);
   if ( nonOrthoFactor != 0.0 )
   {
      // normalize wsv1 again and get new length for it.  
      // float scaledown = sqrt ( 1.0 - nonOrthoFactor * nonOrthoFactor );
      // wsv1 /= scaledown;      // should be normalized right now.  
      // wlen1 *= scaledown;
      Point3F temp = wsv2;
      temp *= nonOrthoFactor;
      wsv1 -= temp;
   }

   // Adjust the world space vectors to be orthogonal...
   //
   Point2F base = point2List[vp[1].textureIndex];
   tsv1  = point2List[vp[0].textureIndex];
   tsv2  = point2List[vp[2].textureIndex];
   tsv1 -= base;
   tsv2 -= base;

   tsv1.x *= (surface.textureSize.x + 1);
   tsv2.x *= (surface.textureSize.x + 1);
   tsv1.y *= (surface.textureSize.y + 1);
   tsv2.y *= (surface.textureSize.y + 1);
   base.x *= (surface.textureSize.x + 1);
   base.y *= (surface.textureSize.y + 1);
   
   // Adjust the texture coords for orthogonality.  We need to normalize our points to do
   //    this and use our own orthonormal factor here.  
   //
   if ( nonOrthoFactor != 0.0 )
   {
      Point2F temp2 = tsv2;
      temp2 *= (nonOrthoFactor / wlen2);
      tsv1 -= temp2;
   }

   Point3F cPoint = worldPoint;
   cPoint   -= world_base;

   wlen1 = wsv1.len();
   wsv1.normalize ();
   scale1 = m_dot(wsv1, cPoint) / wlen1;
   scale2 = m_dot(wsv2, cPoint) / wlen2;
   
   tsv1 *= scale1;
   tsv2 *= scale2;

   Point2F retPoint = tsv1;
   retPoint += tsv2;
   retPoint += base;

   return retPoint;
}
